#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "reactor/connections_sockets_event_update.h"

#include "data/connections_sockets_data.h"

typedef struct socket_raw
{
    char proto[8];
    char local_addr_hex[65];
    char local_port_hex[9];
    char peer_addr_hex[65];
    char peer_port_hex[9];
    char state_hex[3];
    char tx_queue_hex[17];
    char rx_queue_hex[17];
    unsigned long inode;
} SocketRaw;

typedef struct socket_row
{
    char proto[8];
    char state[16];
    unsigned long recv_q;
    unsigned long send_q;
    char local_ip[64];
    unsigned local_port;
    char peer_ip[64];
    unsigned peer_port;
    unsigned long inode;
    pid_t pid;
    char proc_name[64];
} SocketRow;

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

static const char *udp_state_name(uint8_t st)
{
    switch (st)
    {
        case 0x01: return "ESTABLISHED";
        case 0x07: return "UNCONN";
        case 0x0A: return "LISTEN";
        default:   return "UNKNOWN";
    }
}

static int parse_socket_line(const char *line, const char *proto, SocketRaw *raw)
{
    unsigned sl;

    if (line == NULL || proto == NULL || raw == NULL) return -1;

    memset(raw, 0, sizeof(*raw));
    snprintf(raw->proto, sizeof(raw->proto), "%s", proto);

    int n = sscanf(
        line,
        " %u: %64[0-9A-Fa-f]:%8[0-9A-Fa-f] %64[0-9A-Fa-f]:%8[0-9A-Fa-f] %2[0-9A-Fa-f] %16[0-9A-Fa-f]:%16[0-9A-Fa-f] %*2[0-9A-Fa-f]:%*8[0-9A-Fa-f] %*8[0-9A-Fa-f] %*u %*u %lu",
        &sl,
        raw->local_addr_hex,
        raw->local_port_hex,
        raw->peer_addr_hex,
        raw->peer_port_hex,
        raw->state_hex,
        raw->tx_queue_hex,
        raw->rx_queue_hex,
        &raw->inode
    );
    (void) sl;

    return (n == 9) ? 0 : -1;
}

static int read_socket_proto(const SocketRaw *raw, char out[8])
{
    if (raw == NULL || out == NULL) return -1;

    snprintf(out, 8, "%s", raw->proto);
    return 0;
}

static int read_hex_u8(const char *hex, uint8_t *out)
{
    char *end = NULL;
    unsigned long value;

    if (hex == NULL || out == NULL) return -1;

    errno = 0;
    value = strtoul(hex, &end, 16);
    if (errno != 0 || end == hex || *end != '\0' || value > 0xFFul) return -1;

    *out = (uint8_t) value;
    return 0;
}

static int read_hex_unsigned_long(const char *hex, unsigned long *out)
{
    char *end = NULL;

    if (hex == NULL || out == NULL) return -1;

    errno = 0;
    *out = strtoul(hex, &end, 16);
    if (errno != 0 || end == hex || *end != '\0') return -1;

    return 0;
}

static int read_socket_state(const SocketRaw *raw, char out[16])
{
    uint8_t st;
    const char *state_name;

    if (raw == NULL || out == NULL) return -1;
    if (read_hex_u8(raw->state_hex, &st) != 0) return -1;

    if (strncmp(raw->proto, "udp", 3) == 0) state_name = udp_state_name(st);
    else state_name = tcp_state_name(st);

    snprintf(out, 16, "%s", state_name);
    return 0;
}

static int read_socket_recv_q(const SocketRaw *raw, unsigned long *out)
{
    if (raw == NULL || out == NULL) return -1;
    return read_hex_unsigned_long(raw->rx_queue_hex, out);
}

static int read_socket_send_q(const SocketRaw *raw, unsigned long *out)
{
    if (raw == NULL || out == NULL) return -1;
    return read_hex_unsigned_long(raw->tx_queue_hex, out);
}

static int read_socket_port(const char *port_hex, unsigned *out)
{
    unsigned long value;

    if (port_hex == NULL || out == NULL) return -1;
    if (read_hex_unsigned_long(port_hex, &value) != 0) return -1;

    *out = (unsigned) value;
    return 0;
}

static int read_socket_ipv4_ip(const char *hex, char out[64])
{
    unsigned long value;

    if (hex == NULL || out == NULL) return -1;
    if (read_hex_unsigned_long(hex, &value) != 0) return -1;

    snprintf(
        out,
        64,
        "%lu.%lu.%lu.%lu",
        value & 0xFFul,
        (value >> 8) & 0xFFul,
        (value >> 16) & 0xFFul,
        (value >> 24) & 0xFFul
    );
    return 0;
}

