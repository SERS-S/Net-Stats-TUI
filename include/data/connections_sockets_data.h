#ifndef CONNECTIONS_SOCKETS_DATA
#define CONNECTIONS_SOCKETS_DATA

#include <pthread.h>
#include <sys/types.h>

typedef struct connections_sockets_data
{
    pthread_mutex_t mtx;
    int count;
    char **proto;
    char **state;
    unsigned long *recv_q;
    unsigned long *send_q;
    char **local_ip;
    unsigned *local_port;
    char **peer_ip;
    unsigned *peer_port;
    unsigned long *inode;
    pid_t *pid;
    char **proc_name;
} CONSOCK;

void CONSOCK_init(CONSOCK *e);
void CONSOCK_destroy(CONSOCK *e);
void CONSOCK_update_data(
    CONSOCK *e,
    int new_count,
    char **new_proto,
    char **new_state,
    unsigned long *new_recv_q,
    unsigned long *new_send_q,
    char **new_local_ip,
    unsigned *new_local_port,
    char **new_peer_ip,
    unsigned *new_peer_port,
    unsigned long *new_inode,
    pid_t *new_pid,
    char **new_proc_name
);
CONSOCK CONSOCK_get_data(CONSOCK *e);

#endif
