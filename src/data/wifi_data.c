#include "data/wifi_data.h"

#include <pthread.h>
#include <stdlib.h>

void WIFI_init(WIFI *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->iface = NULL;
    e->state = NULL;
    e->ssid = NULL;
    e->bssid = NULL;
    e->rssi_dbm = NULL;
    e->quality_pct = NULL;
    e->tx_bitrate_mbps = NULL;
    e->rx_bitrate_mbps = NULL;
    e->mcs = NULL;
    e->retries_per_sec = NULL;
    e->beacon_loss_per_sec = NULL;
}

void WIFI_destroy(WIFI *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->iface != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->iface[i]);
        free(e->iface);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }
    if (e->ssid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->ssid[i]);
        free(e->ssid);
    }
    if (e->bssid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->bssid[i]);
        free(e->bssid);
    }
    free(e->rssi_dbm);
    free(e->quality_pct);
    free(e->tx_bitrate_mbps);
    free(e->rx_bitrate_mbps);
    free(e->mcs);
    free(e->retries_per_sec);
    free(e->beacon_loss_per_sec);

    e->count = 0;
    e->iface = NULL;
    e->state = NULL;
    e->ssid = NULL;
    e->bssid = NULL;
    e->rssi_dbm = NULL;
    e->quality_pct = NULL;
    e->tx_bitrate_mbps = NULL;
    e->rx_bitrate_mbps = NULL;
    e->mcs = NULL;
    e->retries_per_sec = NULL;
    e->beacon_loss_per_sec = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

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
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->iface != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->iface[i]);
        free(e->iface);
    }
    if (e->state != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->state[i]);
        free(e->state);
    }
    if (e->ssid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->ssid[i]);
        free(e->ssid);
    }
    if (e->bssid != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->bssid[i]);
        free(e->bssid);
    }
    free(e->rssi_dbm);
    free(e->quality_pct);
    free(e->tx_bitrate_mbps);
    free(e->rx_bitrate_mbps);
    free(e->mcs);
    free(e->retries_per_sec);
    free(e->beacon_loss_per_sec);

    e->count = 0;
    e->iface = NULL;
    e->state = NULL;
    e->ssid = NULL;
    e->bssid = NULL;
    e->rssi_dbm = NULL;
    e->quality_pct = NULL;
    e->tx_bitrate_mbps = NULL;
    e->rx_bitrate_mbps = NULL;
    e->mcs = NULL;
    e->retries_per_sec = NULL;
    e->beacon_loss_per_sec = NULL;

    e->count = new_count;
    e->iface = new_iface;
    e->state = new_state;
    e->ssid = new_ssid;
    e->bssid = new_bssid;
    e->rssi_dbm = new_rssi_dbm;
    e->quality_pct = new_quality_pct;
    e->tx_bitrate_mbps = new_tx_bitrate_mbps;
    e->rx_bitrate_mbps = new_rx_bitrate_mbps;
    e->mcs = new_mcs;
    e->retries_per_sec = new_retries_per_sec;
    e->beacon_loss_per_sec = new_beacon_loss_per_sec;
    pthread_mutex_unlock(&e->mtx);
}

WIFI WIFI_get_data(WIFI *e)
{
    WIFI new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
