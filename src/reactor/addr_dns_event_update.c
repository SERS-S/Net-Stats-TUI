#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/stat.h>
#include <unistd.h>

#include "reactor/addr_dns_event_update.h"

#include "data/addr_dns_data.h"

typedef struct ProcNetRouteStats
{
    char *iface;
    char *destination;
    char *gateway;
    char *flags;
    int16_t refctn;
    int16_t use;
    int16_t metric;
    int8_t mask;
    int32_t mtu;
    int16_t window;
    int16_t irtt;
} PNRS;

static int8_t read_proc_net_route_stats(PNRS **pnrs, size_t *sz)
{
    if (pnrs == NULL || sz == NULL) return -1;

    FILE *f = fopen("/proc/net/route", "r");
    if (!f)
    {
        perror("fopen(/proc/net/route)");
        return -1;
    }

    char line[1024];

    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    size_t ct = 0;
    size_t cap = 0;
    int failed = 0;
    PNRS *list = NULL;

    while (fgets(line, sizeof(line), f))
    {
        PNRS e = {0};

        if (ct == cap)
        {
            size_t new_cap = cap ? cap * 2 : 8;
            PNRS *tmp = realloc(list, new_cap * sizeof(*list));
            if (!tmp)
            {
                perror("realloc(PRNS)");
                failed = 1;
                break;
            }
            cap = new_cap;
            list = tmp;
        }

        char iface_buf[16];
        char destination_buf[9];
        char gateway_buf[9];
        char flags_buf[5];
        char mask_buf[9];
        int refcnt = 0;
        int use = 0;
        int metric = 0;
        int mtu = 0;
        int window = 0;
        int irtt = 0;

        int n = sscanf(
            line,
            "%15s %8s %8s %4s %d %d %d %8s %d %d %d",
            iface_buf, destination_buf, gateway_buf,
            flags_buf, &refcnt, &use,
            &metric, mask_buf, &mtu,
            &window, &irtt
        );

        if (n != 11) continue;

        e.iface = strdup(iface_buf);
        e.destination = strdup(destination_buf);
        e.gateway = strdup(gateway_buf);
        e.flags = strdup(flags_buf);
        if (!e.iface || !e.destination || !e.gateway || !e.flags)
        {
            perror("strdup(read_proc_net_route_stats)");
            free(e.iface);
            free(e.destination);
            free(e.gateway);
            free(e.flags);
            failed = 1;
            break;
        }

        unsigned long mask_hex = strtoul(mask_buf, NULL, 16);
        int8_t mask_bits = 0;
        while (mask_hex != 0)
        {
            mask_bits += (int8_t) (mask_hex & 1ul);
            mask_hex >>= 1;
        }

        e.refctn = (int16_t) refcnt;
        e.use = (int16_t) use;
        e.metric = (int16_t) metric;
        e.mask = mask_bits;
        e.mtu = (int32_t) mtu;
        e.window = (int16_t) window;
        e.irtt = (int16_t) irtt;
        list[ct++] = e;
    }

    fclose(f);

    if (failed)
    {
        for (size_t i = 0; i < ct; ++i)
        {
            free(list[i].iface);
            free(list[i].destination);
            free(list[i].gateway);
            free(list[i].flags);
        }
        free(list);
        *pnrs = NULL;
        *sz = 0;
        return -1;
    }

    *pnrs = list;
    *sz = ct;
    return 0;
}

static int read_ifname_ipv4_address(const char *ifname, uint8_t out[4])
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

