#ifndef INTERFACES_DATA
#define INTERFACES_DATA

#include <pthread.h>

typedef struct interfaces_data
{
    pthread_mutex_t mtx;
} INTRF;

void INTRF_init(INTRF *e);
void INTRF_destroy(INTRF *e);
void INTRF_update_data(INTRF *e);
INTRF INTRF_get_data(INTRF *e);

#endif
