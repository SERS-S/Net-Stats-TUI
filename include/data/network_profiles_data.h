#ifndef NETWORK_PROFILES_DATA
#define NETWORK_PROFILES_DATA

#include <pthread.h>

typedef struct network_profiles_data
{
    pthread_mutex_t mtx;
    int count;
    char **name;
    char **uuid;
    char **type;
    char **device;
    char **state;
} NETPROF;

void NETPROF_init(NETPROF *e);
void NETPROF_destroy(NETPROF *e);
void NETPROF_update_data(
    NETPROF *e,
    int new_count,
    char **new_name,
    char **new_uuid,
    char **new_type,
    char **new_device,
    char **new_state
);
NETPROF NETPROF_get_data(NETPROF *e);

#endif
