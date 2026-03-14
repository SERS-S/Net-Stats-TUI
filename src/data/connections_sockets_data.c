#include "data/connections_sockets_data.h"

#include <pthread.h>
#include <stdlib.h>

void CONSOCK_init(CONSOCK *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->proto = NULL;
    e->state = NULL;
    e->recv_q = NULL;
    e->send_q = NULL;
    e->local_ip = NULL;
    e->local_port = NULL;
    e->peer_ip = NULL;
    e->peer_port = NULL;
    e->inode = NULL;
    e->pid = NULL;
    e->proc_name = NULL;
}

void CONSOCK_destroy(CONSOCK *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->proto != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->proto[i]);
        free(e->proto);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }
    if (e->local_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->local_ip[i]);
        free(e->local_ip);
    }
    if (e->peer_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->peer_ip[i]);
        free(e->peer_ip);
    }
    if (e->proc_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->proc_name[i]);
        free(e->proc_name);
    }

    free(e->recv_q);
    free(e->send_q);
    free(e->local_port);
    free(e->peer_port);
    free(e->inode);
    free(e->pid);

    e->count = 0;
    e->proto = NULL;
    e->state = NULL;
    e->recv_q = NULL;
    e->send_q = NULL;
    e->local_ip = NULL;
    e->local_port = NULL;
    e->peer_ip = NULL;
    e->peer_port = NULL;
    e->inode = NULL;
    e->pid = NULL;
    e->proc_name = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

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
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->proto != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->proto[i]);
        free(e->proto);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }
    if (e->local_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->local_ip[i]);
        free(e->local_ip);
    }
    if (e->peer_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->peer_ip[i]);
        free(e->peer_ip);
    }
    if (e->proc_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->proc_name[i]);
        free(e->proc_name);
    }

    free(e->recv_q);
    free(e->send_q);
    free(e->local_port);
    free(e->peer_port);
    free(e->inode);
    free(e->pid);

    e->count = 0;
    e->proto = NULL;
    e->state = NULL;
    e->recv_q = NULL;
    e->send_q = NULL;
    e->local_ip = NULL;
    e->local_port = NULL;
    e->peer_ip = NULL;
    e->peer_port = NULL;
    e->inode = NULL;
    e->pid = NULL;
    e->proc_name = NULL;

    e->count = new_count;
    e->proto = new_proto;
    e->state = new_state;
    e->recv_q = new_recv_q;
    e->send_q = new_send_q;
    e->local_ip = new_local_ip;
    e->local_port = new_local_port;
    e->peer_ip = new_peer_ip;
    e->peer_port = new_peer_port;
    e->inode = new_inode;
    e->pid = new_pid;
    e->proc_name = new_proc_name;
    pthread_mutex_unlock(&e->mtx);
}

CONSOCK CONSOCK_get_data(CONSOCK *e)
{
    CONSOCK new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
