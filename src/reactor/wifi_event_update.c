#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
    #include <net/if.h>
#endif

#if defined(__linux__) && defined(NET_STATS_HAS_LIBNL)
    #include <ctype.h>
    #include <linux/nl80211.h>
    #include <netlink/attr.h>
    #include <netlink/genl/ctrl.h>
    #include <netlink/genl/genl.h>
    #include <netlink/msg.h>
    #include <netlink/netlink.h>
    #include <stdint.h>
#endif

#include "reactor/wifi_event_update.h"

#include "data/wifi_data.h"

#ifdef __linux__
typedef struct wireless_proc_row
{
    char iface[IFNAMSIZ];
    float link;
    float level;
    unsigned long retry;
    unsigned long beacon;
} WirelessProcRow;
#endif

#if defined(__linux__) && defined(NET_STATS_HAS_LIBNL)
typedef struct wifi_nl_ctx
{
    struct nl_sock *sock;
    int family_id;
} WifiNlCtx;

typedef struct wifi_nl_info
{
    int associated;
    char ssid[33];
    char bssid[18];
    int rssi_dbm;
    float tx_bitrate_mbps;
    float rx_bitrate_mbps;
    int mcs;
} WifiNlInfo;

typedef struct wifi_bss_cb_data
{
    WifiNlInfo *info;
} WifiBssCbData;

typedef struct wifi_station_cb_data
{
    WifiNlInfo *info;
} WifiStationCbData;
#endif

