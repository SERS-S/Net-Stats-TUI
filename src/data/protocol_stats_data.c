#include "data/protocol_stats_data.h"
#include <pthread.h>

void PROTST_init(PROTST *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void PROTST_destroy(PROTST *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void PROTST_update_data(PROTST *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

PROTST PROTST_get_data(PROTST *e)
{
    PROTST new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}

