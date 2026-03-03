#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "reactor/interfaces_event_update.h"

#include "data/interfaces_data.h"

#if defined(__has_include)
#  if __has_include(<linux/if_arp.h>)
#    include <linux/if_arp.h>
#    define HAVE_ARPHRD_HEADER 1
#  elif __has_include(<net/if_arp.h>)
#    include <net/if_arp.h>
#    define HAVE_ARPHRD_HEADER 1
#  else
#    define HAVE_ARPHRD_HEADER 0
#  endif
#else
#  ifndef HAVE_ARPHRD_HEADER
#    define HAVE_ARPHRD_HEADER 0
#  endif
#  if HAVE_ARPHRD_HEADER
#    include <linux/if_arp.h>
#  endif
#endif

static const char* iftype_to_str_num(unsigned int t)
{
    switch (t) 
    {
        case 1: return "ether";        /* ARPHRD_ETHER */
        case 772: return "loopback";     /* ARPHRD_LOOPBACK */
        case 512: return "ppp";          /* ARPHRD_PPP */
        case 768: return "ipip";         /* ARPHRD_TUNNEL (часто) */
        case 769: return "ip6tnl";       /* ARPHRD_TUNNEL6 (часто) */
        case 776: return "sit";          /* ARPHRD_SIT */
        case 778: return "gre";          /* ARPHRD_IPGRE (часто) */
        case 65534: return "none";         /* ARPHRD_NONE */
        case 65535: return "void";         /* ARPHRD_VOID */
        default: return "UNKNOWN";
    }
}

const char* iftype_to_str(unsigned int t)
{
#if HAVE_ARPHRD_HEADER
    switch (t) 
    {
    #ifdef ARPHRD_ETHER
        case ARPHRD_ETHER: return "ether";
    #endif
    #ifdef ARPHRD_LOOPBACK
        case ARPHRD_LOOPBACK: return "loopback";
    #endif
    #ifdef ARPHRD_PPP
        case ARPHRD_PPP: return "ppp";
    #endif
    #ifdef ARPHRD_TUNNEL
        case ARPHRD_TUNNEL: return "ipip";
    #endif
    #ifdef ARPHRD_TUNNEL6
        case ARPHRD_TUNNEL6: return "ip6tnl";
    #endif
    #ifdef ARPHRD_SIT
        case ARPHRD_SIT: return "sit";
    #endif
    #ifdef ARPHRD_IPGRE
        case ARPHRD_IPGRE: return "gre";
    #endif
    #ifdef ARPHRD_NONE
        case ARPHRD_NONE: return "none";
    #endif
    #ifdef ARPHRD_VOID
        case ARPHRD_VOID: return "void";
    #endif
        default: return iftype_to_str_num(t);
    }
#else
    return iftype_to_str_num(t);
#endif
}

static uint64_t read_net_stats_by_file(const char *interf_name, const char *file_name)
{
    char path[PATH_MAX];

    if (interf_name == NULL || *interf_name == '\0') return UINT64_MAX;
    if (file_name == NULL) return UINT64_MAX;
    if (strchr(interf_name, '/')) return UINT64_MAX;
    if (*file_name == '\0' || strchr(file_name, '/')) return UINT64_MAX;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/statistics/%s",
        interf_name,
        file_name
    );
    if (path_len < 0 || path_len >= (int)sizeof(path))
    {
        return UINT64_MAX;
    }
    
    FILE *f = fopen(path, "r");
    if (!f) return UINT64_MAX;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return UINT64_MAX;
    }
    fclose(f);

    errno = 0;
    char *end = NULL;
    unsigned long long vl = strtoull(buf, &end, 10);
    if (errno != 0 || end == buf) return UINT64_MAX;

    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
    if (*end != '\0') return UINT64_MAX;

    return (uint64_t) vl;
}

