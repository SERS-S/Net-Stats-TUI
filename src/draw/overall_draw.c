#include "draw/overall_draw.h"
#include "data/overall_data.h"
#include <curses.h>
#include <string.h>
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static void draw_sparkline(int y, int x, const float *data, int count);
static void print_data(const char*, int);

void draw_ovrll(void *ptr, int y, int x){
    OVRLL snapshot = OVRLL_get_data(ptr);
    int x_starter = x;
    wmove(stdscr,y, x_starter);

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
        rx_total, sizeof(rx_total), 
        "RX total: %s", 
        rx_total_buffer
    );

    snprintf(
        tx_total, sizeof(tx_total), 
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

    print_data(rx_rate, 1); 
    print_data(tx_rate, 2);
    print_data(rx_total, 1);
    print_data(tx_total, 2);
    print_data(errors, 1);
    print_data(drops, 2);
    print_data(connections, 1);
    print_data("RX", 1);
    getyx(stdscr, y, x_starter);
    draw_sparkline(y, x_starter + 3, snapshot.rx_sparkline, 8);
    print_data("TX", 2);
    getyx(stdscr, y, x_starter);
    draw_sparkline(y, x_starter + 3, snapshot.tx_sparkline, 8);
    print_data("Retrans/s", 1);
    getyx(stdscr, y, x_starter);
    draw_sparkline(y, x_starter + 3, snapshot.retr_pkg_sparkline, 8);
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



static void draw_sparkline(int y, int x, const float *data, int count) {
    if (!data || count <= 0) return;
    int h = getmaxy(stdscr);
    int current_y;
    int current_x;
    getyx(stdscr, current_y, current_x);
    if(current_y >= h -4){
        return;
    }
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

static void print_data(const char* data, int type){
    int h = getmaxy(stdscr);
    int l = getmaxx(stdscr);
    int rows = 5;
    int skip_rows = MAX((h - 4 - rows)/10, 1);
    int skip_table = l - 50;
    int y;
    int x;
    int len = strlen(data);
    getyx(stdscr, y, x);
    if(type ==1){
        y += skip_rows;
        x = 2;
    }else{
        x = 33 + 10;
    }
    if(y >= h - 4||  len >= l - x){
        wmove(stdscr,y + skip_rows, 2);
        return;
    };
    mvprintw(y, x, "%s", data);
}