#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#endif

#include "reactor/arp_route_event_update.h"

#include "data/arp_route_data.h"

typedef struct route_raw
{
    char ifname[32];
    char destination_hex[9];
    char gateway_hex[9];
    char mask_hex[9];
    unsigned int flags;
    unsigned int metric;
} RouteRaw;

typedef struct arp_raw
{
    char ip[64];
    char mac[32];
    char dev[32];
    unsigned int arp_flags;
} ArpRaw;

typedef struct route_row
{
    char kind[16];
    char dst[16];
    uint8_t prefix_len;
    char gateway[16];
    char dev[32];
    unsigned int metric;
    unsigned int flags;
    uint8_t is_default;
} RouteRow;

typedef struct neighbor_row
{
    int family;
    char ip[INET6_ADDRSTRLEN];
    char mac[32];
    char dev[32];
    unsigned int arp_flags;
    char state[16];
    double last_seen_sec;
} NeighborRow;

static int ensure_capacity(void **rows, int *cap, int count, size_t elem_sz)
{
    if (count < *cap) return 0;

    int new_cap = (*cap > 0) ? (*cap * 2) : 8;
    void *tmp = realloc(*rows, (size_t) new_cap * elem_sz);
    if (!tmp) return -1;

    *rows = tmp;
    *cap = new_cap;
    return 0;
}

static int parse_route_line(const char *line, RouteRaw *raw)
{
    char flags_hex[9];
    int n = sscanf(
        line,
        "%31s %8s %8s %8s %*u %*u %u %8s %*u %*u %*u",
        raw->ifname,
        raw->destination_hex,
        raw->gateway_hex,
        flags_hex,
        &raw->metric,
        raw->mask_hex
    );
    if (n != 6) return -1;

    raw->flags = (unsigned int) strtoul(flags_hex, NULL, 16);
    return 0;
}

static int parse_arp_line(const char *line, ArpRaw *raw)
{
    char hw_type[32];
    char flags_hex[32];
    char mask[32];
    int n = sscanf(
        line,
        "%63s %31s %31s %31s %31s %31s",
        raw->ip,
        hw_type,
        flags_hex,
        raw->mac,
        mask,
        raw->dev
    );
    if (n != 6) return -1;

    raw->arp_flags = (unsigned int) strtoul(flags_hex, NULL, 16);
    return 0;
}

static void hex_ipv4_le_to_str(const char *hex, char out[16])
{
    unsigned long value = strtoul(hex, NULL, 16);

    snprintf(
        out,
        16,
        "%lu.%lu.%lu.%lu",
        value & 0xFFul,
        (value >> 8) & 0xFFul,
        (value >> 16) & 0xFFul,
        (value >> 24) & 0xFFul
    );
}

static uint8_t mask_hex_to_prefix_len(const char *hex)
{
    unsigned long value = strtoul(hex, NULL, 16);
    uint8_t prefix_len = 0;

    while (value != 0)
    {
        prefix_len += (uint8_t) (value & 1ul);
        value >>= 1;
    }

    return prefix_len;
}

static int read_route_is_default(const RouteRaw *raw, uint8_t *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = (strcmp(raw->destination_hex, "00000000") == 0 && strcmp(raw->mask_hex, "00000000") == 0) ? 1u : 0u;
    return 0;
}

static int read_route_kind(const RouteRaw *raw, char out[16])
{
    uint8_t is_default;

    if (raw == NULL || out == NULL) return -1;
    if (read_route_is_default(raw, &is_default) != 0) return -1;

    snprintf(out, 16, "%s", is_default ? "default" : "route");
    return 0;
}

static int read_route_dst(const RouteRaw *raw, char out[16])
{
    if (raw == NULL || out == NULL) return -1;

    hex_ipv4_le_to_str(raw->destination_hex, out);
    return 0;
}

static int read_route_prefix_len(const RouteRaw *raw, uint8_t *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = mask_hex_to_prefix_len(raw->mask_hex);
    return 0;
}

static int read_route_gateway(const RouteRaw *raw, char out[16])
{
    if (raw == NULL || out == NULL) return -1;

    out[0] = '\0';
    if ((raw->flags & 0x2u) == 0u) return 0;

    hex_ipv4_le_to_str(raw->gateway_hex, out);
    return 0;
}

