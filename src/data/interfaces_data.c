#include "data/interfaces_data.h"
#include <pthread.h>
#include <stddef.h>

void INTRF_init(INTRF *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->active_intef_ct = 0;
    e->device_name = NULL;
    e->device_type = NULL;
    e->active_status = NULL;
    e->conn_name = NULL;
    e->tx_rate_kibs = NULL;
    e->rx_rate_kibs = NULL;
    e->mtu_interf = NULL;
    e->mac_address = NULL;
    for (int i = 0; i < 4; ++i) e->ipv4_address[i] = NULL;
    for (int i = 0; i < 16; ++i) e->ipv6_address[i] = NULL;
    for (int i = 0; i < 4; ++i) e->gw_ipv4_address[i] = NULL;
    for (int i = 0; i < 16; ++i) e->gw_ipv6_address[i] = NULL;
    e->rx_total_bytes = NULL;
    e->rx_total_packs = NULL;
    e->rx_total_drops = NULL;
    e->rx_total_errors = NULL;
    e->tx_total_bytes = NULL;
    e->tx_total_packs = NULL;
    e->tx_total_drops = NULL;
    e->tx_total_errors = NULL;
    e->device_link = NULL;
    for (int i = 0; i < 4; ++i) e->duplex_mode[i] = NULL;
    e->operstate_mode = NULL;
}

void INTRF_destroy(INTRF *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void INTRF_update_data(
    INTRF *e,
    int new_active_intef_ct,
    char **new_device_name,
    char **new_device_type,
    bool *new_active_status,
    char **new_conn_name,
    float *new_tx_rate_kibs,
    float *new_rx_rate_kibs,
    int32_t *new_mtu_interf,
    uint8_t **new_mac_address,
    uint8_t *new_ipv4_address[4],
    uint8_t *new_ipv6_address[16],
    uint8_t *new_gw_ipv4_address[4],
    uint8_t *new_gw_ipv6_address[16],
    int *new_rx_total_bytes,
    int *new_rx_total_packs,
    int *new_rx_total_drops,
    int *new_rx_total_errors,
    int *new_tx_total_bytes,
    int *new_tx_total_packs,
    int *new_tx_total_drops,
    int *new_tx_total_errors,
    int *new_device_link,
    char *new_duplex_mode[4],
    char **new_operstate_mode
)
{
    pthread_mutex_lock(&e->mtx);
    e->active_intef_ct = new_active_intef_ct;
    e->device_name = new_device_name;
    e->device_type = new_device_type;
    e->active_status = new_active_status;
    e->conn_name = new_conn_name;
    e->tx_rate_kibs = new_tx_rate_kibs;
    e->rx_rate_kibs = new_rx_rate_kibs;
    e->mtu_interf = new_mtu_interf;
    e->mac_address = new_mac_address;
    for (int i = 0; i < 4; ++i) e->ipv4_address[i] = new_ipv4_address[i];
    for (int i = 0; i < 16; ++i) e->ipv6_address[i] = new_ipv6_address[i];
    for (int i = 0; i < 4; ++i) e->gw_ipv4_address[i] = new_gw_ipv4_address[i];
    for (int i = 0; i < 16; ++i) e->gw_ipv6_address[i] = new_gw_ipv6_address[i];
    e->rx_total_bytes = new_rx_total_bytes;
    e->rx_total_packs = new_rx_total_packs;
    e->rx_total_drops = new_rx_total_drops;
    e->rx_total_errors = new_rx_total_errors;
    e->tx_total_bytes = new_tx_total_bytes;
    e->tx_total_packs = new_tx_total_packs;
    e->tx_total_drops = new_tx_total_drops;
    e->tx_total_errors = new_tx_total_errors;
    e->device_link = new_device_link;
    for (int i = 0; i < 4; ++i) e->duplex_mode[i] = new_duplex_mode[i];
    e->operstate_mode = new_operstate_mode;
    pthread_mutex_unlock(&e->mtx);
}

INTRF INTRF_get_data(INTRF *e)
{
    INTRF new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