static int read_socket_ipv6_ip(const char *hex, char out[64])
{
    unsigned char bytes[16];

    if (hex == NULL || out == NULL) return -1;
    if (strlen(hex) != 32) return -1;

    for (int block = 0; block < 4; ++block)
    {
        for (int byte = 0; byte < 4; ++byte)
        {
            char pair[3];
            pair[0] = hex[(block * 8) + ((3 - byte) * 2)];
            pair[1] = hex[(block * 8) + ((3 - byte) * 2) + 1];
            pair[2] = '\0';
            bytes[(block * 4) + byte] = (unsigned char) strtoul(pair, NULL, 16);
        }
    }

    if (inet_ntop(AF_INET6, bytes, out, 64) == NULL) return -1;
    return 0;
}

static int read_socket_local_ip(const SocketRaw *raw, char out[64])
{
    if (raw == NULL || out == NULL) return -1;

    if (strcmp(raw->proto, "tcp6") == 0 || strcmp(raw->proto, "udp6") == 0)
    {
        return read_socket_ipv6_ip(raw->local_addr_hex, out);
    }
    return read_socket_ipv4_ip(raw->local_addr_hex, out);
}

static int read_socket_local_port(const SocketRaw *raw, unsigned *out)
{
    if (raw == NULL || out == NULL) return -1;
    return read_socket_port(raw->local_port_hex, out);
}

static int read_socket_peer_ip(const SocketRaw *raw, char out[64])
{
    if (raw == NULL || out == NULL) return -1;

    if (strcmp(raw->proto, "tcp6") == 0 || strcmp(raw->proto, "udp6") == 0)
    {
        return read_socket_ipv6_ip(raw->peer_addr_hex, out);
    }
    return read_socket_ipv4_ip(raw->peer_addr_hex, out);
}

static int read_socket_peer_port(const SocketRaw *raw, unsigned *out)
{
    if (raw == NULL || out == NULL) return -1;
    return read_socket_port(raw->peer_port_hex, out);
}

static int read_socket_inode(const SocketRaw *raw, unsigned long *out)
{
    if (raw == NULL || out == NULL) return -1;

    *out = raw->inode;
    return 0;
}

static int read_socket_proc_pid(unsigned long inode, pid_t *out)
{
    DIR *proc_dir;
    struct dirent *proc_ent;
    char inode_link[64];

    if (out == NULL) return -1;

    snprintf(inode_link, sizeof(inode_link), "socket:[%lu]", inode);

    proc_dir = opendir("/proc");
    if (proc_dir == NULL) return -1;

    while ((proc_ent = readdir(proc_dir)) != NULL)
    {
        char *end = NULL;
        long pid_long;
        char fd_dir_path[PATH_MAX];
        DIR *fd_dir;
        struct dirent *fd_ent;

        if (proc_ent->d_name[0] == '.') continue;
        if (!isdigit((unsigned char) proc_ent->d_name[0])) continue;

        errno = 0;
        pid_long = strtol(proc_ent->d_name, &end, 10);
        if (errno != 0 || end == proc_ent->d_name || *end != '\0') continue;

        snprintf(fd_dir_path, sizeof(fd_dir_path), "/proc/%s/fd", proc_ent->d_name);
        fd_dir = opendir(fd_dir_path);
        if (fd_dir == NULL) continue;

        while ((fd_ent = readdir(fd_dir)) != NULL)
        {
            char link_path[PATH_MAX];
            char link_target[PATH_MAX];
            ssize_t link_len;

            if (fd_ent->d_name[0] == '.') continue;

            snprintf(link_path, sizeof(link_path), "%s/%s", fd_dir_path, fd_ent->d_name);
            link_len = readlink(link_path, link_target, sizeof(link_target) - 1);
            if (link_len < 0) continue;
            link_target[link_len] = '\0';

            if (strcmp(link_target, inode_link) == 0)
            {
                closedir(fd_dir);
                closedir(proc_dir);
                *out = (pid_t) pid_long;
                return 0;
            }
        }

        closedir(fd_dir);
    }

    closedir(proc_dir);
    return -1;
}

static int read_socket_proc_name(pid_t pid, char out[64])
{
    char path[PATH_MAX];
    FILE *f;

    if (out == NULL) return -1;

    snprintf(path, sizeof(path), "/proc/%ld/comm", (long) pid);
    f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(out, 64, f))
    {
        fclose(f);
        return -1;
    }
    fclose(f);

    size_t len = strcspn(out, "\r\n");
    out[len] = '\0';
    return 0;
}