static int read_route_dev(const RouteRaw *raw, char out[32])
{
    if (raw == NULL || out == NULL) return -1;

    snprintf(out, 32, "%s", raw->ifname);
    return 0;
}

static int read_route_metric(const RouteRaw *raw, unsigned int *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = raw->metric;
    return 0;
}

static int read_route_flags(const RouteRaw *raw, unsigned int *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = raw->flags;
    return 0;
}

static int read_neighbor_ip(const ArpRaw *raw, char out[INET6_ADDRSTRLEN])
{
    if (raw == NULL || out == NULL) return -1;

    snprintf(out, INET6_ADDRSTRLEN, "%s", raw->ip);
    return 0;
}

static int read_neighbor_mac(const ArpRaw *raw, char out[32])
{
    if (raw == NULL || out == NULL) return -1;

    snprintf(out, 32, "%s", raw->mac);
    return 0;
}

static int read_neighbor_dev(const ArpRaw *raw, char out[32])
{
    if (raw == NULL || out == NULL) return -1;

    snprintf(out, 32, "%s", raw->dev);
    return 0;
}

static int read_neighbor_arp_flags(const ArpRaw *raw, unsigned int *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = raw->arp_flags;
    return 0;
}

static int read_neighbor_default_state(char out[16])
{
    if (out == NULL) return -1;

    snprintf(out, 16, "%s", "UNKNOWN");
    return 0;
}

static int read_neighbor_default_last_seen(double *out)
{
    if (out == NULL) return -1;

    *out = -1.0;
    return 0;
}

static int read_route_rows(RouteRow **rows_out, int *count_out)
{
    FILE *f = fopen("/proc/net/route", "r");
    if (!f) return -1;

    char line[512];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    RouteRow *rows = NULL;
    int count = 0;
    int cap = 0;

    while (fgets(line, sizeof(line), f))
    {
        RouteRaw raw;
        RouteRow row;

        memset(&raw, 0, sizeof(raw));
        memset(&row, 0, sizeof(row));

        if (parse_route_line(line, &raw) != 0) continue;
        if (read_route_is_default(&raw, &row.is_default) != 0) continue;
        if (read_route_kind(&raw, row.kind) != 0) continue;
        if (read_route_dst(&raw, row.dst) != 0) continue;
        if (read_route_prefix_len(&raw, &row.prefix_len) != 0) continue;
        if (read_route_gateway(&raw, row.gateway) != 0) continue;
        if (read_route_dev(&raw, row.dev) != 0) continue;
        if (read_route_metric(&raw, &row.metric) != 0) continue;
        if (read_route_flags(&raw, &row.flags) != 0) continue;

        if (ensure_capacity((void **) &rows, &cap, count, sizeof(*rows)) != 0)
        {
            fclose(f);
            free(rows);
            return -1;
        }

        rows[count++] = row;
    }

    fclose(f);
    *rows_out = rows;
    *count_out = count;
    return 0;
}

static int read_arp_neighbor_rows(NeighborRow **rows_out, int *count_out)
{
    FILE *f = fopen("/proc/net/arp", "r");
    if (!f) return -1;

    char line[512];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    NeighborRow *rows = NULL;
    int count = 0;
    int cap = 0;

    while (fgets(line, sizeof(line), f))
    {
        ArpRaw raw;
        NeighborRow row;

        memset(&raw, 0, sizeof(raw));
        memset(&row, 0, sizeof(row));

        if (parse_arp_line(line, &raw) != 0) continue;

        row.family = AF_INET;
        if (read_neighbor_ip(&raw, row.ip) != 0) continue;
        if (read_neighbor_mac(&raw, row.mac) != 0) continue;
        if (read_neighbor_dev(&raw, row.dev) != 0) continue;
        if (read_neighbor_arp_flags(&raw, &row.arp_flags) != 0) continue;
        if (read_neighbor_default_state(row.state) != 0) continue;
        if (read_neighbor_default_last_seen(&row.last_seen_sec) != 0) continue;

        if (ensure_capacity((void **) &rows, &cap, count, sizeof(*rows)) != 0)
        {
            fclose(f);
            free(rows);
            return -1;
        }

        rows[count++] = row;
    }

    fclose(f);
    *rows_out = rows;
    *count_out = count;
    return 0;
}

