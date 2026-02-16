#include "data/arp_route_data.h"
#include <pthread.h>

void ARPRT_init(ARPRT *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void ARPRT_destroy(ARPRT *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void ARPRT_update_data(ARPRT *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

ARPRT ARPRT_get_data(ARPRT *e)
{
    ARPRT new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
