#include "data/connections_sockets_data.h"
#include <pthread.h>

void CONSOCK_init(CONSOCK *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void CONSOCK_destroy(CONSOCK *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void CONSOCK_update_data(CONSOCK *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

CONSOCK CONSOCK_get_data(CONSOCK *e)
{
    CONSOCK new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}

void draw_consock(){
    mvprintw(6,2,"visualization of socket connections in progress...");  
};
