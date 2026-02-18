#include "data/addr_dns_data.h"
#include <pthread.h>

void ADDRDNS_init(ADDRDNS *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void ADDRDNS_destroy(ADDRDNS *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void ADDRDNS_update_data(ADDRDNS *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
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

