#ifndef CONNECTIONS_SOCKETS_DATA
#define CONNECTIONS_SOCKETS_DATA

#include <pthread.h>

typedef struct connections_sockets_data
{
    pthread_mutex_t mtx;
} CONSOCK;

void CONSOCK_init(CONSOCK *e);
void CONSOCK_destroy(CONSOCK *e);
void CONSOCK_update_data(CONSOCK *e);
CONSOCK CONSOCK_get_data(CONSOCK *e);

#endif