static int read_netlink_neighbor_state(unsigned short ndm_state, char out[16])
{
    const char *state = "UNKNOWN";

#if defined(__linux__)
    if ((ndm_state & NUD_PERMANENT) != 0u) state = "PERMANENT";
    else if ((ndm_state & NUD_REACHABLE) != 0u) state = "REACHABLE";
    else if ((ndm_state & NUD_STALE) != 0u) state = "STALE";
    else if ((ndm_state & NUD_DELAY) != 0u) state = "DELAY";
    else if ((ndm_state & NUD_PROBE) != 0u) state = "PROBE";
    else if ((ndm_state & NUD_FAILED) != 0u) state = "FAILED";
    else if ((ndm_state & NUD_INCOMPLETE) != 0u) state = "INCOMPLETE";
    else if ((ndm_state & NUD_NOARP) != 0u) state = "NOARP";
    else if ((ndm_state & NUD_NONE) != 0u) state = "NONE";
#else
    (void) ndm_state;
#endif

    if (out == NULL) return -1;

    snprintf(out, 16, "%s", state);
    return 0;
}

static int read_netlink_neighbor_mac(const unsigned char *addr, size_t addr_len, char out[32])
{
    if (out == NULL) return -1;

    if (addr == NULL || addr_len == 0)
    {
        out[0] = '\0';
        return 0;
    }

    size_t pos = 0;
    for (size_t i = 0; i < addr_len; ++i)
    {
        int written = snprintf(
            out + pos,
            32 - pos,
            (i + 1 < addr_len) ? "%02x:" : "%02x",
            addr[i]
        );
        if (written < 0) return -1;
        if ((size_t) written >= 32 - pos)
        {
            out[31] = '\0';
            return 0;
        }
        pos += (size_t) written;
    }

    out[pos] = '\0';
    return 0;
}

static int read_netlink_neighbor_ip(int family, const void *data, char out[INET6_ADDRSTRLEN])
{
    if (data == NULL || out == NULL) return -1;

    if (inet_ntop(family, data, out, INET6_ADDRSTRLEN) == NULL) return -1;
    return 0;
}

static int read_netlink_neighbor_dev(unsigned int ifindex, char out[32])
{
    if (out == NULL) return -1;

    if (if_indextoname(ifindex, out) == NULL) return -1;
    return 0;
}

#if defined(__linux__)
static int read_netlink_neighbor_last_seen(const struct nda_cacheinfo *cacheinfo, double *out)
{
    if (out == NULL) return -1;

    if (cacheinfo == NULL)
    {
        *out = -1.0;
        return 0;
    }

    long hz = sysconf(_SC_CLK_TCK);
    if (hz <= 0) hz = 100;

    *out = (double) cacheinfo->ndm_confirmed / (double) hz;
    return 0;
}
#else
static int read_netlink_neighbor_last_seen(const void *cacheinfo, double *out)
{
    (void) cacheinfo;
    return read_neighbor_default_last_seen(out);
}
#endif

