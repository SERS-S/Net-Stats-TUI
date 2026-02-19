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
    char new_con[64];

    format_rate(rx_rate_buffer,sizeof(rx_rate_buffer),snapshot.rx_rate_kibs);
    format_rate(tx_rate_buffer,sizeof(tx_rate_buffer),snapshot.tx_rate_kibs);
    format_rate(rx_total_buffer,sizeof(rx_total_buffer),snapshot.rx_total_kibs);
    format_rate(tx_total_buffer,sizeof(tx_total_buffer),snapshot.tx_total_kibs);

    snprintf(
        rx_rate, sizeof(rx_rate), 
        "RX rate: %s ( %.1f kpps)", 
        rx_rate_buffer,
        snapshot.rx_rate_kpps
    );

    snprintf(
        tx_rate, sizeof(tx_rate), 
        "TX rate: %s ( %.1f kpps)", 
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

    mvprintw( y, x_starter, "%s", rx_rate); 
    getyx(stdscr, y, x);
    mvprintw( y, x + 2, "%s", tx_rate);
    y += 1;
    mvprintw( y, x_starter, "%s" ,rx_total);
    mvprintw( y, x + 2, "%s", tx_total);
    y += 1;
    mvprintw( y, x_starter, "%s", errors);
    mvprintw( y, x + 2, "Drops:");
    y += 2;
    mvprintw( y, x_starter, "Connections:");
    y += 1;
    mvprintw( y, x_starter, "New conss/s:");
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