#ifdef __linux__
static int scan_proc_net_wireless(WirelessProcRow **rows_out, size_t *count_out)
{
    if (rows_out == NULL || count_out == NULL) return -1;

    FILE *f = fopen("/proc/net/wireless", "r");
    if (!f)
    {
        perror("/proc/net/wireless");
        return -1;
    }

    char line[1024];
    if (!fgets(line, sizeof(line), f) || !fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    WirelessProcRow *rows = NULL;
    size_t count = 0;
    size_t cap = 0;
    int failed = 0;

    while (fgets(line, sizeof(line), f))
    {
        WirelessProcRow row;
        memset(&row, 0, sizeof(row));

        unsigned int status = 0;
        float noise = 0;
        unsigned long nwid = 0;
        unsigned long crypt = 0;
        unsigned long frag = 0;
        unsigned long misc = 0;

        int n = sscanf(
            line,
            " %15[^:]: %x %f %f %f %lu %lu %lu %lu %lu %lu",
            row.iface,
            &status,
            &row.link,
            &row.level,
            &noise,
            &nwid,
            &crypt,
            &frag,
            &row.retry,
            &misc,
            &row.beacon
        );
        if (n < 11) continue;

        if (count == cap)
        {
            size_t new_cap = cap ? cap * 2 : 4;
            WirelessProcRow *tmp = realloc(rows, new_cap * sizeof(*rows));
            if (!tmp)
            {
                perror("realloc(WirelessProcRow)");
                failed = 1;
                break;
            }
            rows = tmp;
            cap = new_cap;
        }

        rows[count++] = row;
    }

    fclose(f);

    if (failed)
    {
        free(rows);
        *rows_out = NULL;
        *count_out = 0;
        return -1;
    }

    *rows_out = rows;
    *count_out = count;
    return 0;
}

static int quality_pct_from_link(float link)
{
    if (link < 0.0f) return -1;

    int pct = (int) ((link * 100.0f) / 70.0f + 0.5f);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    return pct;
}

static int rssi_dbm_from_level(float level)
{
    if (level > 0.0f) return (int) (level - 256.0f);
    return (int) level;
}

static int find_prev_iface_index(char **prev_iface, size_t prev_count, const char *iface)
{
    if (prev_iface == NULL || iface == NULL) return -1;

    for (size_t i = 0; i < prev_count; ++i)
    {
        if (prev_iface[i] != NULL && strcmp(prev_iface[i], iface) == 0)
        {
            return (int) i;
        }
    }

    return -1;
}

static int store_prev_wireless_counters(
    const WirelessProcRow *rows,
    size_t count,
    char ***prev_iface_out,
    unsigned long **prev_retry_out,
    unsigned long **prev_beacon_out,
    size_t *prev_count_out
)
{
    if (prev_iface_out == NULL || prev_retry_out == NULL || prev_beacon_out == NULL || prev_count_out == NULL)
    {
        return -1;
    }

    if (count == 0)
    {
        if (*prev_iface_out != NULL)
        {
            for (size_t i = 0; i < *prev_count_out; ++i) free((*prev_iface_out)[i]);
            free(*prev_iface_out);
        }
        free(*prev_retry_out);
        free(*prev_beacon_out);

        *prev_iface_out = NULL;
        *prev_retry_out = NULL;
        *prev_beacon_out = NULL;
        *prev_count_out = 0;
        return 0;
    }

    if (rows == NULL) return -1;

    char **new_prev_iface = calloc(count ? count : 1, sizeof(*new_prev_iface));
    unsigned long *new_prev_retry = calloc(count ? count : 1, sizeof(*new_prev_retry));
    unsigned long *new_prev_beacon = calloc(count ? count : 1, sizeof(*new_prev_beacon));
    if (!new_prev_iface || !new_prev_retry || !new_prev_beacon)
    {
        free(new_prev_iface);
        free(new_prev_retry);
        free(new_prev_beacon);
        return -1;
    }

    for (size_t i = 0; i < count; ++i)
    {
        new_prev_iface[i] = strdup(rows[i].iface);
        if (!new_prev_iface[i])
        {
            for (size_t j = 0; j < i; ++j) free(new_prev_iface[j]);
            free(new_prev_iface);
            free(new_prev_retry);
            free(new_prev_beacon);
            return -1;
        }

        new_prev_retry[i] = rows[i].retry;
        new_prev_beacon[i] = rows[i].beacon;
    }

    if (*prev_iface_out != NULL)
    {
        for (size_t i = 0; i < *prev_count_out; ++i) free((*prev_iface_out)[i]);
        free(*prev_iface_out);
    }
    free(*prev_retry_out);
    free(*prev_beacon_out);

    *prev_iface_out = new_prev_iface;
    *prev_retry_out = new_prev_retry;
    *prev_beacon_out = new_prev_beacon;
    *prev_count_out = count;
    return 0;
}
#endif

#if defined(__linux__) && defined(NET_STATS_HAS_LIBNL)
static void init_wifi_nl_info(WifiNlInfo *info)
{
    if (info == NULL) return;

    info->associated = 0;
    info->ssid[0] = '\0';
    info->bssid[0] = '\0';
    info->rssi_dbm = -1;
    info->tx_bitrate_mbps = -1.0f;
    info->rx_bitrate_mbps = -1.0f;
    info->mcs = -1;
}

static void format_mac_address(const unsigned char *mac, char out[18])
{
    if (mac == NULL || out == NULL) return;

    snprintf(
        out,
        18,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );
}

static void parse_ssid_from_ies(const unsigned char *ies, int ies_len, char out[33])
{
    if (ies == NULL || out == NULL || ies_len <= 0)
    {
        if (out != NULL) out[0] = '\0';
        return;
    }

    out[0] = '\0';

    int i = 0;
    while (i + 1 < ies_len)
    {
        unsigned int id = ies[i];
        unsigned int len = ies[i + 1];
        i += 2;

        if (i + (int) len > ies_len) break;
        if (id != 0)
        {
            i += (int) len;
            continue;
        }

        size_t copy_len = len < 32 ? len : 32;
        for (size_t j = 0; j < copy_len; ++j)
        {
            unsigned char ch = ies[i + (int) j];
            out[j] = isprint(ch) ? (char) ch : '?';
        }
        out[copy_len] = '\0';
        return;
    }
}

static float read_bitrate_from_attr(struct nlattr *bitrate_attr, int *mcs_out)
{
    if (bitrate_attr == NULL) return -1.0f;

    struct nlattr *rate_info[NL80211_RATE_INFO_MAX + 1];
    memset(rate_info, 0, sizeof(rate_info));
    if (nla_parse_nested(rate_info, NL80211_RATE_INFO_MAX, bitrate_attr, NULL) != 0) return -1.0f;

    float bitrate = -1.0f;
    if (rate_info[NL80211_RATE_INFO_BITRATE32])
    {
        bitrate = (float) nla_get_u32(rate_info[NL80211_RATE_INFO_BITRATE32]) / 10.0f;
    }
    else if (rate_info[NL80211_RATE_INFO_BITRATE])
    {
        bitrate = (float) nla_get_u16(rate_info[NL80211_RATE_INFO_BITRATE]) / 10.0f;
    }

    if (mcs_out != NULL)
    {
        if (rate_info[NL80211_RATE_INFO_MCS]) *mcs_out = (int) nla_get_u8(rate_info[NL80211_RATE_INFO_MCS]);
        else if (rate_info[NL80211_RATE_INFO_VHT_MCS]) *mcs_out = (int) nla_get_u8(rate_info[NL80211_RATE_INFO_VHT_MCS]);
        else *mcs_out = -1;
    }

    return bitrate;
}

static int wifi_bss_valid_cb(struct nl_msg *msg, void *arg)
{
    WifiBssCbData *cb_data = (WifiBssCbData *) arg;
    if (cb_data == NULL || cb_data->info == NULL) return NL_SKIP;

    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct genlmsghdr *gnlh = nlmsg_data(nlh);
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    memset(tb, 0, sizeof(tb));

    if (nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL) != 0)
    {
        return NL_SKIP;
    }

    if (!tb[NL80211_ATTR_BSS]) return NL_SKIP;

    struct nlattr *bss[NL80211_BSS_MAX + 1];
    memset(bss, 0, sizeof(bss));
    if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], NULL) != 0)
    {
        return NL_SKIP;
    }

    if (!bss[NL80211_BSS_STATUS]) return NL_SKIP;

    unsigned int status = nla_get_u32(bss[NL80211_BSS_STATUS]);
    if (
        status != NL80211_BSS_STATUS_ASSOCIATED &&
        status != NL80211_BSS_STATUS_IBSS_JOINED
    )
    {
        return NL_SKIP;
    }

    cb_data->info->associated = 1;

    if (bss[NL80211_BSS_BSSID] && nla_len(bss[NL80211_BSS_BSSID]) >= 6)
    {
        format_mac_address((const unsigned char *) nla_data(bss[NL80211_BSS_BSSID]), cb_data->info->bssid);
    }

    if (bss[NL80211_BSS_INFORMATION_ELEMENTS])
    {
        parse_ssid_from_ies(
            (const unsigned char *) nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]),
            nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]),
            cb_data->info->ssid
        );
    }

    if (bss[NL80211_BSS_SIGNAL_MBM])
    {
        cb_data->info->rssi_dbm = (int) nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]) / 100;
    }

    return NL_SKIP;
}

