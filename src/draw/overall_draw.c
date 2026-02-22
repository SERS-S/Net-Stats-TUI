#include "draw/overall_draw.h"
#include "data/overall_data.h"
#include <curses.h>
#define MAX(a,b) ((a) > (b) ? (a) : (b))


void draw_ovrll(void *ptr, int y, int x){
    OVRLL snapshot = OVRLL_get_data(ptr);
    int x_starter = x;

    char rx_rate[64];
    char rx_rate_buffer[16];
    char rx_total[32];
    char rx_total_buffer[16];

    char tx_rate[64];
    char tx_rate_buffer[16];
    char tx_total[32];
    char tx_total_buffer[16];

    char errors[64];
    char drops[64];
    char connections[128];

    format_rate(rx_rate_buffer,sizeof(rx_rate_buffer),snapshot.rx_rate_kibs);
    format_rate(tx_rate_buffer,sizeof(tx_rate_buffer),snapshot.tx_rate_kibs);
    format_rate(rx_total_buffer,sizeof(rx_total_buffer),snapshot.rx_total_kibs);
    format_rate(tx_total_buffer,sizeof(tx_total_buffer),snapshot.tx_total_kibs);

    snprintf(
        rx_rate, sizeof(rx_rate), 
        "RX rate: %s ( %.3f kpps)", 
        rx_rate_buffer,
        snapshot.rx_rate_kpps
    );

    snprintf(
        tx_rate, sizeof(tx_rate), 
        "TX rate: %s ( %.3f kpps)", 
        tx_rate_buffer,
        snapshot.tx_rate_kpps
    );

    snprintf(
        rx_total, sizeof(rx_rate), 
        "RX total: %s", 
        rx_total_buffer
    );

    snprintf(
        tx_total, sizeof(tx_rate), 
        "TX total: %s", 
        tx_total_buffer
    );

    snprintf(
        errors, sizeof(errors),
        "Errors: rx_err %d  tx_err %d",
        snapshot.errors_rx,
        snapshot.errors_tx
    );

    snprintf(
        drops, sizeof(drops),
        "Drops: rx_drop %d  tx_drop %d",
        snapshot.drops_rx,
        snapshot.drops_tx
    );

    snprintf(
        connections, sizeof(connections),
        "Connections: ESTAB %d LISTEN %d TIME_WAIT %d SYN-SENT %d CLOSE_WAIT %d",
        snapshot.conn_estab,
        snapshot.conn_lst,
        snapshot.conn_tmw,
        snapshot.conn_systn,
        snapshot.conn_clsw
    );

    mvprintw( y, x_starter, "%s", rx_rate); 
    getyx(stdscr, y, x);
    mvprintw( y, x + 2, "%s", tx_rate);
    y += 1;
    mvprintw( y, x_starter, "%s" ,rx_total);
    mvprintw( y, x + 2, "%s", tx_total);
    y += 1;
    mvprintw( y, x_starter, "%s", errors);
    mvprintw( y, x + 2,  "%s", drops);
    y += 2;
    mvprintw( y, x_starter, "%s", connections);
    y += 5;
    mvprintw( y, x_starter, "RX");
    draw_sparkline(y, x_starter + 3, snapshot.rx_sparkline, 8);
    getyx(stdscr, y, x);
    mvprintw( y, x + 2, "TX");
    draw_sparkline(y, x + 5, snapshot.tx_sparkline, 8);
    getyx(stdscr, y, x);
    mvprintw( y, x + 2, "Retrans/s");
    getyx(stdscr, y, x);
    draw_sparkline(y, x + 1, snapshot.retr_pkg_sparkline, 8);

};

void format_rate(char *buffer, int buffer_size, double kib_speed) {
    if (kib_speed / (1024.0 * 1024.0) > 1) { 
        snprintf(buffer, buffer_size, "%.1f GiB/s", kib_speed / (1024.0 * 1024.0));
    } else if (kib_speed / 1024.0 > 1) {      
        snprintf(buffer, buffer_size, "%.1f MiB/s", kib_speed / 1024.0);
    } else {                                    
        snprintf(buffer, buffer_size, "%.1f KiB/s", kib_speed);
    }
}



void draw_sparkline(int y, int x, const float *data, int count) {
    if (!data || count <= 0) return;

    mvprintw(y, x++, "[");

    static const char *bars[7] = {"▁","▂","▃","▄","▅","▆","▇"};
        const int MAX_LEVEL = 6;


    float max_val = data[0];
    for (int i = 1; i < count; i++)
        if (data[i] > max_val)
            max_val = data[i];

    if (max_val <= 0.0f) max_val = 1.0f;


    for (int i = 0; i < count ; i++) {
        int idx = (int)((data[i] / max_val) * MAX_LEVEL + 0.5f);

        if (idx < 0) idx = 0;
        if (idx > MAX_LEVEL) idx = MAX_LEVEL;


        mvprintw(y, x + 2 * i, "%s", bars[idx]);
    }
    getyx(stdscr, y, x);
    mvprintw(y, ++x, "]");
}