static int read_ifname_ipv6_address_mask(const char *ifname, uint8_t ipv6_address[16], uint8_t *ipv6_mask)
{
    if (ifname == NULL || *ifname == '\0' || !ipv6_address || !ipv6_mask) return -1;

    FILE *f = fopen("/proc/net/if_inet6", "r");
    if (!f) return -1;

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        char addr_hex[33];
        char interf_name[IFNAMSIZ];
        unsigned int ifindex_hex, prefix_len_hex, scope_hex, flags_hex;

        int n = sscanf(
            line,
            "%32s %x %x %x %x %15s",
            addr_hex, &ifindex_hex, &prefix_len_hex, &scope_hex, &flags_hex, interf_name
        );
        if ((n != 6) || (strcmp(ifname, interf_name) != 0))
        {
            continue;
        }
        if (prefix_len_hex > 128)
        {
            fclose(f);
            return -1;
        }

        *ipv6_mask = (uint8_t) prefix_len_hex;

        for (int i = 0; i < 16; ++i)
        {
            unsigned int v;
            if (sscanf(&addr_hex[i * 2], "%2x", &v) != 1)
            {
                fclose(f);
                return -1;
            }
            ipv6_address[i] = (uint8_t) v;
        }
        fclose(f);
        return 0;
    }

    fclose(f);
    return -1;
}

static int read_dns_manager(char *manager, size_t manager_sz)
{
    const char *p = "/etc/resolv.conf";
    struct stat st;

    if (manager == NULL || manager_sz == 0) return -1;

    if (lstat(p, &st) != 0) return -1;

    if (!S_ISLNK(st.st_mode))
    {
        int n = snprintf(manager, manager_sz, "%s", "static / manual");
        if (n < 0 || (size_t) n >= manager_sz) return -1;
        return 0;
    }

    char resolved[PATH_MAX];
    if (!realpath(p, resolved)) return -1;

    if (strcmp(resolved, "/run/systemd/resolve/stub-resolv.conf") == 0)
    {
        int n = snprintf(manager, manager_sz, "%s", "systemd-resolved (stub)");
        if (n < 0 || (size_t) n >= manager_sz) return -1;
        return 0;
    }
    else if (strcmp(resolved, "/run/systemd/resolve/resolv.conf") == 0)
    {
        int n = snprintf(manager, manager_sz, "%s", "systemd-resolved");
        if (n < 0 || (size_t) n >= manager_sz) return -1;
        return 0;
    }
    else if (strcmp(resolved, "/run/resolvconf/resolv.conf") == 0)
    {
        int n = snprintf(manager, manager_sz, "%s", "resolvconf");
        if (n < 0 || (size_t) n >= manager_sz) return -1;
        return 0;
    }
    else
    {
        int n = snprintf(manager, manager_sz, "%s", "undefined dns mngr");
        if (n < 0 || (size_t) n >= manager_sz) return -1;
        return 0;
    }
}

static int read_dns_servers_list(const char *manager, char **servers_out)
{
    if (manager == NULL || servers_out == NULL) return -1;

    const char *path = "/etc/resolv.conf";
    if (
        strcmp(manager, "systemd-resolved (stub)") == 0 ||
        strcmp(manager, "systemd-resolved") == 0
    )
    {
        path = "/run/systemd/resolve/resolv.conf";
    }
    else if (strcmp(manager, "resolvconf") == 0)
    {
        path = "/run/resolvconf/resolv.conf";
    }

    FILE *f = fopen(path, "r");
    if (!f)
    {
        if (strcmp(path, "/etc/resolv.conf") != 0)
        {
            f = fopen("/etc/resolv.conf", "r");
        }
        if (!f) return -1;
    }

    char line[512];
    char list_buf[1024];
    size_t used = 0;
    list_buf[0] = '\0';

    while (fgets(line, sizeof(line), f))
    {
        char key[32];
        char value[256];
        if (sscanf(line, " %31s %255s", key, value) != 2) continue;
        if (strcmp(key, "nameserver") != 0) continue;

        int n = snprintf(
            list_buf + used,
            sizeof(list_buf) - used,
            "%s%s",
            (used > 0) ? ", " : "",
            value
        );
        if (n < 0 || (size_t) n >= sizeof(list_buf) - used)
        {
            fclose(f);
            return -1;
        }
        used += (size_t) n;
    }
    fclose(f);

    if (used == 0) return -1;

    *servers_out = strdup(list_buf);
    if (*servers_out == NULL) return -1;

    return 0;
}

