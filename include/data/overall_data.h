#ifndef OVERALL_DATA
#define OVERALL_DATA

#include <pthread.h>

typedef struct overall_data
{
    pthread_mutex_t mtx;
} OVRLL;

void OVRLL_init(OVRLL *e);
void OVRLL_destroy(OVRLL *e);
void OVRLL_update_data(OVRLL *e);
OVRLL OVRLL_get_data(OVRLL *e);

#endif
