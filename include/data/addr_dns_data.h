#ifndef ADDR_DNS_DATA
#define ADDR_DNS_DATA

#include <pthread.h>
#include <stdint.h>

typedef struct addr_dns_data
{
    pthread_mutex_t mtx;
    int count;
    char **interf_name;
    uint8_t (*ipv4_address)[4];
    uint8_t *ipv4_mask;
    uint8_t (*ipv6_address)[16];
    uint8_t *ipv6_mask;
    char *manager; // строка manager
    char *servers_list;
    char *search_list;
    char *resolv_path;
} ADDRDNS;

void ADDRDNS_init(ADDRDNS *e);
void ADDRDNS_destroy(ADDRDNS *e);
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
);
ADDRDNS ADDRDNS_get_data(ADDRDNS *e);

#endif