static int32_t read_ifname_link_speed(const char *ifname)
{
    char path[PATH_MAX];

    if (ifname == NULL || *ifname == '\0') return -1;
    if (strchr(ifname, '/')) return -1;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/speed",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    errno = 0;
    char *end = NULL;
    long vl = strtol(buf, &end, 10);
    if (errno != 0 || end == buf) return -1;

    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
    if (*end != '\0') return -1;

    if (vl < 0 || vl > INT32_MAX) return -1;

    return (int32_t) vl;
}

static int read_ifname_duplex(const char *ifname, char *out, size_t out_sz)
{
    char path[PATH_MAX];

    if (ifname == NULL || *ifname == '\0') return -1;
    if (out == NULL || out_sz == 0) return -1;
    if (strchr(ifname, '/')) return -1;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/duplex",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(out, out_sz, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    size_t len = strcspn(out, "\r\n");
    out[len] = '\0';

    return 0;
}

static int read_ifname_carrier(const char *ifname)
{
    char path[PATH_MAX];

    if (ifname == NULL || *ifname == '\0') return -1;
    if (strchr(ifname, '/')) return -1;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/carrier",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    errno = 0;
    char *end = NULL;
    long vl = strtol(buf, &end, 10);
    if (errno != 0 || end == buf) return -1;

    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
    if (*end != '\0') return -1;

    if (vl != 0 && vl != 1) return -1;

    return (int) vl;
}

static int read_ifname_operstate(const char *ifname, char *out, size_t out_sz)
{
    char path[PATH_MAX];

    if (ifname == NULL || *ifname == '\0') return -1;
    if (out == NULL || out_sz == 0) return -1;
    if (strchr(ifname, '/')) return -1;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/operstate",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(out, out_sz, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    size_t len = strcspn(out, "\r\n");
    out[len] = '\0';

    return 0;
}

int get_master_ifname(const char *ifname, char *out, size_t out_sz)
{
    if (!ifname || !out || out_sz == 0) 
    {
        perror("incorrect input data");
        return -1;
    }

    char path[PATH_MAX];
    char resolved[PATH_MAX];

    int n = snprintf(path, sizeof(path), "/sys/class/net/%s/master", ifname);
    if (n < 0 || (size_t) n > sizeof(path))
    {
        perror("too long master path");
        return -1;
    }

    if (realpath(path, resolved) == NULL)
    {
        return -1;
    }

    const char *base = strrchr(resolved, '/');
    base = (base != NULL) ? base + 1 : resolved;
    if (*base == '\0')
    {
        perror("Not found master name");
        return -1;
    }
    else if (strlen(base) + 1 > out_sz)
    {
        perror("Incorrect master name");
        return -1;
    }
    
    strcpy(out, base);
    return 0;
}

int32_t read_ifname_mtu(const char *ifname)
{
    char path[PATH_MAX];

    if (ifname == NULL || *ifname == '\0') return -1;
    if (strchr(ifname, '/')) return -1;

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/mtu",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    errno = 0;
    char *end = NULL;
    long vl = strtol(buf, &end, 10);
    if (errno != 0 || end == buf) return -1;

    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r') ++end;
    if (*end != '\0') return -1;

    if (vl < 0 || vl > INT32_MAX) return -1;

    return (int32_t) vl;
}

int read_ifname_mac_address(const char *ifname, uint8_t out[6])
{
    if (!ifname || *ifname == '\0' || !out) 
    {
        perror("incorrect input data");
        return -1;
    }
    if (strchr(ifname, '/')) return -1;

    char path[PATH_MAX];

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/address",
        ifname
    );
    if (path_len < 0 || path_len >= (int) sizeof(path))
    {
        return -1;
    }

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[64];
    if (!fgets(buf, sizeof(buf), f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    unsigned x[6];
    if (sscanf(
        buf, 
        "%2x:%2x:%2x:%2x:%2x:%2x",
        &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]
    ) != 6)
    {
        return -1;
    }

    for (size_t i = 0; i < 6; ++i) out[i] = (uint8_t) x[i];
    return 0;
}

