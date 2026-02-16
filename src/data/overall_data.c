#include "data/overall_data.h"
#include <pthread.h>

void OVRLL_init(OVRLL *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void OVRLL_destroy(OVRLL *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void OVRLL_update_data(OVRLL *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

OVRLL OVRLL_get_data(OVRLL *e)
{
    OVRLL new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
