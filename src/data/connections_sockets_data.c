#include "data/connections_sockets_data.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

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

void CONSOCK_free_copy(CONSOCK *e)
{
    if (e == NULL) return;

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
}

CONSOCK CONSOCK_get_data(CONSOCK *e)
{
    CONSOCK new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;

    if (e->count > 0)
    {
        new_e.proto = calloc((size_t) e->count, sizeof(*new_e.proto));
        new_e.state = calloc((size_t) e->count, sizeof(*new_e.state));
        new_e.recv_q = calloc((size_t) e->count, sizeof(*new_e.recv_q));
        new_e.send_q = calloc((size_t) e->count, sizeof(*new_e.send_q));
        new_e.local_ip = calloc((size_t) e->count, sizeof(*new_e.local_ip));
        new_e.local_port = calloc((size_t) e->count, sizeof(*new_e.local_port));
        new_e.peer_ip = calloc((size_t) e->count, sizeof(*new_e.peer_ip));
        new_e.peer_port = calloc((size_t) e->count, sizeof(*new_e.peer_port));
        new_e.inode = calloc((size_t) e->count, sizeof(*new_e.inode));
        new_e.pid = calloc((size_t) e->count, sizeof(*new_e.pid));
        new_e.proc_name = calloc((size_t) e->count, sizeof(*new_e.proc_name));

        if (
            new_e.proto == NULL || new_e.state == NULL ||
            new_e.recv_q == NULL || new_e.send_q == NULL ||
            new_e.local_ip == NULL || new_e.local_port == NULL ||
            new_e.peer_ip == NULL || new_e.peer_port == NULL ||
            new_e.inode == NULL || new_e.pid == NULL ||
            new_e.proc_name == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            CONSOCK_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->proto != NULL && e->proto[i] != NULL)
            {
                new_e.proto[i] = strdup(e->proto[i]);
                if (new_e.proto[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    CONSOCK_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->state != NULL && e->state[i] != NULL)
            {
                new_e.state[i] = strdup(e->state[i]);
                if (new_e.state[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    CONSOCK_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->local_ip != NULL && e->local_ip[i] != NULL)
            {
                new_e.local_ip[i] = strdup(e->local_ip[i]);
                if (new_e.local_ip[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    CONSOCK_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->peer_ip != NULL && e->peer_ip[i] != NULL)
            {
                new_e.peer_ip[i] = strdup(e->peer_ip[i]);
                if (new_e.peer_ip[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    CONSOCK_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->proc_name != NULL && e->proc_name[i] != NULL)
            {
                new_e.proc_name[i] = strdup(e->proc_name[i]);
                if (new_e.proc_name[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    CONSOCK_free_copy(&new_e);
                    return new_e;
                }
            }
        }

        if (e->recv_q != NULL) memcpy(new_e.recv_q, e->recv_q, (size_t) e->count * sizeof(*new_e.recv_q));
        if (e->send_q != NULL) memcpy(new_e.send_q, e->send_q, (size_t) e->count * sizeof(*new_e.send_q));
        if (e->local_port != NULL) memcpy(new_e.local_port, e->local_port, (size_t) e->count * sizeof(*new_e.local_port));
        if (e->peer_port != NULL) memcpy(new_e.peer_port, e->peer_port, (size_t) e->count * sizeof(*new_e.peer_port));
        if (e->inode != NULL) memcpy(new_e.inode, e->inode, (size_t) e->count * sizeof(*new_e.inode));
        if (e->pid != NULL) memcpy(new_e.pid, e->pid, (size_t) e->count * sizeof(*new_e.pid));
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
