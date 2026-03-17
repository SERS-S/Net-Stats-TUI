#include "data/arp_route_data.h"

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void ARPRT_init(ARPRT *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->row_type = NULL;
    e->route_kind = NULL;
    e->route_dst = NULL;
    e->route_prefix_len = NULL;
    e->route_gateway = NULL;
    e->route_dev = NULL;
    e->route_metric = NULL;
    e->route_flags = NULL;
    e->route_is_default = NULL;
    e->neighbor_ip = NULL;
    e->neighbor_mac = NULL;
    e->neighbor_dev = NULL;
    e->neighbor_arp_flags = NULL;
    e->neighbor_state = NULL;
    e->neighbor_last_seen_sec = NULL;
}

void ARPRT_destroy(ARPRT *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->route_kind != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_kind[i]);
        free(e->route_kind);
    }
    if (e->route_dst != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dst[i]);
        free(e->route_dst);
    }
    if (e->route_gateway != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_gateway[i]);
        free(e->route_gateway);
    }
    if (e->route_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dev[i]);
        free(e->route_dev);
    }
    if (e->neighbor_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_ip[i]);
        free(e->neighbor_ip);
    }
    if (e->neighbor_mac != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_mac[i]);
        free(e->neighbor_mac);
    }
    if (e->neighbor_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_dev[i]);
        free(e->neighbor_dev);
    }
    if (e->neighbor_state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_state[i]);
        free(e->neighbor_state);
    }

    free(e->row_type);
    free(e->route_prefix_len);
    free(e->route_metric);
    free(e->route_flags);
    free(e->route_is_default);
    free(e->neighbor_arp_flags);
    free(e->neighbor_last_seen_sec);

    e->count = 0;
    e->row_type = NULL;
    e->route_kind = NULL;
    e->route_dst = NULL;
    e->route_prefix_len = NULL;
    e->route_gateway = NULL;
    e->route_dev = NULL;
    e->route_metric = NULL;
    e->route_flags = NULL;
    e->route_is_default = NULL;
    e->neighbor_ip = NULL;
    e->neighbor_mac = NULL;
    e->neighbor_dev = NULL;
    e->neighbor_arp_flags = NULL;
    e->neighbor_state = NULL;
    e->neighbor_last_seen_sec = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

void ARPRT_update_data(
    ARPRT *e,
    int new_count,
    uint8_t *new_row_type,
    char **new_route_kind,
    char **new_route_dst,
    uint8_t *new_route_prefix_len,
    char **new_route_gateway,
    char **new_route_dev,
    unsigned int *new_route_metric,
    unsigned int *new_route_flags,
    uint8_t *new_route_is_default,
    char **new_neighbor_ip,
    char **new_neighbor_mac,
    char **new_neighbor_dev,
    unsigned int *new_neighbor_arp_flags,
    char **new_neighbor_state,
    double *new_neighbor_last_seen_sec
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->route_kind != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_kind[i]);
        free(e->route_kind);
    }
    if (e->route_dst != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dst[i]);
        free(e->route_dst);
    }
    if (e->route_gateway != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_gateway[i]);
        free(e->route_gateway);
    }
    if (e->route_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dev[i]);
        free(e->route_dev);
    }
    if (e->neighbor_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_ip[i]);
        free(e->neighbor_ip);
    }
    if (e->neighbor_mac != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_mac[i]);
        free(e->neighbor_mac);
    }
    if (e->neighbor_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_dev[i]);
        free(e->neighbor_dev);
    }
    if (e->neighbor_state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_state[i]);
        free(e->neighbor_state);
    }

    free(e->row_type);
    free(e->route_prefix_len);
    free(e->route_metric);
    free(e->route_flags);
    free(e->route_is_default);
    free(e->neighbor_arp_flags);
    free(e->neighbor_last_seen_sec);

    e->count = 0;
    e->row_type = NULL;
    e->route_kind = NULL;
    e->route_dst = NULL;
    e->route_prefix_len = NULL;
    e->route_gateway = NULL;
    e->route_dev = NULL;
    e->route_metric = NULL;
    e->route_flags = NULL;
    e->route_is_default = NULL;
    e->neighbor_ip = NULL;
    e->neighbor_mac = NULL;
    e->neighbor_dev = NULL;
    e->neighbor_arp_flags = NULL;
    e->neighbor_state = NULL;
    e->neighbor_last_seen_sec = NULL;

    e->count = new_count;
    e->row_type = new_row_type;
    e->route_kind = new_route_kind;
    e->route_dst = new_route_dst;
    e->route_prefix_len = new_route_prefix_len;
    e->route_gateway = new_route_gateway;
    e->route_dev = new_route_dev;
    e->route_metric = new_route_metric;
    e->route_flags = new_route_flags;
    e->route_is_default = new_route_is_default;
    e->neighbor_ip = new_neighbor_ip;
    e->neighbor_mac = new_neighbor_mac;
    e->neighbor_dev = new_neighbor_dev;
    e->neighbor_arp_flags = new_neighbor_arp_flags;
    e->neighbor_state = new_neighbor_state;
    e->neighbor_last_seen_sec = new_neighbor_last_seen_sec;
    pthread_mutex_unlock(&e->mtx);
}

