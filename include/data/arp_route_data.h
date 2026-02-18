#ifndef ARP_ROUTE_DATA
#define ARP_ROUTE_DATA

#include <pthread.h>

typedef struct arp_route_data
{
    pthread_mutex_t mtx;
} ARPRT;

void ARPRT_init(ARPRT *e);
void ARPRT_destroy(ARPRT *e);
void ARPRT_update_data(ARPRT *e);
ARPRT ARPRT_get_data(ARPRT *e);

#endif