static int parse_hex_byte(const char *hex2, uint8_t *out)
{
    if (!hex2 || !out) return -1;

    unsigned int v = 0;
    if (sscanf(hex2, "%2x", &v) != 1) return -1;
    if (v > 0xFF) return -1;
    *out = (uint8_t)v;
    return 0;
}

int read_ifname_ipv4_address(const char *ifname, uint8_t out[4])
{
    if (!ifname || *ifname == '\0' || !out) return -1;

    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    int found = 0;
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_name || strcmp(ifa->ifa_name, ifname) != 0) continue;
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

        const struct sockaddr_in *sin = (const struct sockaddr_in *) ifa->ifa_addr;
        const uint8_t *src = (const uint8_t *)&sin->sin_addr;
        for (int i = 0; i < 4; ++i) out[i] = src[i];
        found = 1;
        break;
    }

    freeifaddrs(ifaddr);
    return found ? 0 : -1;
}

int read_ifname_ipv6_address(const char *ifname, uint8_t out[16])
{
    if (!ifname || *ifname == '\0' || !out) return -1;

    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    int found = 0;
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_name || strcmp(ifa->ifa_name, ifname) != 0) continue;
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET6) continue;

        const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *) ifa->ifa_addr;
        if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) continue;

        const uint8_t *src = (const uint8_t *)&sin6->sin6_addr;
        for (int i = 0; i < 16; ++i) out[i] = src[i];
        found = 1;
        break;
    }

    if (!found)
    {
        for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_name || strcmp(ifa->ifa_name, ifname) != 0) continue;
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET6) continue;

            const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)ifa->ifa_addr;
            const uint8_t *src = (const uint8_t *)&sin6->sin6_addr;
            for (int i = 0; i < 16; ++i) out[i] = src[i];
            found = 1;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return found ? 0 : -1;
}

int read_ifname_gw_ipv4_address(const char *ifname, uint8_t out[4])
{
    if (!ifname || *ifname == '\0' || !out) return -1;

    FILE *f = fopen("/proc/net/route", "r");
    if (!f) return -1;

    char line[512];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    int found = 0;
    while (fgets(line, sizeof(line), f))
    {
        char iface[IFNAMSIZ];
        unsigned long dest = 0;
        unsigned long gate = 0;
        unsigned int flags = 0;

        int n = sscanf(line, "%15s %lx %lx %x", iface, &dest, &gate, &flags);
        if (n < 4) continue;
        if (strcmp(iface, ifname) != 0) continue;
        if (dest != 0) continue;
        if ((flags & 0x1u) == 0 || (flags & 0x2u) == 0) continue;

        out[0] = (uint8_t)(gate & 0xFFu);
        out[1] = (uint8_t)((gate >> 8) & 0xFFu);
        out[2] = (uint8_t)((gate >> 16) & 0xFFu);
        out[3] = (uint8_t)((gate >> 24) & 0xFFu);
        found = 1;
        break;
    }

    fclose(f);
    return found ? 0 : -1;
}

int read_ifname_gw_ipv6_address(const char *ifname, uint8_t out[16])
{
    if (!ifname || *ifname == '\0' || !out) return -1;

    FILE *f = fopen("/proc/net/ipv6_route", "r");
    if (!f) return -1;

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), f))
    {
        char dest_hex[33], src_hex[33], nh_hex[33], iface[IFNAMSIZ];
        unsigned int dest_plen = 0, src_plen = 0;

        int n = sscanf(
            line,
            "%32s %x %32s %x %32s %*x %*x %*x %*x %15s",
            dest_hex, &dest_plen, src_hex, &src_plen, nh_hex, iface
        );
        if (n < 6) continue;
        if (strcmp(iface, ifname) != 0) continue;
        if (dest_plen != 0 || src_plen != 0) continue;
        if (strcmp(dest_hex, "00000000000000000000000000000000") != 0) continue;

        for (int i = 0; i < 16; ++i)
        {
            if (parse_hex_byte(&nh_hex[i * 2], &out[i]) != 0)
            {
                fclose(f);
                return -1;
            }
        }
        found = 1;
        break;
    }

    fclose(f);
    return found ? 0 : -1;
}