static void free_mixed_payload(
    int count,
    uint8_t *row_type,
    char **route_kind,
    char **route_dst,
    uint8_t *route_prefix_len,
    char **route_gateway,
    char **route_dev,
    unsigned int *route_metric,
    unsigned int *route_flags,
    uint8_t *route_is_default,
    char **neighbor_ip,
    char **neighbor_mac,
    char **neighbor_dev,
    unsigned int *neighbor_arp_flags,
    char **neighbor_state,
    double *neighbor_last_seen_sec
)
{
    if (route_kind != NULL)
    {
        for (int i = 0; i < count; ++i) free(route_kind[i]);
        free(route_kind);
    }
    if (route_dst != NULL)
    {
        for (int i = 0; i < count; ++i) free(route_dst[i]);
        free(route_dst);
    }
    if (route_gateway != NULL)
    {
        for (int i = 0; i < count; ++i) free(route_gateway[i]);
        free(route_gateway);
    }
    if (route_dev != NULL)
    {
        for (int i = 0; i < count; ++i) free(route_dev[i]);
        free(route_dev);
    }
    if (neighbor_ip != NULL)
    {
        for (int i = 0; i < count; ++i) free(neighbor_ip[i]);
        free(neighbor_ip);
    }
    if (neighbor_mac != NULL)
    {
        for (int i = 0; i < count; ++i) free(neighbor_mac[i]);
        free(neighbor_mac);
    }
    if (neighbor_dev != NULL)
    {
        for (int i = 0; i < count; ++i) free(neighbor_dev[i]);
        free(neighbor_dev);
    }
    if (neighbor_state != NULL)
    {
        for (int i = 0; i < count; ++i) free(neighbor_state[i]);
        free(neighbor_state);
    }

    free(row_type);
    free(route_prefix_len);
    free(route_metric);
    free(route_flags);
    free(route_is_default);
    free(neighbor_arp_flags);
    free(neighbor_last_seen_sec);
}

static int update_ipv4_neighbor_from_netlink(
    NeighborRow *rows,
    int count,
    const char *ip,
    const char *dev,
    const char *mac,
    const char *state,
    double last_seen_sec
)
{
    for (int i = 0; i < count; ++i)
    {
        if (rows[i].family != AF_INET) continue;
        if (strcmp(rows[i].ip, ip) != 0) continue;
        if (strcmp(rows[i].dev, dev) != 0) continue;

        if (mac != NULL && *mac != '\0')
        {
            snprintf(rows[i].mac, sizeof(rows[i].mac), "%s", mac);
        }
        if (state != NULL && *state != '\0')
        {
            snprintf(rows[i].state, sizeof(rows[i].state), "%s", state);
        }
        if (last_seen_sec >= 0.0)
        {
            rows[i].last_seen_sec = last_seen_sec;
        }
        return 1;
    }

    return 0;
}

