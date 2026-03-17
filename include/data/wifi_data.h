#ifndef WIFI_DATA
#define WIFI_DATA

#include <pthread.h>

typedef struct wifi_data
{
    pthread_mutex_t mtx;
    int count;
    char **iface;
    char **state;
    char **ssid;
    char **bssid;
    int *rssi_dbm;
    int *quality_pct;
    float *tx_bitrate_mbps;
    float *rx_bitrate_mbps;
    int *mcs;
    float *retries_per_sec;
    float *beacon_loss_per_sec;
} WIFI;

void WIFI_init(WIFI *e);
void WIFI_destroy(WIFI *e);
void WIFI_update_data(
    WIFI *e,
    int new_count,
    char **new_iface,
    char **new_state,
    char **new_ssid,
    char **new_bssid,
    int *new_rssi_dbm,
    int *new_quality_pct,
    float *new_tx_bitrate_mbps,
    float *new_rx_bitrate_mbps,
    int *new_mcs,
    float *new_retries_per_sec,
    float *new_beacon_loss_per_sec
);
WIFI WIFI_get_data(WIFI *e);
void WIFI_free_copy(WIFI *e);

#endif