static int wifi_station_valid_cb(struct nl_msg *msg, void *arg)
{
    WifiStationCbData *cb_data = (WifiStationCbData *) arg;
    if (cb_data == NULL || cb_data->info == NULL) return NL_SKIP;

    struct nlmsghdr *nlh = nlmsg_hdr(msg);
    struct genlmsghdr *gnlh = nlmsg_data(nlh);
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    memset(tb, 0, sizeof(tb));

    if (nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL) != 0)
    {
        return NL_SKIP;
    }

    if (tb[NL80211_ATTR_MAC] && cb_data->info->bssid[0] == '\0' && nla_len(tb[NL80211_ATTR_MAC]) >= 6)
    {
        format_mac_address((const unsigned char *) nla_data(tb[NL80211_ATTR_MAC]), cb_data->info->bssid);
    }

    if (!tb[NL80211_ATTR_STA_INFO]) return NL_SKIP;

    struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
    memset(sinfo, 0, sizeof(sinfo));
    if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], NULL) != 0)
    {
        return NL_SKIP;
    }

    cb_data->info->associated = 1;

    if (sinfo[NL80211_STA_INFO_SIGNAL])
    {
        cb_data->info->rssi_dbm = (int) ((int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]));
    }

    if (sinfo[NL80211_STA_INFO_TX_BITRATE])
    {
        cb_data->info->tx_bitrate_mbps = read_bitrate_from_attr(
            sinfo[NL80211_STA_INFO_TX_BITRATE],
            &cb_data->info->mcs
        );
    }

    if (sinfo[NL80211_STA_INFO_RX_BITRATE])
    {
        cb_data->info->rx_bitrate_mbps = read_bitrate_from_attr(
            sinfo[NL80211_STA_INFO_RX_BITRATE],
            NULL
        );
    }

    return NL_SKIP;
}

static int open_wifi_nl_ctx(WifiNlCtx *ctx)
{
    if (ctx == NULL) return -1;

    ctx->sock = nl_socket_alloc();
    if (!ctx->sock) return -1;

    if (genl_connect(ctx->sock) != 0)
    {
        nl_socket_free(ctx->sock);
        ctx->sock = NULL;
        return -1;
    }

    ctx->family_id = genl_ctrl_resolve(ctx->sock, "nl80211");
    if (ctx->family_id < 0)
    {
        nl_socket_free(ctx->sock);
        ctx->sock = NULL;
        return -1;
    }

    return 0;
}

static void close_wifi_nl_ctx(WifiNlCtx *ctx)
{
    if (ctx == NULL) return;

    if (ctx->sock != NULL)
    {
        nl_socket_free(ctx->sock);
        ctx->sock = NULL;
    }
    ctx->family_id = -1;
}