#if defined(__linux__)
static int enrich_neighbors_with_netlink(
    NeighborRow *arp_rows,
    int arp_count,
    NeighborRow **ipv6_rows_out,
    int *ipv6_count_out
)
{
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) return -1;

    struct sockaddr_nl local_addr;
    struct sockaddr_nl kernel_addr;

    memset(&local_addr, 0, sizeof(local_addr));
    memset(&kernel_addr, 0, sizeof(kernel_addr));
    local_addr.nl_family = AF_NETLINK;
    kernel_addr.nl_family = AF_NETLINK;

    if (bind(fd, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0)
    {
        close(fd);
        return -1;
    }

    struct
    {
        struct nlmsghdr nlh;
        struct ndmsg ndm;
    } req;

    memset(&req, 0, sizeof(req));
    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
    req.nlh.nlmsg_type = RTM_GETNEIGH;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.nlh.nlmsg_seq = 1;
    req.ndm.ndm_family = AF_UNSPEC;

    if (
        sendto(
            fd,
            &req,
            req.nlh.nlmsg_len,
            0,
            (struct sockaddr *) &kernel_addr,
            sizeof(kernel_addr)
        ) < 0
    )
    {
        close(fd);
        return -1;
    }

    NeighborRow *ipv6_rows = NULL;
    int ipv6_count = 0;
    int ipv6_cap = 0;
    int done = 0;

    while (!done)
    {
        char buf[8192];
        ssize_t recv_len = recv(fd, buf, sizeof(buf), 0);
        if (recv_len < 0)
        {
            free(ipv6_rows);
            close(fd);
            return -1;
        }

        int remaining = (int) recv_len;
        for (
            struct nlmsghdr *nlh = (struct nlmsghdr *) buf;
            NLMSG_OK(nlh, remaining);
            nlh = NLMSG_NEXT(nlh, remaining)
        )
        {
            if (nlh->nlmsg_type == NLMSG_DONE)
            {
                done = 1;
                break;
            }

            if (nlh->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA(nlh);
                free(ipv6_rows);
                close(fd);
                errno = (err->error != 0) ? -err->error : EPROTO;
                return -1;
            }

            if (nlh->nlmsg_type != RTM_NEWNEIGH) continue;

            struct ndmsg *ndm = (struct ndmsg *) NLMSG_DATA(nlh);
            if (ndm->ndm_family != AF_INET && ndm->ndm_family != AF_INET6) continue;

            char ip[INET6_ADDRSTRLEN];
            char mac[32];
            char dev[32];
            char state[16];
            double last_seen_sec;
            const struct nda_cacheinfo *cacheinfo = NULL;
            int have_dst = 0;
            int attr_len = NLMSG_PAYLOAD(nlh, sizeof(*ndm));

            ip[0] = '\0';
            mac[0] = '\0';
            dev[0] = '\0';
            state[0] = '\0';
            last_seen_sec = -1.0;

            for (
                struct rtattr *attr = (struct rtattr *) (((char *) ndm) + NLMSG_ALIGN(sizeof(*ndm)));
                RTA_OK(attr, attr_len);
                attr = RTA_NEXT(attr, attr_len)
            )
            {
                if (
                    attr->rta_type == NDA_DST &&
                    read_netlink_neighbor_ip(ndm->ndm_family, RTA_DATA(attr), ip) == 0
                )
                {
                    have_dst = 1;
                }
                else if (attr->rta_type == NDA_LLADDR)
                {
                    if (
                        read_netlink_neighbor_mac(
                            (const unsigned char *) RTA_DATA(attr),
                            (size_t) RTA_PAYLOAD(attr),
                            mac
                        ) != 0
                    )
                    {
                        mac[0] = '\0';
                    }
                }
                else if (
                    attr->rta_type == NDA_CACHEINFO &&
                    RTA_PAYLOAD(attr) >= (int) sizeof(struct nda_cacheinfo)
                )
                {
                    cacheinfo = (const struct nda_cacheinfo *) RTA_DATA(attr);
                }
            }

            if (!have_dst) continue;
            if (read_netlink_neighbor_dev((unsigned int) ndm->ndm_ifindex, dev) != 0) continue;
            if (read_netlink_neighbor_state(ndm->ndm_state, state) != 0) continue;
            if (read_netlink_neighbor_last_seen(cacheinfo, &last_seen_sec) != 0) continue;

            if (ndm->ndm_family == AF_INET)
            {
                (void) update_ipv4_neighbor_from_netlink(
                    arp_rows,
                    arp_count,
                    ip,
                    dev,
                    mac,
                    state,
                    last_seen_sec
                );
                continue;
            }

            NeighborRow row;

            memset(&row, 0, sizeof(row));
            row.family = AF_INET6;
            snprintf(row.ip, sizeof(row.ip), "%s", ip);
            snprintf(row.mac, sizeof(row.mac), "%s", mac);
            snprintf(row.dev, sizeof(row.dev), "%s", dev);
            snprintf(row.state, sizeof(row.state), "%s", state);
            row.arp_flags = 0;
            row.last_seen_sec = last_seen_sec;

            if (ensure_capacity((void **) &ipv6_rows, &ipv6_cap, ipv6_count, sizeof(*ipv6_rows)) != 0)
            {
                free(ipv6_rows);
                close(fd);
                return -1;
            }

            ipv6_rows[ipv6_count++] = row;
        }
    }

    close(fd);
    *ipv6_rows_out = ipv6_rows;
    *ipv6_count_out = ipv6_count;
    return 0;
}
#else
static int enrich_neighbors_with_netlink(
    NeighborRow *arp_rows,
    int arp_count,
    NeighborRow **ipv6_rows_out,
    int *ipv6_count_out
)
{
    (void) arp_rows;
    (void) arp_count;
    *ipv6_rows_out = NULL;
    *ipv6_count_out = 0;
    return 0;
}
#endif

