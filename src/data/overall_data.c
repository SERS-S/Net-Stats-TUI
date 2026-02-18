#include "data/overall_data.h"
#include <pthread.h>
#include <string.h>

void OVRLL_init(OVRLL *e)
{
    pthread_mutex_init(&e->mtx, NULL);
    e->rx_rate_kibs = 0;
    e->rx_rate_kpps = 0;
    e->tx_rate_kibs = 0;
    e->tx_rate_kpps = 0;
    e->rx_total_kibs = 0;
    e->tx_total_kibs = 0;
    e->errors_rx = 0;
    e->errors_tx = 0;
    e->drops_rx = 0;
    e->drops_tx = 0;
    e->conn_estab = 0;
    e->conn_lst = 0;
    e->conn_tmw = 0;
    e->conn_systn = 0;
    e->conn_clsw = 0;
    memset(e->rx_sparkline, 0, 15 * sizeof(int));
    memset(e->tx_sparkline, 0, 15 * sizeof(int));
    memset(e->retr_pkg_sparkline, 0, 15 * sizeof(int));
}

void OVRLL_destroy(OVRLL *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void OVRLL_update_data(
    OVRLL *e,
    float new_rx_rate_kibs,
    float new_rx_rate_kpps,
    float new_tx_rate_kibs,
    float new_tx_rate_kpps,
    float new_rx_total_kibs,
    float new_tx_total_kibs,
    int new_errors_rx,
    int new_errors_tx,
    int new_drops_rx,
    int new_drops_tx,
    int new_conn_estab,
    int new_conn_lst,
    int new_conn_tmw,
    int new_conn_systn,
    int new_conn_clsw,
    int new_retr_pkg
)
{
    pthread_mutex_lock(&e->mtx);
    e->rx_rate_kibs = new_rx_rate_kibs;
    e->rx_rate_kpps = new_rx_rate_kpps;
    e->tx_rate_kibs = new_tx_rate_kibs;
    e->tx_rate_kpps = new_tx_rate_kpps;
    e->rx_total_kibs = new_rx_total_kibs;
    e->tx_total_kibs = new_tx_total_kibs;
    e->errors_rx = new_errors_rx;
    e->errors_tx = new_errors_tx;
    e->drops_rx = new_drops_rx;
    e->drops_tx = new_drops_tx;
    e->conn_estab = new_conn_estab;
    e->conn_lst = new_conn_lst;
    e->conn_tmw = new_conn_tmw;
    e->conn_systn = new_conn_systn;
    e->conn_clsw = new_conn_clsw;

    for (int i = 1; i < 15; ++i) e->rx_sparkline[i - 1] = e->rx_sparkline[i];
    e->rx_sparkline[14] = new_rx_rate_kpps;

    for (int i = 1; i < 15; ++i) e->tx_sparkline[i - 1] = e->tx_sparkline[i];
    e->tx_sparkline[14] = new_tx_rate_kpps;

    for (int i = 1; i < 15; ++i) e->retr_pkg_sparkline[i - 1] = e->retr_pkg_sparkline[i];
    e->retr_pkg_sparkline[14] = new_retr_pkg;

    pthread_mutex_unlock(&e->mtx);
}

OVRLL OVRLL_get_data(OVRLL *e)
{
    OVRLL new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
