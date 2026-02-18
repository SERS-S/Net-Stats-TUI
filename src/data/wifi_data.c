#include "data/wifi_data.h"
#include <pthread.h>

void WIFI_init(WIFI *e)
{
    pthread_mutex_init(&e->mtx, NULL);
}

void WIFI_destroy(WIFI *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void WIFI_update_data(WIFI *e)
{
    pthread_mutex_lock(&e->mtx);
    // data update
    pthread_mutex_unlock(&e->mtx);
}

WIFI WIFI_get_data(WIFI *e)
{
    WIFI new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}

void draw_wifi(){
    mvprintw(6,2,"visualization of wifi data in progress...");  
}
