#include "data/interfaces_data.h"
#include <pthread.h>

void INTRF_init(INTRF *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void INTRF_destroy(INTRF *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void INTRF_update_data(INTRF *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

INTRF INTRF_get_data(INTRF *e)
{
    INTRF new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}

