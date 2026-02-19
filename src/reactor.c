#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>

#include "reactor.h"

#include "data/overall_data.h"
#include "data/interfaces_data.h"
#include "data/addr_dns_data.h"
#include "data/arp_route_data.h"
#include "data/connections_sockets_data.h"
#include "data/protocol_stats_data.h"
#include "data/wifi_data.h"
#include "data/network_profiles_data.h"

typedef struct tcp_connection_list
{
    unsigned sl;
    char local_addr_hex[16];
    uint16_t local_port;
    char rem_addr_hex[16];
    uint16_t rem_port;
    const char *state;
    uint32_t tx_queue;
    uint32_t rx_queue;
    uint8_t tr;
    uint32_t tm_when;
    uint32_t retrnsmt;
    unsigned uid;
    unsigned timeout;
    unsigned long long inode;
} TcpConnLst;

static void hex_ipv4_to_str(uint32_t hex, char out[16]) 
{
    snprintf(
        out, 16, "%u.%u.%u.%u",
        (hex) & 0xFF,
        (hex >> 8) & 0xFF,
        (hex >> 16) & 0xFF,
        (hex >> 24) & 0xFF
    );
}

static const char *tcp_state_name(uint8_t st)
{
    switch (st)
    {
        case 0x01: return "ESTABLISHED";
        case 0x02: return "SYN_SENT";
        case 0x03: return "SYN_RECV";
        case 0x04: return "FIN_WAIT1";
        case 0x05: return "FIN_WAIT2";
        case 0x06: return "TIME_WAIT";
        case 0x07: return "CLOSE";
        case 0x08: return "CLOSE_WAIT";
        case 0x09: return "LAST_ACK";
        case 0x0A: return "LISTEN";
        case 0x0B: return "CLOSING";
        default:   return "UNKNOWN";
    }
}

static int8_t scan_tcp_conn_list(TcpConnLst **tcl, size_t *sz)
{
    if (tcl == NULL || sz == NULL) return 1;

    FILE *f = fopen("/proc/net/tcp", "r");
    if (!f)
    {
        perror("fopen(/proc/net/tcp)");
        return 1;
    }

    char line[1024];

    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return 1;
    }

    size_t ct = 0;
    size_t cap = 0;
    int failed = 0;
    TcpConnLst *list = NULL;

    while (fgets(line, sizeof(line), f))
    {
        TcpConnLst e;

        if (ct == cap)
        {
            size_t new_cap = cap ? cap * 2 : 8;
            TcpConnLst *tmp = realloc(list, new_cap * sizeof(*list));
            if (!tmp)
            {
                perror("realloc(TcpConnLst)");
                failed = 1;
                break;
            }
            cap = new_cap;
            list = tmp;
        }

        unsigned sl;
        uint32_t local_addr_hex;
        uint16_t local_port;
        uint32_t rem_addr_hex;
        uint16_t rem_port;
        uint8_t state;
        uint32_t tx_queue;
        uint32_t rx_queue;
        uint8_t tr;
        uint32_t tm_when;
        uint32_t retrnsmt;
        unsigned uid;
        unsigned timeout;
        unsigned long long inode;

        int n = sscanf(
            line, 
            " %u: %8X:%4hX %8X:%4hX %2hhX %8X:%8X %2hhX:%8X %8X %u %u %llu",
            &sl, &local_addr_hex,
            &local_port, &rem_addr_hex,
            &rem_port, &state,
            &tx_queue, &rx_queue,
            &tr, &tm_when,
            &retrnsmt, &uid,
            &timeout, &inode
        );

        if (n < 14) continue;

        hex_ipv4_to_str(local_addr_hex, e.local_addr_hex);
        hex_ipv4_to_str(rem_addr_hex, e.rem_addr_hex);

        e.sl = sl;
        e.local_port = local_port;
        e.rem_port = rem_port;
        e.state = tcp_state_name(state);
        e.tx_queue = tx_queue;
        e.rx_queue = rx_queue;
        e.tr = tr;
        e.tm_when = tm_when;
        e.retrnsmt = retrnsmt;
        e.uid = uid;
        e.timeout = timeout;
        e.inode = inode;

        list[ct++] = e;
    }

    fclose(f);

    if (failed)
    {
        free(list);
        *tcl = NULL;
        *sz = 0;
        return 1;
    }

    *tcl = list;
    *sz = ct;
    return 0;
}