static int read_dns_search_list(const char *manager, char **search_out)
{
    if (manager == NULL || search_out == NULL) return -1;

    const char *path = "/etc/resolv.conf";
    if (strcmp(manager, "systemd-resolved") == 0)
    {
        path = "/run/systemd/resolve/resolv.conf";
    }
    else if (strcmp(manager, "resolvconf") == 0)
    {
        path = "/run/resolvconf/resolv.conf";
    }

    FILE *f = fopen(path, "r");
    if (!f)
    {
        if (strcmp(path, "/etc/resolv.conf") != 0)
        {
            f = fopen("/etc/resolv.conf", "r");
        }
        if (!f) return -1;
    }

    char line[512];
    char list_buf[1024];
    size_t used = 0;
    list_buf[0] = '\0';

    while (fgets(line, sizeof(line), f))
    {
        char fields[512];
        int has_values = 0;

        if (sscanf(line, " search %511[^\n]", fields) == 1) has_values = 1;
        else if (sscanf(line, " domain %511[^\n]", fields) == 1) has_values = 1;
        if (!has_values) continue;

        char *comment = strpbrk(fields, "#;");
        if (comment) *comment = '\0';

        char *tok = strtok(fields, " \t\r\n");
        while (tok != NULL)
        {
            int n = snprintf(
                list_buf + used,
                sizeof(list_buf) - used,
                "%s%s",
                (used > 0) ? ", " : "",
                tok
            );
            if (n < 0 || (size_t) n >= sizeof(list_buf) - used)
            {
                fclose(f);
                return -1;
            }
            used += (size_t) n;
            tok = strtok(NULL, " \t\r\n");
        }
    }
    fclose(f);

    if (used == 0) return -1;

    *search_out = strdup(list_buf);
    if (*search_out == NULL) return -1;

    return 0;
}

static int read_dns_resolv_path(char *out, size_t out_sz)
{
    if (out == NULL || out_sz == 0) return -1;

    const char *p = "/etc/resolv.conf";
    struct stat st;
    if (lstat(p, &st) != 0) return -1;

    if (!S_ISLNK(st.st_mode))
    {
        int n = snprintf(out, out_sz, "%s -> (static / manual)", p);
        if (n < 0 || (size_t) n >= out_sz) return -1;
        return 0;
    }

    char link_target[PATH_MAX];
    ssize_t rn = readlink(p, link_target, sizeof(link_target) - 1);
    if (rn < 0) return -1;
    link_target[rn] = '\0';

    char resolved[PATH_MAX];
    const char *dst = link_target;
    if (realpath(p, resolved) != NULL)
    {
        dst = resolved;
    }

    const char *kind = "(other)";
    if (strcmp(dst, "/run/systemd/resolve/stub-resolv.conf") == 0)
    {
        kind = "(stub)";
    }
    else if (strcmp(dst, "/run/systemd/resolve/resolv.conf") == 0)
    {
        kind = "(systemd-resolved)";
    }
    else if (strcmp(dst, "/run/resolvconf/resolv.conf") == 0)
    {
        kind = "(resolvconf)";
    }

    int n = snprintf(out, out_sz, "%s -> %s %s", p, dst, kind);
    if (n < 0 || (size_t) n >= out_sz) return -1;

    return 0;
}

