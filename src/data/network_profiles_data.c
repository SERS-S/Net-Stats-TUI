#include "data/network_profiles_data.h"

#include <pthread.h>
#include <stdlib.h>

void NETPROF_init(NETPROF *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->name = NULL;
    e->uuid = NULL;
    e->type = NULL;
    e->device = NULL;
    e->state = NULL;
}

void NETPROF_destroy(NETPROF *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->name[i]);
        free(e->name);
    }
    if (e->uuid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->uuid[i]);
        free(e->uuid);
    }
    if (e->type != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->type[i]);
        free(e->type);
    }
    if (e->device != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device[i]);
        free(e->device);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }

    e->count = 0;
    e->name = NULL;
    e->uuid = NULL;
    e->type = NULL;
    e->device = NULL;
    e->state = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

void NETPROF_update_data(
    NETPROF *e,
    int new_count,
    char **new_name,
    char **new_uuid,
    char **new_type,
    char **new_device,
    char **new_state
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->name[i]);
        free(e->name);
    }
    if (e->uuid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->uuid[i]);
        free(e->uuid);
    }
    if (e->type != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->type[i]);
        free(e->type);
    }
    if (e->device != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device[i]);
        free(e->device);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }

    e->count = 0;
    e->name = NULL;
    e->uuid = NULL;
    e->type = NULL;
    e->device = NULL;
    e->state = NULL;

    e->count = new_count;
    e->name = new_name;
    e->uuid = new_uuid;
    e->type = new_type;
    e->device = new_device;
    e->state = new_state;
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