static int read_socket_rows_from_file(
    const char *path,
    const char *proto,
    SocketRow **rows,
    int *count,
    int *cap
)
{
    FILE *f;
    char line[1024];

    if (path == NULL || proto == NULL || rows == NULL || count == NULL || cap == NULL) return -1;

    f = fopen(path, "r");
    if (!f) return -1;

    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    while (fgets(line, sizeof(line), f))
    {
        SocketRaw raw;
        SocketRow row;

        memset(&raw, 0, sizeof(raw));
        memset(&row, 0, sizeof(row));

        if (parse_socket_line(line, proto, &raw) != 0) continue;
        if (read_socket_proto(&raw, row.proto) != 0) continue;
        if (read_socket_state(&raw, row.state) != 0) continue;
        if (read_socket_recv_q(&raw, &row.recv_q) != 0) continue;
        if (read_socket_send_q(&raw, &row.send_q) != 0) continue;
        if (read_socket_local_ip(&raw, row.local_ip) != 0) continue;
        if (read_socket_local_port(&raw, &row.local_port) != 0) continue;
        if (read_socket_peer_ip(&raw, row.peer_ip) != 0) continue;
        if (read_socket_peer_port(&raw, &row.peer_port) != 0) continue;
        if (read_socket_inode(&raw, &row.inode) != 0) continue;

        row.pid = 0;
        row.proc_name[0] = '\0';
        if (read_socket_proc_pid(row.inode, &row.pid) == 0)
        {
            if (read_socket_proc_name(row.pid, row.proc_name) != 0)
            {
                row.proc_name[0] = '\0';
            }
        }

        if (ensure_capacity((void **) rows, cap, *count, sizeof(**rows)) != 0)
        {
            fclose(f);
            return -1;
        }

        (*rows)[(*count)++] = row;
    }

    fclose(f);
    return 0;
}

static void free_socket_payload(
    int count,
    char **proto,
    char **state,
    unsigned long *recv_q,
    unsigned long *send_q,
    char **local_ip,
    unsigned *local_port,
    char **peer_ip,
    unsigned *peer_port,
    unsigned long *inode,
    pid_t *pid,
    char **proc_name
)
{
    if (proto != NULL)
    {
        for (int i = 0; i < count; ++i) free(proto[i]);
        free(proto);
    }
    if (state != NULL)
    {
        for (int i = 0; i < count; ++i) free(state[i]);
        free(state);
    }
    if (local_ip != NULL)
    {
        for (int i = 0; i < count; ++i) free(local_ip[i]);
        free(local_ip);
    }
    if (peer_ip != NULL)
    {
        for (int i = 0; i < count; ++i) free(peer_ip[i]);
        free(peer_ip);
    }
    if (proc_name != NULL)
    {
        for (int i = 0; i < count; ++i) free(proc_name[i]);
        free(proc_name);
    }

    free(recv_q);
    free(send_q);
    free(local_port);
    free(peer_port);
    free(inode);
    free(pid);
}

