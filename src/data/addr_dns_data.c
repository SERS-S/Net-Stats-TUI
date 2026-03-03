#include "data/addr_dns_data.h"
#include <pthread.h>
#include <stddef.h>

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

ADDRDNS ADDRDNS_get_data(ADDRDNS *e)
{
    ADDRDNS new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