static int build_mixed_payload(
    RouteRow *route_rows,
    int route_count,
    NeighborRow *arp_rows,
    int arp_count,
    NeighborRow *ipv6_rows,
    int ipv6_count,
    int *count_out,
    uint8_t **row_type_out,
    char ***route_kind_out,
    char ***route_dst_out,
    uint8_t **route_prefix_len_out,
    char ***route_gateway_out,
    char ***route_dev_out,
    unsigned int **route_metric_out,
    unsigned int **route_flags_out,
    uint8_t **route_is_default_out,
    char ***neighbor_ip_out,
    char ***neighbor_mac_out,
    char ***neighbor_dev_out,
    unsigned int **neighbor_arp_flags_out,
    char ***neighbor_state_out,
    double **neighbor_last_seen_sec_out
)
{
    int total = route_count + arp_count + ipv6_count;
    uint8_t *row_type = NULL;
    char **route_kind = NULL;
    char **route_dst = NULL;
    uint8_t *route_prefix_len = NULL;
    char **route_gateway = NULL;
    char **route_dev = NULL;
    unsigned int *route_metric = NULL;
    unsigned int *route_flags = NULL;
    uint8_t *route_is_default = NULL;
    char **neighbor_ip = NULL;
    char **neighbor_mac = NULL;
    char **neighbor_dev = NULL;
    unsigned int *neighbor_arp_flags = NULL;
    char **neighbor_state = NULL;
    double *neighbor_last_seen_sec = NULL;
    int idx = 0;

    *count_out = 0;
    *row_type_out = NULL;
    *route_kind_out = NULL;
    *route_dst_out = NULL;
    *route_prefix_len_out = NULL;
    *route_gateway_out = NULL;
    *route_dev_out = NULL;
    *route_metric_out = NULL;
    *route_flags_out = NULL;
    *route_is_default_out = NULL;
    *neighbor_ip_out = NULL;
    *neighbor_mac_out = NULL;
    *neighbor_dev_out = NULL;
    *neighbor_arp_flags_out = NULL;
    *neighbor_state_out = NULL;
    *neighbor_last_seen_sec_out = NULL;

    if (total == 0) return 0;

    row_type = calloc((size_t) total, sizeof(*row_type));
    route_kind = calloc((size_t) total, sizeof(*route_kind));
    route_dst = calloc((size_t) total, sizeof(*route_dst));
    route_prefix_len = calloc((size_t) total, sizeof(*route_prefix_len));
    route_gateway = calloc((size_t) total, sizeof(*route_gateway));
    route_dev = calloc((size_t) total, sizeof(*route_dev));
    route_metric = calloc((size_t) total, sizeof(*route_metric));
    route_flags = calloc((size_t) total, sizeof(*route_flags));
    route_is_default = calloc((size_t) total, sizeof(*route_is_default));
    neighbor_ip = calloc((size_t) total, sizeof(*neighbor_ip));
    neighbor_mac = calloc((size_t) total, sizeof(*neighbor_mac));
    neighbor_dev = calloc((size_t) total, sizeof(*neighbor_dev));
    neighbor_arp_flags = calloc((size_t) total, sizeof(*neighbor_arp_flags));
    neighbor_state = calloc((size_t) total, sizeof(*neighbor_state));
    neighbor_last_seen_sec = calloc((size_t) total, sizeof(*neighbor_last_seen_sec));
    if (
        row_type == NULL || route_kind == NULL || route_dst == NULL ||
        route_prefix_len == NULL || route_gateway == NULL || route_dev == NULL ||
        route_metric == NULL || route_flags == NULL || route_is_default == NULL ||
        neighbor_ip == NULL || neighbor_mac == NULL || neighbor_dev == NULL ||
        neighbor_arp_flags == NULL || neighbor_state == NULL ||
        neighbor_last_seen_sec == NULL
    )
    {
        free_mixed_payload(
            total,
            row_type,
            route_kind,
            route_dst,
            route_prefix_len,
            route_gateway,
            route_dev,
            route_metric,
            route_flags,
            route_is_default,
            neighbor_ip,
            neighbor_mac,
            neighbor_dev,
            neighbor_arp_flags,
            neighbor_state,
            neighbor_last_seen_sec
        );
        return -1;
    }

    for (int i = 0; i < route_count; ++i, ++idx)
    {
        row_type[idx] = ARPRT_ROW_ROUTE;
        route_kind[idx] = strdup(route_rows[i].kind);
        route_dst[idx] = strdup(route_rows[i].dst);
        route_prefix_len[idx] = route_rows[i].prefix_len;
        route_gateway[idx] = strdup(route_rows[i].gateway);
        route_dev[idx] = strdup(route_rows[i].dev);
        route_metric[idx] = route_rows[i].metric;
        route_flags[idx] = route_rows[i].flags;
        route_is_default[idx] = route_rows[i].is_default;
        neighbor_ip[idx] = strdup("");
        neighbor_mac[idx] = strdup("");
        neighbor_dev[idx] = strdup("");
        neighbor_state[idx] = strdup("");
        neighbor_arp_flags[idx] = 0;
        neighbor_last_seen_sec[idx] = -1.0;
        if (
            route_kind[idx] == NULL || route_dst[idx] == NULL ||
            route_gateway[idx] == NULL || route_dev[idx] == NULL ||
            neighbor_ip[idx] == NULL || neighbor_mac[idx] == NULL ||
            neighbor_dev[idx] == NULL || neighbor_state[idx] == NULL
        )
        {
            free_mixed_payload(
                total,
                row_type,
                route_kind,
                route_dst,
                route_prefix_len,
                route_gateway,
                route_dev,
                route_metric,
                route_flags,
                route_is_default,
                neighbor_ip,
                neighbor_mac,
                neighbor_dev,
                neighbor_arp_flags,
                neighbor_state,
                neighbor_last_seen_sec
            );
            return -1;
        }
    }

    for (int i = 0; i < arp_count; ++i, ++idx)
    {
        row_type[idx] = ARPRT_ROW_NEIGHBOR;
        route_kind[idx] = strdup("");
        route_dst[idx] = strdup("");
        route_prefix_len[idx] = 0;
        route_gateway[idx] = strdup("");
        route_dev[idx] = strdup("");
        route_metric[idx] = 0;
        route_flags[idx] = 0;
        route_is_default[idx] = 0;
        neighbor_ip[idx] = strdup(arp_rows[i].ip);
        neighbor_mac[idx] = strdup(arp_rows[i].mac);
        neighbor_dev[idx] = strdup(arp_rows[i].dev);
        neighbor_state[idx] = strdup(arp_rows[i].state);
        neighbor_arp_flags[idx] = arp_rows[i].arp_flags;
        neighbor_last_seen_sec[idx] = arp_rows[i].last_seen_sec;
        if (
            route_kind[idx] == NULL || route_dst[idx] == NULL ||
            route_gateway[idx] == NULL || route_dev[idx] == NULL ||
            neighbor_ip[idx] == NULL || neighbor_mac[idx] == NULL ||
            neighbor_dev[idx] == NULL || neighbor_state[idx] == NULL
        )
        {
            free_mixed_payload(
                total,
                row_type,
                route_kind,
                route_dst,
                route_prefix_len,
                route_gateway,
                route_dev,
                route_metric,
                route_flags,
                route_is_default,
                neighbor_ip,
                neighbor_mac,
                neighbor_dev,
                neighbor_arp_flags,
                neighbor_state,
                neighbor_last_seen_sec
            );
            return -1;
        }
    }

    for (int i = 0; i < ipv6_count; ++i, ++idx)
    {
        row_type[idx] = ARPRT_ROW_NEIGHBOR;
        route_kind[idx] = strdup("");
        route_dst[idx] = strdup("");
        route_prefix_len[idx] = 0;
        route_gateway[idx] = strdup("");
        route_dev[idx] = strdup("");
        route_metric[idx] = 0;
        route_flags[idx] = 0;
        route_is_default[idx] = 0;
        neighbor_ip[idx] = strdup(ipv6_rows[i].ip);
        neighbor_mac[idx] = strdup(ipv6_rows[i].mac);
        neighbor_dev[idx] = strdup(ipv6_rows[i].dev);
        neighbor_state[idx] = strdup(ipv6_rows[i].state);
        neighbor_arp_flags[idx] = ipv6_rows[i].arp_flags;
        neighbor_last_seen_sec[idx] = ipv6_rows[i].last_seen_sec;
        if (
            route_kind[idx] == NULL || route_dst[idx] == NULL ||
            route_gateway[idx] == NULL || route_dev[idx] == NULL ||
            neighbor_ip[idx] == NULL || neighbor_mac[idx] == NULL ||
            neighbor_dev[idx] == NULL || neighbor_state[idx] == NULL
        )
        {
            free_mixed_payload(
                total,
                row_type,
                route_kind,
                route_dst,
                route_prefix_len,
                route_gateway,
                route_dev,
                route_metric,
                route_flags,
                route_is_default,
                neighbor_ip,
                neighbor_mac,
                neighbor_dev,
                neighbor_arp_flags,
                neighbor_state,
                neighbor_last_seen_sec
            );
            return -1;
        }
    }

    *count_out = total;
    *row_type_out = row_type;
    *route_kind_out = route_kind;
    *route_dst_out = route_dst;
    *route_prefix_len_out = route_prefix_len;
    *route_gateway_out = route_gateway;
    *route_dev_out = route_dev;
    *route_metric_out = route_metric;
    *route_flags_out = route_flags;
    *route_is_default_out = route_is_default;
    *neighbor_ip_out = neighbor_ip;
    *neighbor_mac_out = neighbor_mac;
    *neighbor_dev_out = neighbor_dev;
    *neighbor_arp_flags_out = neighbor_arp_flags;
    *neighbor_state_out = neighbor_state;
    *neighbor_last_seen_sec_out = neighbor_last_seen_sec;
    return 0;
}