void ARPRT_free_copy(ARPRT *e)
{
    if (e == NULL) return;

    if (e->route_kind != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_kind[i]);
        free(e->route_kind);
    }
    if (e->route_dst != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dst[i]);
        free(e->route_dst);
    }
    if (e->route_gateway != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_gateway[i]);
        free(e->route_gateway);
    }
    if (e->route_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->route_dev[i]);
        free(e->route_dev);
    }
    if (e->neighbor_ip != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_ip[i]);
        free(e->neighbor_ip);
    }
    if (e->neighbor_mac != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_mac[i]);
        free(e->neighbor_mac);
    }
    if (e->neighbor_dev != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_dev[i]);
        free(e->neighbor_dev);
    }
    if (e->neighbor_state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->neighbor_state[i]);
        free(e->neighbor_state);
    }

    free(e->row_type);
    free(e->route_prefix_len);
    free(e->route_metric);
    free(e->route_flags);
    free(e->route_is_default);
    free(e->neighbor_arp_flags);
    free(e->neighbor_last_seen_sec);

    e->count = 0;
    e->row_type = NULL;
    e->route_kind = NULL;
    e->route_dst = NULL;
    e->route_prefix_len = NULL;
    e->route_gateway = NULL;
    e->route_dev = NULL;
    e->route_metric = NULL;
    e->route_flags = NULL;
    e->route_is_default = NULL;
    e->neighbor_ip = NULL;
    e->neighbor_mac = NULL;
    e->neighbor_dev = NULL;
    e->neighbor_arp_flags = NULL;
    e->neighbor_state = NULL;
    e->neighbor_last_seen_sec = NULL;
}

