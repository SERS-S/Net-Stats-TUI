#include "data/wifi_data.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

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

void WIFI_free_copy(WIFI *e)
{
    if (e == NULL) return;

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
}

WIFI WIFI_get_data(WIFI *e)
{
    WIFI new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;

    if (e->count > 0)
    {
        new_e.iface = calloc((size_t) e->count, sizeof(*new_e.iface));
        new_e.state = calloc((size_t) e->count, sizeof(*new_e.state));
        new_e.ssid = calloc((size_t) e->count, sizeof(*new_e.ssid));
        new_e.bssid = calloc((size_t) e->count, sizeof(*new_e.bssid));
        new_e.rssi_dbm = calloc((size_t) e->count, sizeof(*new_e.rssi_dbm));
        new_e.quality_pct = calloc((size_t) e->count, sizeof(*new_e.quality_pct));
        new_e.tx_bitrate_mbps = calloc((size_t) e->count, sizeof(*new_e.tx_bitrate_mbps));
        new_e.rx_bitrate_mbps = calloc((size_t) e->count, sizeof(*new_e.rx_bitrate_mbps));
        new_e.mcs = calloc((size_t) e->count, sizeof(*new_e.mcs));
        new_e.retries_per_sec = calloc((size_t) e->count, sizeof(*new_e.retries_per_sec));
        new_e.beacon_loss_per_sec = calloc((size_t) e->count, sizeof(*new_e.beacon_loss_per_sec));

        if (
            new_e.iface == NULL || new_e.state == NULL ||
            new_e.ssid == NULL || new_e.bssid == NULL ||
            new_e.rssi_dbm == NULL || new_e.quality_pct == NULL ||
            new_e.tx_bitrate_mbps == NULL || new_e.rx_bitrate_mbps == NULL ||
            new_e.mcs == NULL || new_e.retries_per_sec == NULL ||
            new_e.beacon_loss_per_sec == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            WIFI_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->iface != NULL && e->iface[i] != NULL)
            {
                new_e.iface[i] = strdup(e->iface[i]);
                if (new_e.iface[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    WIFI_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->state != NULL && e->state[i] != NULL)
            {
                new_e.state[i] = strdup(e->state[i]);
                if (new_e.state[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    WIFI_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->ssid != NULL && e->ssid[i] != NULL)
            {
                new_e.ssid[i] = strdup(e->ssid[i]);
                if (new_e.ssid[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    WIFI_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->bssid != NULL && e->bssid[i] != NULL)
            {
                new_e.bssid[i] = strdup(e->bssid[i]);
                if (new_e.bssid[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    WIFI_free_copy(&new_e);
                    return new_e;
                }
            }
        }

        if (e->rssi_dbm != NULL) memcpy(new_e.rssi_dbm, e->rssi_dbm, (size_t) e->count * sizeof(*new_e.rssi_dbm));
        if (e->quality_pct != NULL) memcpy(new_e.quality_pct, e->quality_pct, (size_t) e->count * sizeof(*new_e.quality_pct));
        if (e->tx_bitrate_mbps != NULL) memcpy(new_e.tx_bitrate_mbps, e->tx_bitrate_mbps, (size_t) e->count * sizeof(*new_e.tx_bitrate_mbps));
        if (e->rx_bitrate_mbps != NULL) memcpy(new_e.rx_bitrate_mbps, e->rx_bitrate_mbps, (size_t) e->count * sizeof(*new_e.rx_bitrate_mbps));
        if (e->mcs != NULL) memcpy(new_e.mcs, e->mcs, (size_t) e->count * sizeof(*new_e.mcs));
        if (e->retries_per_sec != NULL) memcpy(new_e.retries_per_sec, e->retries_per_sec, (size_t) e->count * sizeof(*new_e.retries_per_sec));
        if (e->beacon_loss_per_sec != NULL) memcpy(new_e.beacon_loss_per_sec, e->beacon_loss_per_sec, (size_t) e->count * sizeof(*new_e.beacon_loss_per_sec));
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