static int8_t scan_tcp6_conn_list(TcpConnLst **tcl, size_t *sz)
{
    if (tcl == NULL || sz == NULL) return 1;

    FILE *f = fopen("/proc/net/tcp6", "r");
    if (!f)
    {
        perror("fopen(/proc/net/tcp6)");
        return 1;
    }

    char line[1024];

    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return 1;
    }

    size_t ct = 0;
    size_t cap = 0;
    int failed = 0;
    TcpConnLst *list = NULL;

    while (fgets(line, sizeof(line), f))
    {
        TcpConnLst e;
        memset(&e, 0, sizeof(e));

        if (ct == cap)
        {
            size_t new_cap = cap ? cap * 2 : 8;
            TcpConnLst *tmp = realloc(list, new_cap * sizeof(*list));
            if (!tmp)
            {
                perror("realloc(TcpConnLst tcp6)");
                failed = 1;
                break;
            }
            cap = new_cap;
            list = tmp;
        }

        unsigned sl;
        uint8_t state;
        uint32_t retrnsmt;

        int n = sscanf(
            line,
            " %u: %*32[0-9A-Fa-f]:%*4[0-9A-Fa-f] %*32[0-9A-Fa-f]:%*4[0-9A-Fa-f] %2hhX %*8[0-9A-Fa-f]:%*8[0-9A-Fa-f] %*2[0-9A-Fa-f]:%*8[0-9A-Fa-f] %8X",
            &sl,
            &state,
            &retrnsmt
        );
        if (n < 3) continue;

        e.sl = sl;
        e.state = tcp_state_name(state);
        e.retrnsmt = retrnsmt;

        list[ct++] = e;
    }

    fclose(f);

    if (failed)
    {
        free(list);
        *tcl = NULL;
        *sz = 0;
        return 1;
    }

    *tcl = list;
    *sz = ct;
    return 0;
}

