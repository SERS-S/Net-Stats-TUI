#ifndef ARP_ROUTE_DATA
#define ARP_ROUTE_DATA

#include <pthread.h>
#include <stdint.h>

typedef enum arp_route_row_type
{
    ARPRT_ROW_ROUTE = 1,
    ARPRT_ROW_NEIGHBOR = 2
} ARPRT_ROW_TYPE;

typedef struct arp_route_data
{
    pthread_mutex_t mtx;
    int count;
    uint8_t *row_type;
    char **route_kind;
    char **route_dst;
    uint8_t *route_prefix_len;
    char **route_gateway;
    char **route_dev;
    unsigned int *route_metric;
    unsigned int *route_flags;
    uint8_t *route_is_default;
    char **neighbor_ip;
    char **neighbor_mac;
    char **neighbor_dev;
    unsigned int *neighbor_arp_flags;
    char **neighbor_state;
    double *neighbor_last_seen_sec;
} ARPRT;

void ARPRT_init(ARPRT *e);
void ARPRT_destroy(ARPRT *e);
void ARPRT_update_data(
    ARPRT *e,
    int new_count,
    uint8_t *new_row_type,
    char **new_route_kind,
    char **new_route_dst,
    uint8_t *new_route_prefix_len,
    char **new_route_gateway,
    char **new_route_dev,
    unsigned int *new_route_metric,
    unsigned int *new_route_flags,
    uint8_t *new_route_is_default,
    char **new_neighbor_ip,
    char **new_neighbor_mac,
    char **new_neighbor_dev,
    unsigned int *new_neighbor_arp_flags,
    char **new_neighbor_state,
    double *new_neighbor_last_seen_sec
);
ARPRT ARPRT_get_data(ARPRT *e);

#endif