void* arp_route_event_update(void *arg)
{
    ARPRT *arprt = (ARPRT *) arg;

    while (1)
    {
        RouteRow *route_rows = NULL;
        NeighborRow *arp_rows = NULL;
        NeighborRow *ipv6_rows = NULL;
        int route_count = 0;
        int arp_count = 0;
        int ipv6_count = 0;

        uint8_t *row_type = NULL;
        char **route_kind = NULL;
        char **route_dst = NULL;
        uint8_t *route_prefix_len = NULL;
        char **route_gateway = NULL;
        char **route_dev = NULL;
        unsigned int *route_metric = NULL;
        unsigned int *route_flags = NULL;
        uint8_t *route_is_default = NULL;
        char **neighbor_ip = NULL;
        char **neighbor_mac = NULL;
        char **neighbor_dev = NULL;
        unsigned int *neighbor_arp_flags = NULL;
        char **neighbor_state = NULL;
        double *neighbor_last_seen_sec = NULL;
        int total = 0;

        if (read_route_rows(&route_rows, &route_count) != 0)
        {
            perror("read_route_rows");
            sleep(1);
            continue;
        }

        if (read_arp_neighbor_rows(&arp_rows, &arp_count) != 0)
        {
            perror("read_arp_neighbor_rows");
            free(route_rows);
            sleep(1);
            continue;
        }

        if (enrich_neighbors_with_netlink(arp_rows, arp_count, &ipv6_rows, &ipv6_count) != 0)
        {
            perror("enrich_neighbors_with_netlink");
            ipv6_rows = NULL;
            ipv6_count = 0;
        }

        if (
            build_mixed_payload(
                route_rows,
                route_count,
                arp_rows,
                arp_count,
                ipv6_rows,
                ipv6_count,
                &total,
                &row_type,
                &route_kind,
                &route_dst,
                &route_prefix_len,
                &route_gateway,
                &route_dev,
                &route_metric,
                &route_flags,
                &route_is_default,
                &neighbor_ip,
                &neighbor_mac,
                &neighbor_dev,
                &neighbor_arp_flags,
                &neighbor_state,
                &neighbor_last_seen_sec
            ) != 0
        )
        {
            perror("build_mixed_payload");
            free(route_rows);
            free(arp_rows);
            free(ipv6_rows);
            sleep(1);
            continue;
        }

        ARPRT_update_data(
            arprt,
            total,
            row_type,
            route_kind,
            route_dst,
            route_prefix_len,
            route_gateway,
            route_dev,
            route_metric,
            route_flags,
            route_is_default,
            neighbor_ip,
            neighbor_mac,
            neighbor_dev,
            neighbor_arp_flags,
            neighbor_state,
            neighbor_last_seen_sec
        );

        free(route_rows);
        free(arp_rows);
        free(ipv6_rows);
        sleep(1);
    }

    return NULL;
}