void* addr_dns_event_update(void *arg)
{
    ADDRDNS *addrdns = (ADDRDNS *) arg;
    (void)addrdns;

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
        int parse_failed = 0;

        char **interf_name = calloc((size_t) count, sizeof(*interf_name));
        uint8_t (*ipv4_address)[4] = calloc((size_t) count, sizeof(*ipv4_address));
        uint8_t *ipv4_mask = calloc((size_t) count, sizeof(uint8_t));
        uint8_t (*ipv6_address)[16] = calloc((size_t) count, sizeof(*ipv6_address));
        uint8_t *ipv6_mask = calloc((size_t) count, sizeof(uint8_t));
        char *manager = calloc(24, sizeof(char));
        char *servers_list = NULL;
        char *search_list = NULL;
        char *resolv_path = calloc(PATH_MAX, sizeof(char));

        if (
            !interf_name || !ipv4_address || !ipv4_mask || 
            !ipv6_address || !ipv6_mask || !manager ||
            !resolv_path
        )
        {
            perror("calloc(addr_dns_event_update)");
            parse_failed = 1;
        }

        if (parse_failed) perror("calloc(pointer arrays)");

        /* STRUCT FIELDS */

        if (!parse_failed)
        {
            /* manager field */

            if (read_dns_manager(manager, 24) != 0)
            {
                int n = snprintf(manager, 24, "%s", "-");
                if (n < 0 || n >= 24)
                {
                    manager[0] = '\0';
                }
            }

            /* servers_list field */

            if (read_dns_servers_list(manager, &servers_list) != 0)
            {
                servers_list = strdup("-");
                if (!servers_list)
                {
                    perror("strdup(servers_list)");
                    parse_failed = 1;
                }
            }

            /* search_list field */

            if (!parse_failed && read_dns_search_list(manager, &search_list) != 0)
            {
                search_list = strdup("-");
                if (!search_list)
                {
                    perror("strdup(search_list)");
                    parse_failed = 1;
                }
            }

            /* resolv_path field */

            if (!parse_failed && read_dns_resolv_path(resolv_path, PATH_MAX) != 0)
            {
                int n = snprintf(resolv_path, PATH_MAX, "%s", "-");
                if (n < 0 || n >= PATH_MAX)
                {
                    parse_failed = 1;
                }
            }
        }

        if (!parse_failed)
        {
            /* INTERFACES DATA CYCLE */

            PNRS *prns = NULL;
            size_t route_sz = 0;
            if (read_proc_net_route_stats(&prns, &route_sz) != 0)
            {
                prns = NULL;
                route_sz = 0;
            }

            for (size_t i = 0; i < ct && !parse_failed; ++i)
            {
                /* interf_name field */

                interf_name[i] = strdup(D_NAMES[i]);
                if (!interf_name[i])
                {
                    perror("strdup(interf_name)");
                    parse_failed = 1;
                    break;
                }

                /* ipv4_address field */

                if (read_ifname_ipv4_address(D_NAMES[i], ipv4_address[i]) != 0)
                {
                    memset(ipv4_address[i], 0, sizeof(ipv4_address[i]));
                }

                /* ipv4_mask field */

                ipv4_mask[i] = 0;
                for (size_t j = 0; j < route_sz; ++j)
                {
                    if (strcmp(prns[j].iface, D_NAMES[i]) == 0)
                    {
                        ipv4_mask[i] = (uint8_t) prns[j].mask;
                        break;
                    }
                }

                /* ipv6_address | ipv6_mask fields */

                if (read_ifname_ipv6_address_mask(D_NAMES[i], ipv6_address[i], &ipv6_mask[i]) != 0)
                {
                    memset(ipv6_address[i], 0, sizeof(ipv6_address[i]));
                    ipv6_mask[i] = 0;
                }
            }

            for (size_t i = 0; i < route_sz; ++i)
            {
                free(prns[i].iface);
                free(prns[i].destination);
                free(prns[i].gateway);
                free(prns[i].flags);
            }
            free(prns);
        }

        if (!parse_failed)
        {
            ADDRDNS_update_data(
                addrdns,
                count,
                interf_name,
                ipv4_address,
                ipv4_mask,
                ipv6_address,
                ipv6_mask,
                manager,
                servers_list,
                search_list,
                resolv_path
            );
        }
        else
        {
            for (size_t i = 0; i < ct; ++i)
            {
                free(interf_name ? interf_name[i] : NULL);
            }
            free(interf_name);
            free(ipv4_address);
            free(ipv4_mask);
            free(ipv6_address);
            free(ipv6_mask);
            free(manager);
            free(servers_list);
            free(search_list);
            free(resolv_path);
        }

        for (size_t i = 0; i < ct; ++i)
        {
            free(D_NAMES[i]);
        }
        free(D_NAMES);
    }

    return NULL;
}
