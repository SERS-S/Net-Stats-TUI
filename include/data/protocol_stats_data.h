#ifndef PROTOCOL_STATS_DATA
#define PROTOCOL_STATS_DATA

#include <pthread.h>

typedef struct protocol_stats_data
{
    pthread_mutex_t mtx;
} PROTST;

void PROTST_init(PROTST *e);
void PROTST_destroy(PROTST *e);
void PROTST_update_data(PROTST *e);
PROTST PROTST_get_data(PROTST *e);

#endif