static int build_socket_payload(
    SocketRow *rows,
    int count,
    char ***proto_out,
    char ***state_out,
    unsigned long **recv_q_out,
    unsigned long **send_q_out,
    char ***local_ip_out,
    unsigned **local_port_out,
    char ***peer_ip_out,
    unsigned **peer_port_out,
    unsigned long **inode_out,
    pid_t **pid_out,
    char ***proc_name_out
)
{
    char **proto = NULL;
    char **state = NULL;
    unsigned long *recv_q = NULL;
    unsigned long *send_q = NULL;
    char **local_ip = NULL;
    unsigned *local_port = NULL;
    char **peer_ip = NULL;
    unsigned *peer_port = NULL;
    unsigned long *inode = NULL;
    pid_t *pid = NULL;
    char **proc_name = NULL;

    *proto_out = NULL;
    *state_out = NULL;
    *recv_q_out = NULL;
    *send_q_out = NULL;
    *local_ip_out = NULL;
    *local_port_out = NULL;
    *peer_ip_out = NULL;
    *peer_port_out = NULL;
    *inode_out = NULL;
    *pid_out = NULL;
    *proc_name_out = NULL;

    if (count == 0) return 0;

    proto = calloc((size_t) count, sizeof(*proto));
    state = calloc((size_t) count, sizeof(*state));
    recv_q = calloc((size_t) count, sizeof(*recv_q));
    send_q = calloc((size_t) count, sizeof(*send_q));
    local_ip = calloc((size_t) count, sizeof(*local_ip));
    local_port = calloc((size_t) count, sizeof(*local_port));
    peer_ip = calloc((size_t) count, sizeof(*peer_ip));
    peer_port = calloc((size_t) count, sizeof(*peer_port));
    inode = calloc((size_t) count, sizeof(*inode));
    pid = calloc((size_t) count, sizeof(*pid));
    proc_name = calloc((size_t) count, sizeof(*proc_name));
    if (
        proto == NULL || state == NULL || recv_q == NULL || send_q == NULL ||
        local_ip == NULL || local_port == NULL || peer_ip == NULL ||
        peer_port == NULL || inode == NULL || pid == NULL || proc_name == NULL
    )
    {
        free_socket_payload(
            count,
            proto,
            state,
            recv_q,
            send_q,
            local_ip,
            local_port,
            peer_ip,
            peer_port,
            inode,
            pid,
            proc_name
        );
        return -1;
    }

    for (int i = 0; i < count; ++i)
    {
        proto[i] = strdup(rows[i].proto);
        state[i] = strdup(rows[i].state);
        local_ip[i] = strdup(rows[i].local_ip);
        peer_ip[i] = strdup(rows[i].peer_ip);
        proc_name[i] = strdup(rows[i].proc_name);
        recv_q[i] = rows[i].recv_q;
        send_q[i] = rows[i].send_q;
        local_port[i] = rows[i].local_port;
        peer_port[i] = rows[i].peer_port;
        inode[i] = rows[i].inode;
        pid[i] = rows[i].pid;
        if (
            proto[i] == NULL || state[i] == NULL || local_ip[i] == NULL ||
            peer_ip[i] == NULL || proc_name[i] == NULL
        )
        {
            free_socket_payload(
                count,
                proto,
                state,
                recv_q,
                send_q,
                local_ip,
                local_port,
                peer_ip,
                peer_port,
                inode,
                pid,
                proc_name
            );
            return -1;
        }
    }

    *proto_out = proto;
    *state_out = state;
    *recv_q_out = recv_q;
    *send_q_out = send_q;
    *local_ip_out = local_ip;
    *local_port_out = local_port;
    *peer_ip_out = peer_ip;
    *peer_port_out = peer_port;
    *inode_out = inode;
    *pid_out = pid;
    *proc_name_out = proc_name;
    return 0;
}

void* connections_sockets_event_update(void *arg)
{
    CONSOCK *consock = (CONSOCK *) arg;

    while (1)
    {
        SocketRow *rows = NULL;
        int count = 0;
        int cap = 0;

        char **proto = NULL;
        char **state = NULL;
        unsigned long *recv_q = NULL;
        unsigned long *send_q = NULL;
        char **local_ip = NULL;
        unsigned *local_port = NULL;
        char **peer_ip = NULL;
        unsigned *peer_port = NULL;
        unsigned long *inode = NULL;
        pid_t *pid = NULL;
        char **proc_name = NULL;

        if (read_socket_rows_from_file("/proc/net/tcp", "tcp", &rows, &count, &cap) != 0)
        {
            perror("read_socket_rows_from_file(/proc/net/tcp)");
            free(rows);
            sleep(1);
            continue;
        }
        if (read_socket_rows_from_file("/proc/net/tcp6", "tcp6", &rows, &count, &cap) != 0)
        {
            perror("read_socket_rows_from_file(/proc/net/tcp6)");
            free(rows);
            sleep(1);
            continue;
        }
        if (read_socket_rows_from_file("/proc/net/udp", "udp", &rows, &count, &cap) != 0)
        {
            perror("read_socket_rows_from_file(/proc/net/udp)");
            free(rows);
            sleep(1);
            continue;
        }
        if (read_socket_rows_from_file("/proc/net/udp6", "udp6", &rows, &count, &cap) != 0)
        {
            perror("read_socket_rows_from_file(/proc/net/udp6)");
            free(rows);
            sleep(1);
            continue;
        }

        if (
            build_socket_payload(
                rows,
                count,
                &proto,
                &state,
                &recv_q,
                &send_q,
                &local_ip,
                &local_port,
                &peer_ip,
                &peer_port,
                &inode,
                &pid,
                &proc_name
            ) != 0
        )
        {
            perror("build_socket_payload");
            free(rows);
            sleep(1);
            continue;
        }

        CONSOCK_update_data(
            consock,
            count,
            proto,
            state,
            recv_q,
            send_q,
            local_ip,
            local_port,
            peer_ip,
            peer_port,
            inode,
            pid,
            proc_name
        );

        free(rows);
        sleep(1);
    }

    return NULL;
}
