#ifndef OVERALL_DATA
#define OVERALL_DATA

#include <pthread.h>

typedef struct overall_data
{
    pthread_mutex_t mtx;
    float rx_rate_kibs;
    float rx_rate_kpps;
    float tx_rate_kibs;
    float tx_rate_kpps;
    float rx_total_kibs;
    float tx_total_kibs;
    int errors_rx;
    int errors_tx;
    int drops_rx;
    int drops_tx;
    int conn_estab;
    int conn_lst;
    int conn_tmw;
    int conn_systn;
    int conn_clsw;
    int rx_sparkline[15];
    int tx_sparkline[15];
    int retr_pkg_sparkline[15];
} OVRLL;

void OVRLL_init(OVRLL *e);
void OVRLL_destroy(OVRLL *e);
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
);
OVRLL OVRLL_get_data(OVRLL *e);
void draw_ovrll();

#endif
