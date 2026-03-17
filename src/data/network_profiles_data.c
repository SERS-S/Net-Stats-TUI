#include "data/network_profiles_data.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

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

void NETPROF_free_copy(NETPROF *e)
{
    if (e == NULL) return;

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
}

NETPROF NETPROF_get_data(NETPROF *e)
{
    NETPROF new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;

    if (e->count > 0)
    {
        new_e.name = calloc((size_t) e->count, sizeof(*new_e.name));
        new_e.uuid = calloc((size_t) e->count, sizeof(*new_e.uuid));
        new_e.type = calloc((size_t) e->count, sizeof(*new_e.type));
        new_e.device = calloc((size_t) e->count, sizeof(*new_e.device));
        new_e.state = calloc((size_t) e->count, sizeof(*new_e.state));

        if (
            new_e.name == NULL || new_e.uuid == NULL ||
            new_e.type == NULL || new_e.device == NULL ||
            new_e.state == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            NETPROF_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->name != NULL && e->name[i] != NULL)
            {
                new_e.name[i] = strdup(e->name[i]);
                if (new_e.name[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    NETPROF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->uuid != NULL && e->uuid[i] != NULL)
            {
                new_e.uuid[i] = strdup(e->uuid[i]);
                if (new_e.uuid[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    NETPROF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->type != NULL && e->type[i] != NULL)
            {
                new_e.type[i] = strdup(e->type[i]);
                if (new_e.type[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    NETPROF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->device != NULL && e->device[i] != NULL)
            {
                new_e.device[i] = strdup(e->device[i]);
                if (new_e.device[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    NETPROF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->state != NULL && e->state[i] != NULL)
            {
                new_e.state[i] = strdup(e->state[i]);
                if (new_e.state[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    NETPROF_free_copy(&new_e);
                    return new_e;
                }
            }
        }
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
