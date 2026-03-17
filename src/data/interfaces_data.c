#include "data/interfaces_data.h"
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void INTRF_init(INTRF *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->count = 0;
    e->active_interf_ct = 0;
    e->device_name = NULL;
    e->device_type = NULL;
    e->active_status = NULL;
    e->conn_name = NULL;
    e->tx_rate_kibs = NULL;
    e->rx_rate_kibs = NULL;
    e->mtu_interf = NULL;
    e->mac_address = NULL;
    e->ipv4_address = NULL;
    e->ipv6_address = NULL;
    e->gw_ipv4_address = NULL;
    e->gw_ipv6_address = NULL;
    e->rx_total_bytes = NULL;
    e->rx_total_packs = NULL;
    e->rx_total_drops = NULL;
    e->rx_total_errors = NULL;
    e->tx_total_bytes = NULL;
    e->tx_total_packs = NULL;
    e->tx_total_drops = NULL;
    e->tx_total_errors = NULL;
    e->device_link = NULL;
    e->duplex_mode = NULL;
    e->operstate_mode = NULL;
}

void INTRF_destroy(INTRF *e)
{
    pthread_mutex_lock(&e->mtx);
    if (e->device_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_name[i]);
        free(e->device_name);
    }
    if (e->device_type != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_type[i]);
        free(e->device_type);
    }
    if (e->conn_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->conn_name[i]);
        free(e->conn_name);
    }
    if (e->operstate_mode != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->operstate_mode[i]);
        free(e->operstate_mode);
    }

    free(e->active_status);
    free(e->tx_rate_kibs);
    free(e->rx_rate_kibs);
    free(e->mtu_interf);
    free(e->mac_address);
    free(e->ipv4_address);
    free(e->ipv6_address);
    free(e->gw_ipv4_address);
    free(e->gw_ipv6_address);
    free(e->rx_total_bytes);
    free(e->rx_total_packs);
    free(e->rx_total_drops);
    free(e->rx_total_errors);
    free(e->tx_total_bytes);
    free(e->tx_total_packs);
    free(e->tx_total_drops);
    free(e->tx_total_errors);
    free(e->device_link);
    free(e->duplex_mode);

    e->count = 0;
    e->active_interf_ct = 0;
    e->device_name = NULL;
    e->device_type = NULL;
    e->active_status = NULL;
    e->conn_name = NULL;
    e->tx_rate_kibs = NULL;
    e->rx_rate_kibs = NULL;
    e->mtu_interf = NULL;
    e->mac_address = NULL;
    e->ipv4_address = NULL;
    e->ipv6_address = NULL;
    e->gw_ipv4_address = NULL;
    e->gw_ipv6_address = NULL;
    e->rx_total_bytes = NULL;
    e->rx_total_packs = NULL;
    e->rx_total_drops = NULL;
    e->rx_total_errors = NULL;
    e->tx_total_bytes = NULL;
    e->tx_total_packs = NULL;
    e->tx_total_drops = NULL;
    e->tx_total_errors = NULL;
    e->device_link = NULL;
    e->duplex_mode = NULL;
    e->operstate_mode = NULL;
    pthread_mutex_unlock(&e->mtx);
    pthread_mutex_destroy(&e->mtx);
}

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
)
{
    pthread_mutex_lock(&e->mtx);
    if (e->device_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_name[i]);
        free(e->device_name);
    }
    if (e->device_type != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_type[i]);
        free(e->device_type);
    }
    if (e->conn_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->conn_name[i]);
        free(e->conn_name);
    }
    if (e->operstate_mode != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->operstate_mode[i]);
        free(e->operstate_mode);
    }

    free(e->active_status);
    free(e->tx_rate_kibs);
    free(e->rx_rate_kibs);
    free(e->mtu_interf);
    free(e->mac_address);
    free(e->ipv4_address);
    free(e->ipv6_address);
    free(e->gw_ipv4_address);
    free(e->gw_ipv6_address);
    free(e->rx_total_bytes);
    free(e->rx_total_packs);
    free(e->rx_total_drops);
    free(e->rx_total_errors);
    free(e->tx_total_bytes);
    free(e->tx_total_packs);
    free(e->tx_total_drops);
    free(e->tx_total_errors);
    free(e->device_link);
    free(e->duplex_mode);

    e->count = 0;
    e->active_interf_ct = 0;
    e->device_name = NULL;
    e->device_type = NULL;
    e->active_status = NULL;
    e->conn_name = NULL;
    e->tx_rate_kibs = NULL;
    e->rx_rate_kibs = NULL;
    e->mtu_interf = NULL;
    e->mac_address = NULL;
    e->ipv4_address = NULL;
    e->ipv6_address = NULL;
    e->gw_ipv4_address = NULL;
    e->gw_ipv6_address = NULL;
    e->rx_total_bytes = NULL;
    e->rx_total_packs = NULL;
    e->rx_total_drops = NULL;
    e->rx_total_errors = NULL;
    e->tx_total_bytes = NULL;
    e->tx_total_packs = NULL;
    e->tx_total_drops = NULL;
    e->tx_total_errors = NULL;
    e->device_link = NULL;
    e->duplex_mode = NULL;
    e->operstate_mode = NULL;

    e->count = new_count;
    e->active_interf_ct = new_active_interf_ct;
    e->device_name = new_device_name;
    e->device_type = new_device_type;
    e->active_status = new_active_status;
    e->conn_name = new_conn_name;
    e->tx_rate_kibs = new_tx_rate_kibs;
    e->rx_rate_kibs = new_rx_rate_kibs;
    e->mtu_interf = new_mtu_interf;
    e->mac_address = new_mac_address;
    e->ipv4_address = new_ipv4_address;
    e->ipv6_address = new_ipv6_address;
    e->gw_ipv4_address = new_gw_ipv4_address;
    e->gw_ipv6_address = new_gw_ipv6_address;
    e->rx_total_bytes = new_rx_total_bytes;
    e->rx_total_packs = new_rx_total_packs;
    e->rx_total_drops = new_rx_total_drops;
    e->rx_total_errors = new_rx_total_errors;
    e->tx_total_bytes = new_tx_total_bytes;
    e->tx_total_packs = new_tx_total_packs;
    e->tx_total_drops = new_tx_total_drops;
    e->tx_total_errors = new_tx_total_errors;
    e->device_link = new_device_link;
    e->duplex_mode = new_duplex_mode;
    e->operstate_mode = new_operstate_mode;
    pthread_mutex_unlock(&e->mtx);
}

