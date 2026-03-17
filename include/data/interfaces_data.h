#ifndef INTERFACES_DATA
#define INTERFACES_DATA

#include <pthread.h>
#include <stdint.h>

typedef struct interfaces_data
{
    pthread_mutex_t mtx;
    int count;
    int active_interf_ct;
    char **device_name;
    char **device_type;
    char *active_status; // active_status_slot = 16 (каждые 16 бит следующее слово)
    char **conn_name;
    float *tx_rate_kibs;
    float *rx_rate_kibs;
    int32_t *mtu_interf;
    uint8_t (*mac_address)[6];
    uint8_t (*ipv4_address)[4];
    uint8_t (*ipv6_address)[16];
    uint8_t (*gw_ipv4_address)[4];
    uint8_t (*gw_ipv6_address)[16];
    int *rx_total_bytes;
    int *rx_total_packs;
    int *rx_total_drops;
    int *rx_total_errors;
    int *tx_total_bytes;
    int *tx_total_packs;
    int *tx_total_drops;
    int *tx_total_errors;
    int *device_link; // "{device_link//1000}G"
    char (*duplex_mode)[16]; // half | full | unknown
    char **operstate_mode;
} INTRF;

void INTRF_init(INTRF *e);
void INTRF_destroy(INTRF *e);
void INTRF_update_data(
    INTRF *e,
    int new_count,
    int new_active_interf_ct,
    char **new_device_name,
    char **new_device_type,
    char *new_active_status,
    char **new_conn_name,
    float *new_tx_rate_kibs,
    float *new_rx_rate_kibs,
    int32_t *new_mtu_interf,
    uint8_t (*new_mac_address)[6],
    uint8_t (*new_ipv4_address)[4],
    uint8_t (*new_ipv6_address)[16],
    uint8_t (*new_gw_ipv4_address)[4],
    uint8_t (*new_gw_ipv6_address)[16],
    int *new_rx_total_bytes,
    int *new_rx_total_packs,
    int *new_rx_total_drops,
    int *new_rx_total_errors,
    int *new_tx_total_bytes,
    int *new_tx_total_packs,
    int *new_tx_total_drops,
    int *new_tx_total_errors,
    int *new_device_link,
    char (*new_duplex_mode)[16],
    char **new_operstate_mode
);
INTRF INTRF_get_data(INTRF *e);
void INTRF_free_copy(INTRF *e);

#endif
