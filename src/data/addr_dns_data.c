#include "data/addr_dns_data.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void ADDRDNS_init(ADDRDNS *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->interf_name = NULL;
    e->ipv4_address = NULL;
    e->ipv4_mask = NULL;
    e->ipv6_address = NULL;
    e->ipv6_mask = NULL;
    e->manager = NULL;
    e->servers_list = NULL;
    e->search_list = NULL;
    e->resolv_path = NULL;
}

void ADDRDNS_destroy(ADDRDNS *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->interf_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->interf_name[i]);
        free(e->interf_name);
    }
    free(e->ipv4_address);
    free(e->ipv4_mask);
    free(e->ipv6_address);
    free(e->ipv6_mask);
    free(e->manager);
    free(e->servers_list);
    free(e->search_list);
    free(e->resolv_path);

    e->count = 0;
    e->interf_name = NULL;
    e->ipv4_address = NULL;
    e->ipv4_mask = NULL;
    e->ipv6_address = NULL;
    e->ipv6_mask = NULL;
    e->manager = NULL;
    e->servers_list = NULL;
    e->search_list = NULL;
    e->resolv_path = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

void ADDRDNS_update_data(
    ADDRDNS *e,
    int new_count,
    char **new_interf_name,
    uint8_t (*new_ipv4_address)[4],
    uint8_t *new_ipv4_mask,
    uint8_t (*new_ipv6_address)[16],
    uint8_t *new_ipv6_mask,
    char *new_manager,
    char *new_servers_list,
    char *new_search_list,
    char *new_resolv_path
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->interf_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->interf_name[i]);
        free(e->interf_name);
    }
    free(e->ipv4_address);
    free(e->ipv4_mask);
    free(e->ipv6_address);
    free(e->ipv6_mask);
    free(e->manager);
    free(e->servers_list);
    free(e->search_list);
    free(e->resolv_path);

    e->count = 0;
    e->interf_name = NULL;
    e->ipv4_address = NULL;
    e->ipv4_mask = NULL;
    e->ipv6_address = NULL;
    e->ipv6_mask = NULL;
    e->manager = NULL;
    e->servers_list = NULL;
    e->search_list = NULL;
    e->resolv_path = NULL;

    e->count = new_count;
    e->interf_name = new_interf_name;
    e->ipv4_address = new_ipv4_address;
    e->ipv4_mask = new_ipv4_mask;
    e->ipv6_address = new_ipv6_address;
    e->ipv6_mask = new_ipv6_mask;
    e->manager = new_manager;
    e->servers_list = new_servers_list;
    e->search_list = new_search_list;
    e->resolv_path = new_resolv_path;
    pthread_mutex_unlock(&e->mtx);
}

void ADDRDNS_free_copy(ADDRDNS *e)
{
    if (e == NULL) return;

    if (e->interf_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->interf_name[i]);
        free(e->interf_name);
    }
    free(e->ipv4_address);
    free(e->ipv4_mask);
    free(e->ipv6_address);
    free(e->ipv6_mask);
    free(e->manager);
    free(e->servers_list);
    free(e->search_list);
    free(e->resolv_path);

    e->count = 0;
    e->interf_name = NULL;
    e->ipv4_address = NULL;
    e->ipv4_mask = NULL;
    e->ipv6_address = NULL;
    e->ipv6_mask = NULL;
    e->manager = NULL;
    e->servers_list = NULL;
    e->search_list = NULL;
    e->resolv_path = NULL;
}

ADDRDNS ADDRDNS_get_data(ADDRDNS *e)
{
    ADDRDNS new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;

    if (e->count > 0)
    {
        new_e.interf_name = calloc((size_t) e->count, sizeof(*new_e.interf_name));
        new_e.ipv4_address = calloc((size_t) e->count, sizeof(*new_e.ipv4_address));
        new_e.ipv4_mask = calloc((size_t) e->count, sizeof(*new_e.ipv4_mask));
        new_e.ipv6_address = calloc((size_t) e->count, sizeof(*new_e.ipv6_address));
        new_e.ipv6_mask = calloc((size_t) e->count, sizeof(*new_e.ipv6_mask));

        if (
            new_e.interf_name == NULL || new_e.ipv4_address == NULL ||
            new_e.ipv4_mask == NULL || new_e.ipv6_address == NULL ||
            new_e.ipv6_mask == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            ADDRDNS_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->interf_name != NULL && e->interf_name[i] != NULL)
            {
                new_e.interf_name[i] = strdup(e->interf_name[i]);
                if (new_e.interf_name[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    ADDRDNS_free_copy(&new_e);
                    return new_e;
                }
            }

            if (e->ipv4_address != NULL) memcpy(new_e.ipv4_address[i], e->ipv4_address[i], sizeof(new_e.ipv4_address[i]));
            if (e->ipv4_mask != NULL) new_e.ipv4_mask[i] = e->ipv4_mask[i];
            if (e->ipv6_address != NULL) memcpy(new_e.ipv6_address[i], e->ipv6_address[i], sizeof(new_e.ipv6_address[i]));
            if (e->ipv6_mask != NULL) new_e.ipv6_mask[i] = e->ipv6_mask[i];
        }
    }

    if (e->manager != NULL)
    {
        new_e.manager = strdup(e->manager);
        if (new_e.manager == NULL)
        {
            pthread_mutex_unlock(&e->mtx);
            ADDRDNS_free_copy(&new_e);
            return new_e;
        }
    }

    if (e->servers_list != NULL)
    {
        new_e.servers_list = strdup(e->servers_list);
        if (new_e.servers_list == NULL)
        {
            pthread_mutex_unlock(&e->mtx);
            ADDRDNS_free_copy(&new_e);
            return new_e;
        }
    }

    if (e->search_list != NULL)
    {
        new_e.search_list = strdup(e->search_list);
        if (new_e.search_list == NULL)
        {
            pthread_mutex_unlock(&e->mtx);
            ADDRDNS_free_copy(&new_e);
            return new_e;
        }
    }

    if (e->resolv_path != NULL)
    {
        new_e.resolv_path = strdup(e->resolv_path);
        if (new_e.resolv_path == NULL)
        {
            pthread_mutex_unlock(&e->mtx);
            ADDRDNS_free_copy(&new_e);
            return new_e;
        }
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