uint64_t read_net_stats(const char *interf_name, const char *direction, const char *metric)
{
    char path[PATH_MAX];

    if (interf_name == NULL || *interf_name == '\0') return UINT64_MAX;
    if (direction == NULL || metric == NULL) return UINT64_MAX;
    if (strchr(interf_name, '/')) return UINT64_MAX;
    if (strcmp(direction, "rx") != 0 && strcmp(direction, "tx") != 0) return UINT64_MAX;
    if (*metric == '\0' || strchr(metric, '/')) return UINT64_MAX;

    for (const unsigned char *p = (const unsigned char *)metric; *p != '\0'; ++p)
    {
        if (!(isalnum(*p) || *p == '_')) return UINT64_MAX;
    }

    int path_len = snprintf(
        path,
        sizeof(path),
        "/sys/class/net/%s/statistics/%s_%s",
        interf_name,
        direction,
        metric
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

static int8_t is_virtual_interface(const char *name)
{
    const char *virt_prefix = "/sys/devices/virtual/net/";
    char path[PATH_MAX];
    char resolver[PATH_MAX];

    if (name == NULL || *name == '\0') return -1;
    if (strchr(name, '/')) return -1;
    if (snprintf(path, sizeof(path), "/sys/class/net/%s", name) >= (int) sizeof(path)) return -1;
    if (realpath(path, resolver) == NULL) return -1;

    return (strncmp(resolver, virt_prefix, strlen(virt_prefix)) == 0) ? 1 : 0;
}

static void log_thread_error(const char *section, int rc)
{
    fprintf(stderr, "%s section thread error: %s\n", section, strerror(rc));
}

static void* overall_event_update(void *arg)
{
    OVRLL *ovrll = (OVRLL*) arg;

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

        /* Varify network interfaces for virtuality of origin  */

        size_t fct = 0;
        for (size_t i = 0; i < ct; ++i)
        {
            int8_t chres = is_virtual_interface(D_NAMES[i]);
            if (chres == -1)
            {
                fprintf(stderr, "virtual origin check fail: %s\n", D_NAMES[i]);
                free(D_NAMES[i]);
                continue;
            }
            else if (chres == 0)
            {
                D_NAMES[fct++] = D_NAMES[i];
            }
            else
            {
                free(D_NAMES[i]);
            }
        }

        /* Parsing interfaces network data  */

        for (size_t i = 0; i < fct; ++i)
        {
            float rx_rate_kibs;
            float rx_rate_kpps;
            float tx_rate_kibs;
            float tx_rate_kpps;
            float rx_total_kibs;
            float tx_total_kibs;
            int errors_rx;
            int errors_tx;
            int drops_rx;
            int drops_tx;
            int conn_estab;
            int conn_lst;
            int conn_tmw;
            int conn_systn;
            int conn_clsw;
            int retr_pkg;

            uint8_t interval_sec = 1;

            uint64_t rx_bytes_t1 = read_net_stats(D_NAMES[i], "rx", "bytes");
            uint64_t rx_packs_t1 = read_net_stats(D_NAMES[i], "rx", "packets");
            if (rx_bytes_t1 == UINT64_MAX || rx_packs_t1 == UINT64_MAX)
            {
                perror("read_net_stats(rx bytes|packets) t1 error");
                break;
            }
            uint64_t tx_bytes_t1 = read_net_stats(D_NAMES[i], "tx", "bytes");
            uint64_t tx_packs_t1 = read_net_stats(D_NAMES[i], "tx", "packets");
            if (tx_bytes_t1 == UINT64_MAX || tx_packs_t1 == UINT64_MAX)
            {
                perror("read_net_stats(tx bytes|packets) t1 error");
                break;
            }

            sleep(interval_sec);

            uint64_t rx_bytes_t2 = read_net_stats(D_NAMES[i], "rx", "bytes");
            uint64_t rx_packs_t2 = read_net_stats(D_NAMES[i], "rx", "packets");
            if (rx_bytes_t2 == UINT64_MAX || rx_packs_t2 == UINT64_MAX)
            {
                perror("read_net_stats(rx bytes|packets) t2 error");
                break;
            }
            uint64_t tx_bytes_t2 = read_net_stats(D_NAMES[i], "tx", "bytes");
            uint64_t tx_packs_t2 = read_net_stats(D_NAMES[i], "tx", "packets");
            if (tx_bytes_t2 == UINT64_MAX || tx_packs_t2 == UINT64_MAX)
            {
                perror("read_net_stats(tx bytes|packets) t2 error");
                break;
            }

            rx_rate_kibs = (float) ((rx_bytes_t2 - rx_bytes_t1) / interval_sec / 1024.0);
            rx_rate_kpps = (float) ((rx_packs_t2 - rx_packs_t1) / interval_sec / 1000.0);

            tx_rate_kibs = (float) ((tx_bytes_t2 - tx_bytes_t1) / interval_sec / 1024.0);
            tx_rate_kpps = (float) ((tx_packs_t2 - tx_packs_t1) / interval_sec / 1000.0);
            
            rx_total_kibs = (float) (rx_bytes_t2 / 1024.0);
            tx_total_kibs = (float) (tx_bytes_t2 / 1024.0);

            uint64_t rx_errors = read_net_stats(D_NAMES[i], "rx", "errors");
            uint64_t tx_errors = read_net_stats(D_NAMES[i], "tx", "errors");

            errors_rx = (int) rx_errors;
            errors_tx = (int) tx_errors;

            uint64_t rx_dropped = read_net_stats(D_NAMES[i], "rx", "dropped");
            uint64_t tx_dropped = read_net_stats(D_NAMES[i], "tx", "dropped");

            drops_rx = (int) rx_dropped;
            drops_tx = (int) tx_dropped;

            TcpConnLst *tcl = NULL;
            size_t sz = 0;
            if (scan_tcp_conn_list(&tcl, &sz) != 0)
            {
                perror("scan_tcp_conn_list error");
                break;
            }

            conn_estab = 0;
            conn_lst = 0;
            conn_tmw = 0;
            conn_systn = 0;
            conn_clsw = 0;
            retr_pkg = 0;
            for (size_t j = 0; j < sz; ++j)
            {
                if (strcmp(tcl[j].state, "ESTABLISHED") == 0) conn_estab++;
                else if (strcmp(tcl[j].state, "LISTEN") == 0) conn_lst++;
                else if (strcmp(tcl[j].state, "TIME_WAIT") == 0) conn_tmw++;
                else if (strcmp(tcl[j].state, "SYN_SENT") == 0) conn_systn++;
                else if (strcmp(tcl[j].state, "CLOSE_WAIT") == 0) conn_clsw++;
                retr_pkg += (int) tcl[j].retrnsmt;
            }

            free(tcl);

            TcpConnLst *tcl6 = NULL;
            size_t sz6 = 0;
            if (scan_tcp6_conn_list(&tcl6, &sz6) != 0)
            {
                perror("scan_tcp6_conn_list error");
                break;
            }

            for (size_t j = 0; j < sz6; ++j)
            {
                if (strcmp(tcl6[j].state, "ESTABLISHED") == 0) conn_estab++;
                else if (strcmp(tcl6[j].state, "LISTEN") == 0) conn_lst++;
                else if (strcmp(tcl6[j].state, "TIME_WAIT") == 0) conn_tmw++;
                else if (strcmp(tcl6[j].state, "SYN_SENT") == 0) conn_systn++;
                else if (strcmp(tcl6[j].state, "CLOSE_WAIT") == 0) conn_clsw++;
                retr_pkg += (int) tcl6[j].retrnsmt;
            }

            free(tcl6);

            OVRLL_update_data(
                ovrll, 
                rx_rate_kibs, 
                rx_rate_kpps,
                tx_rate_kibs,
                tx_rate_kpps,
                rx_total_kibs,
                tx_total_kibs,
                errors_rx,
                errors_tx,
                drops_rx,
                drops_tx,
                conn_estab,
                conn_lst,
                conn_tmw,
                conn_systn,
                conn_clsw,
                (float)retr_pkg
            );
        }

        for (size_t i = 0; i < fct; ++i) free(D_NAMES[i]);
        free(D_NAMES);
    }

    return NULL;
}

static void* interfaces_event_update(void *arg)
{
    INTRF *intrf = (INTRF *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* addr_dns_event_update(void *arg)
{
    ADDRDNS *addrdns = (ADDRDNS *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* arp_route_event_update(void *arg)
{
    ARPRT *arprt = (ARPRT *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* connections_sockets_event_update(void *arg)
{
    CONSOCK *consock = (CONSOCK *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* protocol_stats_event_update(void *arg)
{
    PROTST *protst = (PROTST *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* wifi_event_update(void *arg)
{
    WIFI *wifi = (WIFI *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* network_profiles_event_update(void *arg)
{
    NETPROF *netprof = (NETPROF *) arg;

    while (1)
    {
        
    }

    return NULL;
}

void event_loop(
    OVRLL *ovrll,
    INTRF *intrf,
    ADDRDNS *addrdns,
    ARPRT *arprt,
    CONSOCK *consock,
    PROTST *protst,
    WIFI *wifi,
    NETPROF *netprof
)
{
    pthread_t ovrll_t;
    pthread_t intrf_t;
    pthread_t addrdns_t;
    pthread_t arprt_t;
    pthread_t consock_t;
    pthread_t protst_t;
    pthread_t wifi_t;
    pthread_t netprof_t;

    int ovrll_rc = pthread_create(&ovrll_t, NULL, overall_event_update, ovrll);
    if (ovrll_rc != 0) log_thread_error("overall", ovrll_rc);
    else pthread_detach(ovrll_t);

    int intrf_rc = pthread_create(&intrf_t, NULL, interfaces_event_update, intrf);
    if (intrf_rc != 0) log_thread_error("interfaces", intrf_rc);
    else pthread_detach(intrf_t);

    int addrdns_rc = pthread_create(&addrdns_t, NULL, addr_dns_event_update, addrdns);
    if (addrdns_rc != 0) log_thread_error("addr_dns", addrdns_rc);
    else pthread_detach(addrdns_t);

    int arprt_rc = pthread_create(&arprt_t, NULL, arp_route_event_update, arprt);
    if (arprt_rc != 0) log_thread_error("arp_route", arprt_rc);
    else pthread_detach(arprt_t);

    int consock_rc = pthread_create(&consock_t, NULL, connections_sockets_event_update, consock);
    if (consock_rc != 0) log_thread_error("connections_sockets", consock_rc);
    else pthread_detach(consock_t);

    int protst_rc = pthread_create(&protst_t, NULL, protocol_stats_event_update, protst);
    if (protst_rc != 0) log_thread_error("protocol_stats", protst_rc);
    else pthread_detach(protst_t);

    int wifi_rc = pthread_create(&wifi_t, NULL, wifi_event_update, wifi);
    if (wifi_rc != 0) log_thread_error("wifi", wifi_rc);
    else pthread_detach(wifi_t);

    int netprof_rc = pthread_create(&netprof_t, NULL, network_profiles_event_update, netprof);
    if (netprof_rc != 0) log_thread_error("network_profiles", netprof_rc);
    else pthread_detach(netprof_t);

    return;
}