static int read_wifi_bss_info(WifiNlCtx *ctx, int ifindex, WifiNlInfo *info)
{
    if (ctx == NULL || ctx->sock == NULL || info == NULL) return -1;

    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) return -1;

    if (!genlmsg_put(msg, 0, 0, ctx->family_id, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0))
    {
        nlmsg_free(msg);
        return -1;
    }

    if (nla_put_u32(msg, NL80211_ATTR_IFINDEX, (unsigned int) ifindex) != 0)
    {
        nlmsg_free(msg);
        return -1;
    }

    WifiBssCbData cb_data = { .info = info };
    nl_socket_modify_cb(ctx->sock, NL_CB_VALID, NL_CB_CUSTOM, wifi_bss_valid_cb, &cb_data);

    int rc = nl_send_auto(ctx->sock, msg);
    if (rc >= 0) rc = nl_recvmsgs_default(ctx->sock);

    nlmsg_free(msg);
    return rc < 0 ? -1 : 0;
}

static int read_wifi_station_info(WifiNlCtx *ctx, int ifindex, WifiNlInfo *info)
{
    if (ctx == NULL || ctx->sock == NULL || info == NULL) return -1;

    struct nl_msg *msg = nlmsg_alloc();
    if (!msg) return -1;

    if (!genlmsg_put(msg, 0, 0, ctx->family_id, 0, NLM_F_DUMP, NL80211_CMD_GET_STATION, 0))
    {
        nlmsg_free(msg);
        return -1;
    }

    if (nla_put_u32(msg, NL80211_ATTR_IFINDEX, (unsigned int) ifindex) != 0)
    {
        nlmsg_free(msg);
        return -1;
    }

    WifiStationCbData cb_data = { .info = info };
    nl_socket_modify_cb(ctx->sock, NL_CB_VALID, NL_CB_CUSTOM, wifi_station_valid_cb, &cb_data);

    int rc = nl_send_auto(ctx->sock, msg);
    if (rc >= 0) rc = nl_recvmsgs_default(ctx->sock);

    nlmsg_free(msg);
    return rc < 0 ? -1 : 0;
}

static int read_wifi_nl80211_info(WifiNlCtx *ctx, const char *ifname, WifiNlInfo *info)
{
    if (ctx == NULL || ifname == NULL || info == NULL) return -1;

    init_wifi_nl_info(info);

    unsigned int ifindex = if_nametoindex(ifname);
    if (ifindex == 0) return -1;

    int bss_rc = read_wifi_bss_info(ctx, (int) ifindex, info);
    int sta_rc = read_wifi_station_info(ctx, (int) ifindex, info);

    if (sta_rc == 0 && info->associated) return 0;
    if (bss_rc == 0 && info->associated) return 0;
    return -1;
}
#endif