void INTRF_free_copy(INTRF *e)
{
    if (e == NULL) return;

    if (e->device_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_name[i]);
        free(e->device_name);
    }
    if (e->device_type != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->device_type[i]);
        free(e->device_type);
    }
    if (e->conn_name != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->conn_name[i]);
        free(e->conn_name);
    }
    if (e->operstate_mode != NULL)
    {
        for (int i = 0; i < e->count; ++i) free(e->operstate_mode[i]);
        free(e->operstate_mode);
    }

    free(e->active_status);
    free(e->tx_rate_kibs);
    free(e->rx_rate_kibs);
    free(e->mtu_interf);
    free(e->mac_address);
    free(e->ipv4_address);
    free(e->ipv6_address);
    free(e->gw_ipv4_address);
    free(e->gw_ipv6_address);
    free(e->rx_total_bytes);
    free(e->rx_total_packs);
    free(e->rx_total_drops);
    free(e->rx_total_errors);
    free(e->tx_total_bytes);
    free(e->tx_total_packs);
    free(e->tx_total_drops);
    free(e->tx_total_errors);
    free(e->device_link);
    free(e->duplex_mode);

    e->count = 0;
    e->active_interf_ct = 0;
    e->device_name = NULL;
    e->device_type = NULL;
    e->active_status = NULL;
    e->conn_name = NULL;
    e->tx_rate_kibs = NULL;
    e->rx_rate_kibs = NULL;
    e->mtu_interf = NULL;
    e->mac_address = NULL;
    e->ipv4_address = NULL;
    e->ipv6_address = NULL;
    e->gw_ipv4_address = NULL;
    e->gw_ipv6_address = NULL;
    e->rx_total_bytes = NULL;
    e->rx_total_packs = NULL;
    e->rx_total_drops = NULL;
    e->rx_total_errors = NULL;
    e->tx_total_bytes = NULL;
    e->tx_total_packs = NULL;
    e->tx_total_drops = NULL;
    e->tx_total_errors = NULL;
    e->device_link = NULL;
    e->duplex_mode = NULL;
    e->operstate_mode = NULL;
}

