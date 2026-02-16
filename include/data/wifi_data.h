#ifndef WIFI_DATA
#define WIFI_DATA

#include <pthread.h>

typedef struct wifi_data
{
    pthread_mutex_t mtx;
} WIFI;

void WIFI_init(WIFI *e);
void WIFI_destroy(WIFI *e);
void WIFI_update_data(WIFI *e);
WIFI WIFI_get_data(WIFI *e);

#endif
