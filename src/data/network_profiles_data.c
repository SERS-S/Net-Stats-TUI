#include "data/network_profiles_data.h"
#include <pthread.h>

void NETPROF_init(NETPROF *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void NETPROF_destroy(NETPROF *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void NETPROF_update_data(NETPROF *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

NETPROF NETPROF_get_data(NETPROF *e)
{
    NETPROF new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}

