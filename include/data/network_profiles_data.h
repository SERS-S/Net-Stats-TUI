#ifndef NETWORK_PROFILES_DATA
#define NETWORK_PROFILES_DATA

#include <pthread.h>

typedef struct network_profiles_data
{
    pthread_mutex_t mtx;
} NETPROF;

void NETPROF_init(NETPROF *e);
void NETPROF_destroy(NETPROF *e);
void NETPROF_update_data(NETPROF *e);
NETPROF NETPROF_get_data(NETPROF *e);
void draw_netprof();

#endif