INTRF INTRF_get_data(INTRF *e)
{
    INTRF new_e = {0};

    pthread_mutex_lock(&e->mtx);
    new_e.count = e->count;
    new_e.active_interf_ct = e->active_interf_ct;

    if (e->count > 0)
    {
        new_e.device_name = calloc((size_t) e->count, sizeof(*new_e.device_name));
        new_e.device_type = calloc((size_t) e->count, sizeof(*new_e.device_type));
        new_e.active_status = calloc((size_t) e->count, 16);
        new_e.conn_name = calloc((size_t) e->count, sizeof(*new_e.conn_name));
        new_e.tx_rate_kibs = calloc((size_t) e->count, sizeof(*new_e.tx_rate_kibs));
        new_e.rx_rate_kibs = calloc((size_t) e->count, sizeof(*new_e.rx_rate_kibs));
        new_e.mtu_interf = calloc((size_t) e->count, sizeof(*new_e.mtu_interf));
        new_e.mac_address = calloc((size_t) e->count, sizeof(*new_e.mac_address));
        new_e.ipv4_address = calloc((size_t) e->count, sizeof(*new_e.ipv4_address));
        new_e.ipv6_address = calloc((size_t) e->count, sizeof(*new_e.ipv6_address));
        new_e.gw_ipv4_address = calloc((size_t) e->count, sizeof(*new_e.gw_ipv4_address));
        new_e.gw_ipv6_address = calloc((size_t) e->count, sizeof(*new_e.gw_ipv6_address));
        new_e.rx_total_bytes = calloc((size_t) e->count, sizeof(*new_e.rx_total_bytes));
        new_e.rx_total_packs = calloc((size_t) e->count, sizeof(*new_e.rx_total_packs));
        new_e.rx_total_drops = calloc((size_t) e->count, sizeof(*new_e.rx_total_drops));
        new_e.rx_total_errors = calloc((size_t) e->count, sizeof(*new_e.rx_total_errors));
        new_e.tx_total_bytes = calloc((size_t) e->count, sizeof(*new_e.tx_total_bytes));
        new_e.tx_total_packs = calloc((size_t) e->count, sizeof(*new_e.tx_total_packs));
        new_e.tx_total_drops = calloc((size_t) e->count, sizeof(*new_e.tx_total_drops));
        new_e.tx_total_errors = calloc((size_t) e->count, sizeof(*new_e.tx_total_errors));
        new_e.device_link = calloc((size_t) e->count, sizeof(*new_e.device_link));
        new_e.duplex_mode = calloc((size_t) e->count, sizeof(*new_e.duplex_mode));
        new_e.operstate_mode = calloc((size_t) e->count, sizeof(*new_e.operstate_mode));

        if (
            new_e.device_name == NULL || new_e.device_type == NULL ||
            new_e.active_status == NULL || new_e.conn_name == NULL ||
            new_e.tx_rate_kibs == NULL || new_e.rx_rate_kibs == NULL ||
            new_e.mtu_interf == NULL || new_e.mac_address == NULL ||
            new_e.ipv4_address == NULL || new_e.ipv6_address == NULL ||
            new_e.gw_ipv4_address == NULL || new_e.gw_ipv6_address == NULL ||
            new_e.rx_total_bytes == NULL || new_e.rx_total_packs == NULL ||
            new_e.rx_total_drops == NULL || new_e.rx_total_errors == NULL ||
            new_e.tx_total_bytes == NULL || new_e.tx_total_packs == NULL ||
            new_e.tx_total_drops == NULL || new_e.tx_total_errors == NULL ||
            new_e.device_link == NULL || new_e.duplex_mode == NULL ||
            new_e.operstate_mode == NULL
        )
        {
            pthread_mutex_unlock(&e->mtx);
            INTRF_free_copy(&new_e);
            return new_e;
        }

        for (int i = 0; i < e->count; ++i)
        {
            if (e->device_name != NULL && e->device_name[i] != NULL)
            {
                new_e.device_name[i] = strdup(e->device_name[i]);
                if (new_e.device_name[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    INTRF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->device_type != NULL && e->device_type[i] != NULL)
            {
                new_e.device_type[i] = strdup(e->device_type[i]);
                if (new_e.device_type[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    INTRF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->conn_name != NULL && e->conn_name[i] != NULL)
            {
                new_e.conn_name[i] = strdup(e->conn_name[i]);
                if (new_e.conn_name[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    INTRF_free_copy(&new_e);
                    return new_e;
                }
            }
            if (e->operstate_mode != NULL && e->operstate_mode[i] != NULL)
            {
                new_e.operstate_mode[i] = strdup(e->operstate_mode[i]);
                if (new_e.operstate_mode[i] == NULL)
                {
                    pthread_mutex_unlock(&e->mtx);
                    INTRF_free_copy(&new_e);
                    return new_e;
                }
            }
        }

        if (e->active_status != NULL) memcpy(new_e.active_status, e->active_status, (size_t) e->count * 16);
        if (e->tx_rate_kibs != NULL) memcpy(new_e.tx_rate_kibs, e->tx_rate_kibs, (size_t) e->count * sizeof(*new_e.tx_rate_kibs));
        if (e->rx_rate_kibs != NULL) memcpy(new_e.rx_rate_kibs, e->rx_rate_kibs, (size_t) e->count * sizeof(*new_e.rx_rate_kibs));
        if (e->mtu_interf != NULL) memcpy(new_e.mtu_interf, e->mtu_interf, (size_t) e->count * sizeof(*new_e.mtu_interf));
        if (e->mac_address != NULL) memcpy(new_e.mac_address, e->mac_address, (size_t) e->count * sizeof(*new_e.mac_address));
        if (e->ipv4_address != NULL) memcpy(new_e.ipv4_address, e->ipv4_address, (size_t) e->count * sizeof(*new_e.ipv4_address));
        if (e->ipv6_address != NULL) memcpy(new_e.ipv6_address, e->ipv6_address, (size_t) e->count * sizeof(*new_e.ipv6_address));
        if (e->gw_ipv4_address != NULL) memcpy(new_e.gw_ipv4_address, e->gw_ipv4_address, (size_t) e->count * sizeof(*new_e.gw_ipv4_address));
        if (e->gw_ipv6_address != NULL) memcpy(new_e.gw_ipv6_address, e->gw_ipv6_address, (size_t) e->count * sizeof(*new_e.gw_ipv6_address));
        if (e->rx_total_bytes != NULL) memcpy(new_e.rx_total_bytes, e->rx_total_bytes, (size_t) e->count * sizeof(*new_e.rx_total_bytes));
        if (e->rx_total_packs != NULL) memcpy(new_e.rx_total_packs, e->rx_total_packs, (size_t) e->count * sizeof(*new_e.rx_total_packs));
        if (e->rx_total_drops != NULL) memcpy(new_e.rx_total_drops, e->rx_total_drops, (size_t) e->count * sizeof(*new_e.rx_total_drops));
        if (e->rx_total_errors != NULL) memcpy(new_e.rx_total_errors, e->rx_total_errors, (size_t) e->count * sizeof(*new_e.rx_total_errors));
        if (e->tx_total_bytes != NULL) memcpy(new_e.tx_total_bytes, e->tx_total_bytes, (size_t) e->count * sizeof(*new_e.tx_total_bytes));
        if (e->tx_total_packs != NULL) memcpy(new_e.tx_total_packs, e->tx_total_packs, (size_t) e->count * sizeof(*new_e.tx_total_packs));
        if (e->tx_total_drops != NULL) memcpy(new_e.tx_total_drops, e->tx_total_drops, (size_t) e->count * sizeof(*new_e.tx_total_drops));
        if (e->tx_total_errors != NULL) memcpy(new_e.tx_total_errors, e->tx_total_errors, (size_t) e->count * sizeof(*new_e.tx_total_errors));
        if (e->device_link != NULL) memcpy(new_e.device_link, e->device_link, (size_t) e->count * sizeof(*new_e.device_link));
        if (e->duplex_mode != NULL) memcpy(new_e.duplex_mode, e->duplex_mode, (size_t) e->count * sizeof(*new_e.duplex_mode));
    }

    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
