#ifndef ADDR_DNS_DATA
#define ADDR_DNS_DATA

#include <pthread.h>

typedef struct addr_dns_data
{
    pthread_mutex_t mtx;
} ADDRDNS;

void ADDRDNS_init(ADDRDNS *e);
void ADDRDNS_destroy(ADDRDNS *e);
void ADDRDNS_update_data(ADDRDNS *e);
ADDRDNS ADDRDNS_get_data(ADDRDNS *e);
void draw_addrdns();

#endif
