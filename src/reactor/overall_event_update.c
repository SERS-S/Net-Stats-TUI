#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <dirent.h>

#include "reactor/overall_event_update.h"

#include "data/overall_data.h"

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

static void print_float_array(const char *name, const float *arr, size_t n)
{
    printf("%s=[", name);
    for (size_t i = 0; i < n; ++i)
    {
        printf("%.3f", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
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

    for (const unsigned char *p = (const unsigned char *) metric; *p != '\0'; ++p)
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
    if (path_len < 0 || path_len >= (int) sizeof(path))
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

void* overall_event_update(void *arg)
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

        float rx_rate_kibs = 0.0f;
        float rx_rate_kpps = 0.0f;
        float tx_rate_kibs = 0.0f;
        float tx_rate_kpps = 0.0f;
        float rx_total_kibs = 0.0f;
        float tx_total_kibs = 0.0f;
        int errors_rx = 0;
        int errors_tx = 0;
        int drops_rx = 0;
        int drops_tx = 0;
        int conn_estab = 0;
        int conn_lst = 0;
        int conn_tmw = 0;
        int conn_systn = 0;
        int conn_clsw = 0;
        int retr_pkg = 0;
        int parse_failed = 0;

        uint8_t interval_sec = 1;

        uint64_t *rx_bytes_t1 = NULL;
        uint64_t *rx_packs_t1 = NULL;
        uint64_t *tx_bytes_t1 = NULL;
        uint64_t *tx_packs_t1 = NULL;
        if (fct > 0)
        {
            rx_bytes_t1 = calloc(fct, sizeof(*rx_bytes_t1));
            rx_packs_t1 = calloc(fct, sizeof(*rx_packs_t1));
            tx_bytes_t1 = calloc(fct, sizeof(*tx_bytes_t1));
            tx_packs_t1 = calloc(fct, sizeof(*tx_packs_t1));
            if (!rx_bytes_t1 || !rx_packs_t1 || !tx_bytes_t1 || !tx_packs_t1)
            {
                perror("calloc(overall_event_update t1)");
                parse_failed = 1;
            }
        }

        if (!parse_failed && fct > 0)
        {
            for (size_t i = 0; i < fct; ++i)
            {
                rx_bytes_t1[i] = read_net_stats(D_NAMES[i], "rx", "bytes");
                rx_packs_t1[i] = read_net_stats(D_NAMES[i], "rx", "packets");
                tx_bytes_t1[i] = read_net_stats(D_NAMES[i], "tx", "bytes");
                tx_packs_t1[i] = read_net_stats(D_NAMES[i], "tx", "packets");
                if (
                    rx_bytes_t1[i] == UINT64_MAX || rx_packs_t1[i] == UINT64_MAX ||
                    tx_bytes_t1[i] == UINT64_MAX || tx_packs_t1[i] == UINT64_MAX
                )
                {
                    perror("read_net_stats(bytes|packets) t1 error");
                    parse_failed = 1;
                    break;
                }
            }
        }

        if (!parse_failed) sleep(interval_sec);

        if (!parse_failed && fct > 0)
        {
            for (size_t i = 0; i < fct; ++i)
            {
                uint64_t rx_bytes_t2 = read_net_stats(D_NAMES[i], "rx", "bytes");
                uint64_t rx_packs_t2 = read_net_stats(D_NAMES[i], "rx", "packets");
                uint64_t tx_bytes_t2 = read_net_stats(D_NAMES[i], "tx", "bytes");
                uint64_t tx_packs_t2 = read_net_stats(D_NAMES[i], "tx", "packets");
                if (
                    rx_bytes_t2 == UINT64_MAX || rx_packs_t2 == UINT64_MAX ||
                    tx_bytes_t2 == UINT64_MAX || tx_packs_t2 == UINT64_MAX
                )
                {
                    perror("read_net_stats(bytes|packets) t2 error");
                    parse_failed = 1;
                    break;
                }

                uint64_t rx_errors = read_net_stats(D_NAMES[i], "rx", "errors");
                uint64_t tx_errors = read_net_stats(D_NAMES[i], "tx", "errors");
                uint64_t rx_dropped = read_net_stats(D_NAMES[i], "rx", "dropped");
                uint64_t tx_dropped = read_net_stats(D_NAMES[i], "tx", "dropped");
                if (
                    rx_errors == UINT64_MAX || tx_errors == UINT64_MAX ||
                    rx_dropped == UINT64_MAX || tx_dropped == UINT64_MAX
                )
                {
                    perror("read_net_stats(errors|dropped) error");
                    parse_failed = 1;
                    break;
                }

                uint64_t rx_bytes_delta = (rx_bytes_t2 >= rx_bytes_t1[i]) ? (rx_bytes_t2 - rx_bytes_t1[i]) : 0;
                uint64_t rx_packs_delta = (rx_packs_t2 >= rx_packs_t1[i]) ? (rx_packs_t2 - rx_packs_t1[i]) : 0;
                uint64_t tx_bytes_delta = (tx_bytes_t2 >= tx_bytes_t1[i]) ? (tx_bytes_t2 - tx_bytes_t1[i]) : 0;
                uint64_t tx_packs_delta = (tx_packs_t2 >= tx_packs_t1[i]) ? (tx_packs_t2 - tx_packs_t1[i]) : 0;

                rx_rate_kibs += (float) (rx_bytes_delta / interval_sec / 1024.0);
                rx_rate_kpps += (float) (rx_packs_delta / interval_sec / 1000.0);
                tx_rate_kibs += (float) (tx_bytes_delta / interval_sec / 1024.0);
                tx_rate_kpps += (float) (tx_packs_delta / interval_sec / 1000.0);

                rx_total_kibs += (float) (rx_bytes_t2 / 1024.0);
                tx_total_kibs += (float) (tx_bytes_t2 / 1024.0);

                errors_rx += (int) rx_errors;
                errors_tx += (int) tx_errors;
                drops_rx += (int) rx_dropped;
                drops_tx += (int) tx_dropped;
            }
        }

        free(rx_bytes_t1);
        free(rx_packs_t1);
        free(tx_bytes_t1);
        free(tx_packs_t1);

        if (!parse_failed)
        {
            TcpConnLst *tcl = NULL;
            size_t sz = 0;
            if (scan_tcp_conn_list(&tcl, &sz) != 0)
            {
                perror("scan_tcp_conn_list error");
                parse_failed = 1;
            }
            else
            {
                for (size_t j = 0; j < sz; ++j)
                {
                    if (strcmp(tcl[j].state, "ESTABLISHED") == 0) conn_estab++;
                    else if (strcmp(tcl[j].state, "LISTEN") == 0) conn_lst++;
                    else if (strcmp(tcl[j].state, "TIME_WAIT") == 0) conn_tmw++;
                    else if (strcmp(tcl[j].state, "SYN_SENT") == 0) conn_systn++;
                    else if (strcmp(tcl[j].state, "CLOSE_WAIT") == 0) conn_clsw++;
                    retr_pkg += (int) tcl[j].retrnsmt;
                }
            }
            free(tcl);
        }

        if (!parse_failed)
        {
            TcpConnLst *tcl6 = NULL;
            size_t sz6 = 0;
            if (scan_tcp6_conn_list(&tcl6, &sz6) != 0)
            {
                perror("scan_tcp6_conn_list error");
                parse_failed = 1;
            }
            else
            {
                for (size_t j = 0; j < sz6; ++j)
                {
                    if (strcmp(tcl6[j].state, "ESTABLISHED") == 0) conn_estab++;
                    else if (strcmp(tcl6[j].state, "LISTEN") == 0) conn_lst++;
                    else if (strcmp(tcl6[j].state, "TIME_WAIT") == 0) conn_tmw++;
                    else if (strcmp(tcl6[j].state, "SYN_SENT") == 0) conn_systn++;
                    else if (strcmp(tcl6[j].state, "CLOSE_WAIT") == 0) conn_clsw++;
                    retr_pkg += (int) tcl6[j].retrnsmt;
                }
            }
            free(tcl6);
        }

        if (!parse_failed)
        {
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
        // sleep(2);
        // break;
    }

    return NULL;
}