static int update_empty_wifi_snapshot(WIFI *wifi)
{
    if (wifi == NULL) return -1;
    WIFI_update_data(wifi, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    return 0;
}

void* wifi_event_update(void *arg)
{
    WIFI *wifi = (WIFI *) arg;
    if (wifi == NULL) return NULL;

#ifndef __linux__
    while (1)
    {
        update_empty_wifi_snapshot(wifi);
        sleep(1);
    }
    return NULL;
#else
    char **prev_iface = NULL;
    unsigned long *prev_retry = NULL;
    unsigned long *prev_beacon = NULL;
    size_t prev_count = 0;

    while (1)
    {
        WirelessProcRow *rows = NULL;
        size_t count = 0;

        if (scan_proc_net_wireless(&rows, &count) != 0)
        {
            update_empty_wifi_snapshot(wifi);
            sleep(1);
            continue;
        }

        char **iface = calloc(count ? count : 1, sizeof(*iface));
        char **state = calloc(count ? count : 1, sizeof(*state));
        char **ssid = calloc(count ? count : 1, sizeof(*ssid));
        char **bssid = calloc(count ? count : 1, sizeof(*bssid));
        int *rssi_dbm = malloc((count ? count : 1) * sizeof(*rssi_dbm));
        int *quality_pct = malloc((count ? count : 1) * sizeof(*quality_pct));
        float *tx_bitrate_mbps = malloc((count ? count : 1) * sizeof(*tx_bitrate_mbps));
        float *rx_bitrate_mbps = malloc((count ? count : 1) * sizeof(*rx_bitrate_mbps));
        int *mcs = malloc((count ? count : 1) * sizeof(*mcs));
        float *retries_per_sec = malloc((count ? count : 1) * sizeof(*retries_per_sec));
        float *beacon_loss_per_sec = malloc((count ? count : 1) * sizeof(*beacon_loss_per_sec));

        if (
            !iface || !state || !ssid || !bssid || !rssi_dbm || !quality_pct ||
            !tx_bitrate_mbps || !rx_bitrate_mbps || !mcs || !retries_per_sec || !beacon_loss_per_sec
        )
        {
            free(rows);
            free(iface);
            free(state);
            free(ssid);
            free(bssid);
            free(rssi_dbm);
            free(quality_pct);
            free(tx_bitrate_mbps);
            free(rx_bitrate_mbps);
            free(mcs);
            free(retries_per_sec);
            free(beacon_loss_per_sec);
            sleep(1);
            continue;
        }

#if defined(NET_STATS_HAS_LIBNL)
        WifiNlCtx nl_ctx;
        memset(&nl_ctx, 0, sizeof(nl_ctx));
        int nl_ctx_ready = (open_wifi_nl_ctx(&nl_ctx) == 0);
#endif

        int failed = 0;
        for (size_t i = 0; i < count; ++i)
        {
            iface[i] = strdup(rows[i].iface);
            state[i] = strdup("not connected");
            ssid[i] = strdup("");
            bssid[i] = strdup("");
            if (!iface[i] || !state[i] || !ssid[i] || !bssid[i])
            {
                failed = 1;
                break;
            }

            rssi_dbm[i] = rssi_dbm_from_level(rows[i].level);
            quality_pct[i] = quality_pct_from_link(rows[i].link);
            tx_bitrate_mbps[i] = -1.0f;
            rx_bitrate_mbps[i] = -1.0f;
            mcs[i] = -1;

            int prev_idx = find_prev_iface_index(prev_iface, prev_count, rows[i].iface);
            if (prev_idx >= 0 && rows[i].retry >= prev_retry[prev_idx])
            {
                retries_per_sec[i] = (float) (rows[i].retry - prev_retry[prev_idx]);
            }
            else
            {
                retries_per_sec[i] = 0.0f;
            }

            if (prev_idx >= 0 && rows[i].beacon >= prev_beacon[prev_idx])
            {
                beacon_loss_per_sec[i] = (float) (rows[i].beacon - prev_beacon[prev_idx]);
            }
            else
            {
                beacon_loss_per_sec[i] = 0.0f;
            }

#if defined(NET_STATS_HAS_LIBNL)
            if (nl_ctx_ready)
            {
                WifiNlInfo nl_info;
                if (read_wifi_nl80211_info(&nl_ctx, rows[i].iface, &nl_info) == 0)
                {
                    free(state[i]);
                    state[i] = strdup(nl_info.associated ? "associated" : "not connected");
                    if (!state[i])
                    {
                        failed = 1;
                        break;
                    }

                    if (nl_info.ssid[0] != '\0')
                    {
                        free(ssid[i]);
                        ssid[i] = strdup(nl_info.ssid);
                        if (!ssid[i])
                        {
                            failed = 1;
                            break;
                        }
                    }

                    if (nl_info.bssid[0] != '\0')
                    {
                        free(bssid[i]);
                        bssid[i] = strdup(nl_info.bssid);
                        if (!bssid[i])
                        {
                            failed = 1;
                            break;
                        }
                    }

                    if (nl_info.rssi_dbm != -1) rssi_dbm[i] = nl_info.rssi_dbm;
                    tx_bitrate_mbps[i] = nl_info.tx_bitrate_mbps;
                    rx_bitrate_mbps[i] = nl_info.rx_bitrate_mbps;
                    mcs[i] = nl_info.mcs;
                }
            }
#endif
        }

#if defined(NET_STATS_HAS_LIBNL)
        if (nl_ctx_ready) close_wifi_nl_ctx(&nl_ctx);
#endif

        if (!failed)
        {
            WIFI_update_data(
                wifi,
                (int) count,
                iface,
                state,
                ssid,
                bssid,
                rssi_dbm,
                quality_pct,
                tx_bitrate_mbps,
                rx_bitrate_mbps,
                mcs,
                retries_per_sec,
                beacon_loss_per_sec
            );
        }
        else
        {
            for (size_t i = 0; i < count; ++i)
            {
                free(iface[i]);
                free(state[i]);
                free(ssid[i]);
                free(bssid[i]);
            }
            free(iface);
            free(state);
            free(ssid);
            free(bssid);
            free(rssi_dbm);
            free(quality_pct);
            free(tx_bitrate_mbps);
            free(rx_bitrate_mbps);
            free(mcs);
            free(retries_per_sec);
            free(beacon_loss_per_sec);
        }

        if (store_prev_wireless_counters(rows, count, &prev_iface, &prev_retry, &prev_beacon, &prev_count) != 0)
        {
            fprintf(stderr, "store_prev_wireless_counters failed\n");
        }

        free(rows);
        sleep(1);
    }

    return NULL;
#endif
}