ARPRT ARPRT_get_data(ARPRT *e)
{
    ARPRT new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;

    if (e->count > 0)
    {
        new_e.row_type = calloc((size_t) e->count, sizeof(*new_e.row_type));
        new_e.route_kind = calloc((size_t) e->count, sizeof(*new_e.route_kind));
        new_e.route_dst = calloc((size_t) e->count, sizeof(*new_e.route_dst));
        new_e.route_prefix_len = calloc((size_t) e->count, sizeof(*new_e.route_prefix_len));
        new_e.route_gateway = calloc((size_t) e->count, sizeof(*new_e.route_gateway));
        new_e.route_dev = calloc((size_t) e->count, sizeof(*new_e.route_dev));
        new_e.route_metric = calloc((size_t) e->count, sizeof(*new_e.route_metric));
        new_e.route_flags = calloc((size_t) e->count, sizeof(*new_e.route_flags));
        new_e.route_is_default = calloc((size_t) e->count, sizeof(*new_e.route_is_default));
        new_e.neighbor_ip = calloc((size_t) e->count, sizeof(*new_e.neighbor_ip));
        new_e.neighbor_mac = calloc((size_t) e->count, sizeof(*new_e.neighbor_mac));
        new_e.neighbor_dev = calloc((size_t) e->count, sizeof(*new_e.neighbor_dev));
        new_e.neighbor_arp_flags = calloc((size_t) e->count, sizeof(*new_e.neighbor_arp_flags));
        new_e.neighbor_state = calloc((size_t) e->count, sizeof(*new_e.neighbor_state));
        new_e.neighbor_last_seen_sec = calloc((size_t) e->count, sizeof(*new_e.neighbor_last_seen_sec));

        if (
            new_e.row_type == NULL || new_e.route_kind == NULL ||
            new_e.route_dst == NULL || new_e.route_prefix_len == NULL ||
            new_e.route_gateway == NULL || new_e.route_dev == NULL ||
            new_e.route_metric == NULL || new_e.route_flags == NULL ||
            new_e.route_is_default == NULL || new_e.neighbor_ip == NULL ||
            new_e.neighbor_mac == NULL || new_e.neighbor_dev == NULL ||
            new_e.neighbor_arp_flags == NULL || new_e.neighbor_state == NULL ||
            new_e.neighbor_last_seen_sec == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            ARPRT_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->route_kind != NULL && e->route_kind[i] != NULL)
            {
                new_e.route_kind[i] = strdup(e->route_kind[i]);
                if (new_e.route_kind[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->route_dst != NULL && e->route_dst[i] != NULL)
            {
                new_e.route_dst[i] = strdup(e->route_dst[i]);
                if (new_e.route_dst[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->route_gateway != NULL && e->route_gateway[i] != NULL)
            {
                new_e.route_gateway[i] = strdup(e->route_gateway[i]);
                if (new_e.route_gateway[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->route_dev != NULL && e->route_dev[i] != NULL)
            {
                new_e.route_dev[i] = strdup(e->route_dev[i]);
                if (new_e.route_dev[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->neighbor_ip != NULL && e->neighbor_ip[i] != NULL)
            {
                new_e.neighbor_ip[i] = strdup(e->neighbor_ip[i]);
                if (new_e.neighbor_ip[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->neighbor_mac != NULL && e->neighbor_mac[i] != NULL)
            {
                new_e.neighbor_mac[i] = strdup(e->neighbor_mac[i]);
                if (new_e.neighbor_mac[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->neighbor_dev != NULL && e->neighbor_dev[i] != NULL)
            {
                new_e.neighbor_dev[i] = strdup(e->neighbor_dev[i]);
                if (new_e.neighbor_dev[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->neighbor_state != NULL && e->neighbor_state[i] != NULL)
            {
                new_e.neighbor_state[i] = strdup(e->neighbor_state[i]);
                if (new_e.neighbor_state[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ARPRT_free_copy(&new_e);
                    return new_e;
                }
            }
        }

        if (e->row_type != NULL) memcpy(new_e.row_type, e->row_type, (size_t) e->count * sizeof(*new_e.row_type));
        if (e->route_prefix_len != NULL) memcpy(new_e.route_prefix_len, e->route_prefix_len, (size_t) e->count * sizeof(*new_e.route_prefix_len));
        if (e->route_metric != NULL) memcpy(new_e.route_metric, e->route_metric, (size_t) e->count * sizeof(*new_e.route_metric));
        if (e->route_flags != NULL) memcpy(new_e.route_flags, e->route_flags, (size_t) e->count * sizeof(*new_e.route_flags));
        if (e->route_is_default != NULL) memcpy(new_e.route_is_default, e->route_is_default, (size_t) e->count * sizeof(*new_e.route_is_default));
        if (e->neighbor_arp_flags != NULL) memcpy(new_e.neighbor_arp_flags, e->neighbor_arp_flags, (size_t) e->count * sizeof(*new_e.neighbor_arp_flags));
        if (e->neighbor_last_seen_sec != NULL) memcpy(new_e.neighbor_last_seen_sec, e->neighbor_last_seen_sec, (size_t) e->count * sizeof(*new_e.neighbor_last_seen_sec));
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