void* interfaces_event_update(void *arg)
{
    INTRF *intrf = (INTRF *) arg;
    (void)intrf;

    while (1)
    {
        /* Get network interfaces list */

        const char *path = "/sys/class/net";
        DIR *dir = opendir(path);
        if (dir == NULL)
        {
            perror("opendir(/sys/class/net)");
            break;
        }

        errno = 0;
        struct dirent *ent = NULL;
        int read_failed = 0;

        size_t ct = 0, cap = 0;
        char **D_NAMES = NULL;
        while ((ent = readdir(dir)) != NULL) 
        {
            if (ent->d_name[0] == '.') continue;

            if (ct == cap)
            {
                size_t new_cap = cap ? cap * 2 : 8;
                char **tmp = realloc(D_NAMES, new_cap * sizeof(*D_NAMES));
                if (!tmp)
                {
                    perror("realloc(D_NAMES)");
                    read_failed = 1;
                    break;
                }
                D_NAMES = tmp;
                cap = new_cap;
            }

            D_NAMES[ct] = strdup(ent->d_name);
            if (!D_NAMES[ct])
            {
                perror("strdup(ent->d_name)");
                read_failed = 1;
                break;
            }
            ++ct;
        }

        if (errno != 0) 
        {
            perror("readdir(/sys/class/net)");
            read_failed = 1;
        }

        if (closedir(dir) != 0) perror("closedir(/sys/class/net)");
        if (ct == 0 || read_failed) fprintf(stderr, "interfaces reading failed\n");

        int count = (int) ct;
        const size_t active_status_slot = 16;
        int active_interf_ct = 0;
        int parse_failed = 0;
        uint8_t interval_sec = 1;

        char **device_name = calloc((size_t) count, sizeof(*device_name));
        char **device_type = calloc((size_t) count, sizeof(*device_type));
        char *active_status = calloc((size_t) count, active_status_slot);
        char **conn_name = calloc((size_t) count, sizeof(*conn_name));
        float *tx_rate_kibs = calloc((size_t) count, sizeof(*tx_rate_kibs));
        float *rx_rate_kibs = calloc((size_t) count, sizeof(*rx_rate_kibs));
        int32_t *mtu_interf = calloc((size_t) count, sizeof(*mtu_interf));
        uint8_t (*mac_address)[6] = calloc((size_t) count, sizeof(*mac_address));
        uint8_t (*ipv4_address)[4] = calloc((size_t) count, sizeof(*ipv4_address));
        uint8_t (*ipv6_address)[16] = calloc((size_t) count, sizeof(*ipv6_address));
        uint8_t (*gw_ipv4_address)[4] = calloc((size_t) count, sizeof(*gw_ipv4_address));
        uint8_t (*gw_ipv6_address)[16] = calloc((size_t) count, sizeof(*gw_ipv6_address));
        int *rx_total_bytes = calloc((size_t) count, sizeof(*rx_total_bytes));
        int *rx_total_packs = calloc((size_t) count, sizeof(*rx_total_packs));
        int *rx_total_drops = calloc((size_t) count, sizeof(*rx_total_drops));
        int *rx_total_errors = calloc((size_t) count, sizeof(*rx_total_errors));
        int *tx_total_bytes = calloc((size_t) count, sizeof(*tx_total_bytes));
        int *tx_total_packs = calloc((size_t) count, sizeof(*tx_total_packs));
        int *tx_total_drops = calloc((size_t) count, sizeof(*tx_total_drops));
        int *tx_total_errors = calloc((size_t) count, sizeof(*tx_total_errors));
        int *device_link = calloc((size_t) count, sizeof(*device_link)); // "{device_link//1000}G"
        char (*duplex_mode)[16] = calloc((size_t) count, sizeof(*duplex_mode)); // half | full | unknown
        char **operstate_mode = calloc((size_t) count, sizeof(*operstate_mode));

        uint64_t *rx_bytes_t1 = calloc((size_t) count, sizeof(*rx_bytes_t1));
        uint64_t *rx_packs_t1 = calloc((size_t) count, sizeof(*rx_packs_t1));
        uint64_t *tx_bytes_t1 = calloc((size_t) count, sizeof(*tx_bytes_t1));
        uint64_t *tx_packs_t1 = calloc((size_t) count, sizeof(*tx_packs_t1));

        if (
            !device_name || !device_type || !active_status || !conn_name ||
            !tx_rate_kibs || !rx_rate_kibs || !mtu_interf || !ipv4_address ||
            !ipv6_address || !gw_ipv4_address || !gw_ipv6_address || !rx_total_bytes ||
            !rx_total_packs || !rx_total_drops || !rx_total_errors ||
            !tx_total_bytes || !tx_total_packs || !tx_total_drops || !tx_total_errors ||
            !device_link || !operstate_mode || !mac_address || !rx_bytes_t1 || !rx_packs_t1 ||
            !tx_bytes_t1 || !tx_packs_t1 || !duplex_mode
        )
        {
            perror("calloc(interfaces_event_update)");
            parse_failed = 1;
        }

        if (parse_failed) perror("calloc(pointer arrays)");

        if (!parse_failed)
        {
            for (size_t i = 0; i < ct; ++i)
            {
                rx_bytes_t1[i] = read_net_stats_by_file(D_NAMES[i], "rx_bytes");
                rx_packs_t1[i] = read_net_stats_by_file(D_NAMES[i], "rx_packets");
                tx_bytes_t1[i] = read_net_stats_by_file(D_NAMES[i], "tx_bytes");
                tx_packs_t1[i] = read_net_stats_by_file(D_NAMES[i], "tx_packets");
                if (
                    rx_bytes_t1[i] == UINT64_MAX || rx_packs_t1[i] == UINT64_MAX ||
                    tx_bytes_t1[i] == UINT64_MAX || tx_packs_t1[i] == UINT64_MAX
                )
                {
                    perror("read_net_stats_by_file(bytes|packets) t1 error");
                    parse_failed = 1;
                    break;
                }
            }
        }

        if (!parse_failed) sleep(interval_sec);

        if (!parse_failed)
        {
            for (size_t i = 0; i < ct; ++i)
            {

                /* tx_rate_kibs | rx_rate_kibs | (rx / tx) bytes / packs / drops / errors */

                uint64_t rx_bytes_t2 = read_net_stats_by_file(D_NAMES[i], "rx_bytes");
                uint64_t rx_packs_t2 = read_net_stats_by_file(D_NAMES[i], "rx_packets");
                uint64_t tx_bytes_t2 = read_net_stats_by_file(D_NAMES[i], "tx_bytes");
                uint64_t tx_packs_t2 = read_net_stats_by_file(D_NAMES[i], "tx_packets");
                if (
                    rx_bytes_t2 == UINT64_MAX || rx_packs_t2 == UINT64_MAX ||
                    tx_bytes_t2 == UINT64_MAX || tx_packs_t2 == UINT64_MAX
                )
                {
                    perror("read_net_stats_by_file(bytes|packets) t2 error");
                    parse_failed = 1;
                    break;
                }

                uint64_t rx_bytes_delta = (rx_bytes_t2 >= rx_bytes_t1[i]) ? (rx_bytes_t2 - rx_bytes_t1[i]) : 0;
                uint64_t tx_bytes_delta = (tx_bytes_t2 >= tx_bytes_t1[i]) ? (tx_bytes_t2 - tx_bytes_t1[i]) : 0;

                rx_rate_kibs[i] = (float) (rx_bytes_delta / interval_sec / 1024.0);
                tx_rate_kibs[i] = (float) (tx_bytes_delta / interval_sec / 1024.0);

                rx_total_bytes[i] = (rx_bytes_t2 > INT_MAX) ? INT_MAX : (int) rx_bytes_t2;
                rx_total_packs[i] = (rx_packs_t2 > INT_MAX) ? INT_MAX : (int) rx_packs_t2;
                tx_total_bytes[i] = (tx_bytes_t2 > INT_MAX) ? INT_MAX : (int) tx_bytes_t2;
                tx_total_packs[i] = (tx_packs_t2 > INT_MAX) ? INT_MAX : (int) tx_packs_t2;

                uint64_t rx_errors = read_net_stats_by_file(D_NAMES[i], "rx_errors");
                uint64_t tx_errors = read_net_stats_by_file(D_NAMES[i], "tx_errors");
                uint64_t rx_dropped = read_net_stats_by_file(D_NAMES[i], "rx_dropped");
                uint64_t tx_dropped = read_net_stats_by_file(D_NAMES[i], "tx_dropped");
                if (
                    rx_errors == UINT64_MAX || tx_errors == UINT64_MAX ||
                    rx_dropped == UINT64_MAX || tx_dropped == UINT64_MAX
                )
                {
                    perror("read_net_stats_by_file(errors|dropped) error");
                    parse_failed = 1;
                    break;
                }

                rx_total_errors[i] = (rx_errors > INT_MAX) ? INT_MAX : (int) rx_errors;
                tx_total_errors[i] = (tx_errors > INT_MAX) ? INT_MAX : (int) tx_errors;
                rx_total_drops[i] = (rx_dropped > INT_MAX) ? INT_MAX : (int) rx_dropped;
                tx_total_drops[i] = (tx_dropped > INT_MAX) ? INT_MAX : (int) tx_dropped;

                /* device_name field */

                device_name[i] = strdup(D_NAMES[i]);
                if (!device_name[i])
                {
                    perror("strdup(device_name)");
                    parse_failed = 1;
                    break;
                }

                /* device_type field */

                uint64_t type_code = read_net_stats_by_file(D_NAMES[i], "type");
                if (type_code == UINT64_MAX) device_type[i] = strdup("UNKNOWN");
                else device_type[i] = strdup(iftype_to_str((unsigned int) type_code));
                if (!device_type[i])
                {
                    perror("strdup(device_type)");
                    parse_failed = 1;
                    break;
                }

                /* active_status / operstate / carrier field */

                char operstate_raw[32];
                char *status = active_status + (i * active_status_slot);

                if (read_ifname_operstate(D_NAMES[i], operstate_raw, sizeof(operstate_raw)) != 0)
                {
                    operstate_raw[0] = '\0';
                }

                operstate_mode[i] = strdup(operstate_raw[0] ? operstate_raw : "UNKNOWN");
                if (!operstate_mode[i])
                {
                    perror("strdup(operstate_mode)");
                    parse_failed = 1;
                    break;
                }

                int carrier = read_ifname_carrier(D_NAMES[i]);
                const char *status_label = "DOWN";
                if (carrier == 1 || (carrier == -1 && strcmp(operstate_raw, "up") == 0))
                {
                    status_label = "UP";
                }

                snprintf(status, active_status_slot, "%s", status_label);
                if (strcmp(status_label, "UP") == 0)
                {
                    ++active_interf_ct;
                }

                /* conn_name field */

                char master[64];
                if (get_master_ifname(D_NAMES[i], master, sizeof(master)) == 0)
                {
                    conn_name[i] = strdup(master);
                }
                else
                {
                    conn_name[i] = strdup("-");
                }
                if (!conn_name[i])
                {
                    perror("strdup(conn_name)");
                    parse_failed = 1;
                    break;
                }

                /* mtu_interf field */

                int32_t mtu = read_ifname_mtu(D_NAMES[i]);
                if (mtu == -1)
                {
                    perror("read_ifname_mtu failed");
                    parse_failed = 1;
                    break;
                }
                else mtu_interf[i] = (uint32_t) mtu;

                /* mac_address field */

                if (read_ifname_mac_address(D_NAMES[i], mac_address[i]) != 0)
                {
                    perror("read_ifname_mac_address failed");
                    parse_failed = 1;
                    break;
                }

                /* ipv4_address field */

                if (read_ifname_ipv4_address(D_NAMES[i], ipv4_address[i]) != 0)
                {
                    memset(ipv4_address[i], 0, sizeof(ipv4_address[i]));
                }

                /* ipv6_address field */

                if (read_ifname_ipv6_address(D_NAMES[i], ipv6_address[i]) != 0)
                {
                    memset(ipv6_address[i], 0, sizeof(ipv6_address[i]));
                }

                /* gw_ipv4_address field */

                if (read_ifname_gw_ipv4_address(D_NAMES[i], gw_ipv4_address[i]) != 0)
                {
                    memset(gw_ipv4_address[i], 0, sizeof(gw_ipv4_address[i]));
                }

                /* gw_ipv6_address field */

                if (read_ifname_gw_ipv6_address(D_NAMES[i], gw_ipv6_address[i]) != 0)
                {
                    memset(gw_ipv6_address[i], 0, sizeof(gw_ipv6_address[i]));
                }

                /* device_link field */

                int32_t link_speed = read_ifname_link_speed(D_NAMES[i]);
                device_link[i] = (link_speed >= 0) ? (int) link_speed : 0;

                /* duplex_mode field */

                if (read_ifname_duplex(D_NAMES[i], duplex_mode[i], sizeof(duplex_mode[i])) != 0)
                {
                    snprintf(duplex_mode[i], sizeof(duplex_mode[i]), "%s", "unknown");
                }

            }
        }

        if (!parse_failed)
        {
            INTRF_update_data(
                intrf,
                count,
                active_interf_ct,
                device_name,
                device_type,
                active_status,
                conn_name,
                tx_rate_kibs,
                rx_rate_kibs,
                mtu_interf,
                mac_address,
                ipv4_address,
                ipv6_address,
                gw_ipv4_address,
                gw_ipv6_address,
                rx_total_bytes,
                rx_total_packs,
                rx_total_drops,
                rx_total_errors,
                tx_total_bytes,
                tx_total_packs,
                tx_total_drops,
                tx_total_errors,
                device_link,
                duplex_mode,
                operstate_mode
            );
        }
        else
        {
            for (int i = 0; i < count; ++i)
            {
                free(device_name ? device_name[i] : NULL);
                free(device_type ? device_type[i] : NULL);
                free(conn_name ? conn_name[i] : NULL);
                free(operstate_mode ? operstate_mode[i] : NULL);
            }
            free(device_name);
            free(device_type);
            free(active_status);
            free(conn_name);
            free(tx_rate_kibs);
            free(rx_rate_kibs);
            free(mtu_interf);
            free(mac_address);
            free(ipv4_address);
            free(ipv6_address);
            free(gw_ipv4_address);
            free(gw_ipv6_address);
            free(rx_total_bytes);
            free(rx_total_packs);
            free(rx_total_drops);
            free(rx_total_errors);
            free(tx_total_bytes);
            free(tx_total_packs);
            free(tx_total_drops);
            free(tx_total_errors);
            free(device_link);
            free(duplex_mode);
            free(operstate_mode);
        }

        free(rx_bytes_t1);
        free(rx_packs_t1);
        free(tx_bytes_t1);
        free(tx_packs_t1);

        for (size_t i = 0; i < ct; ++i) free(D_NAMES[i]);
        free(D_NAMES);
    }

    return NULL;
}
