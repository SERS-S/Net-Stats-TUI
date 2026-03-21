#include "draw/interfaces_draw.h"
#include "data/interfaces_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>


extern int last_key;

static int focused_row = 0;
static int sort_ascending = 1; 
static int sorted = 0;
static int *sort_indices = NULL;
static int sort_indices_size = 0;  
static INTRF *global_intrf = NULL;  

static const char* format_bytes(int bytes);
static const char* format_packets(int packets);
static void format_rate(char *buffer, size_t buffer_sz, float kib_speed);
static void draw_box_panel(int top, int left, int width, int height, const char *title);

static int compare_asc(const void *a, const void *b) {
    int idx_a = *(int *)a;
    int idx_b = *(int *)b;
    
    if (!global_intrf || !global_intrf->device_name) return 0;
    
    const char *name_a = global_intrf->device_name[idx_a] ? global_intrf->device_name[idx_a] : "";
    const char *name_b = global_intrf->device_name[idx_b] ? global_intrf->device_name[idx_b] : "";
    
    return strcmp(name_a, name_b);
}


static int compare_desc(const void *a, const void *b) {
    return -compare_asc(a, b);
}

static void init_sort_indices(INTRF *intrf) {
    if (!intrf || intrf->count <= 0) return;
    
    if (sort_indices_size != intrf->count) {
        if (sort_indices) {
            free(sort_indices);
        }
        sort_indices = (int *)malloc(intrf->count * sizeof(int));
        sort_indices_size = intrf->count;
    }
    
    for (int i = 0; i < intrf->count; i++) {
        sort_indices[i] = i;
    }
    
    sorted = 0;
}

static void sort_interfaces(INTRF *intrf) {
    if (!intrf || intrf->count <= 0) return;
    
    init_sort_indices(intrf);
    

    if (sort_ascending) {
        qsort(sort_indices, intrf->count, sizeof(int), compare_asc);
    } else {
        qsort(sort_indices, intrf->count, sizeof(int), compare_desc);
    }
    
    sorted = 1;
}

static void draw_interfaces_table(INTRF *intrf, int start_y, int start_x) {
    if (!intrf || intrf->count <= 0) {
        if (start_y < getmaxy(stdscr)) {
            mvprintw(start_y, start_x, "Нет данных об интерфейсах");
        }
        return;
    }
    
    global_intrf = intrf;
    
    if (sort_indices_size != intrf->count) {
        init_sort_indices(intrf);
    }
    
    if (last_key == 's' || last_key == 'S') {
        if (sorted) {
            sort_ascending = !sort_ascending;
        }
        sort_interfaces(intrf);
        focused_row = 0;
        last_key = 0;
    } else if (last_key == KEY_UP && focused_row > 0) {
        focused_row--;
        last_key = 0;
    } else if (last_key == KEY_DOWN && focused_row < intrf->count - 1) {
        focused_row++;
        last_key = 0;
    }
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (start_y >= max_y || start_x >= max_x) {
        return;
    }
    
    int table_y = start_y;
    int frame_left = start_x;
    int frame_width = max_x - start_x - 3;
    int details_reserved = 9;
    int row_start = start_y + 5;
    int max_rows = max_y - details_reserved - row_start;
    if (max_rows < 1) max_rows = 1;
    int visible_rows = intrf->count < max_rows ? intrf->count : max_rows;
    int frame_top = start_y + 2;
    int frame_height = visible_rows + 4;
    int frame_bottom = frame_top + frame_height - 1;

    if (frame_width < 20) return;

    attron(A_BOLD);
    mvprintw(table_y, start_x, "INTERFACES");
    attroff(A_BOLD);

    mvprintw(
        table_y + 1,
        start_x,
        "Active: %d/%d   Sort: name %s   (s: sort)",
        intrf->active_interf_ct,
        intrf->count,
        sort_ascending ? "v" : "^"
    );

    draw_box_panel(frame_top, frame_left, frame_width, frame_height, NULL);
    mvhline(frame_top + 2, frame_left + 1, ACS_HLINE, frame_width - 2);

    attron(A_BOLD);
    mvprintw(
        frame_top + 1,
        frame_left + 2,
        "%-6s | %-10s | %-7s | %-12s | %-11s | %-11s | %-5s | %-17s",
        "Dev", "Type", "State", "Conn", "TX (rate)", "RX (rate)", "MTU", "MAC"
    );
    attroff(A_BOLD);

    table_y = row_start;

    for (int display_pos = 0; display_pos < visible_rows; display_pos++) {
        if (table_y >= frame_bottom) break;
    
        int i = sorted ? sort_indices[display_pos] : display_pos;
        char *status_slot = intrf->active_status ? intrf->active_status + (i * 16) : NULL;
        const char *status = (status_slot && *status_slot) ? status_slot : "-";
        int active = strcmp(status, "UP") == 0;

        const char *dev_str;
        char tx_str[16];
        char rx_str[16];
        char mtu_str[8];
        char mac_str[18];
        dev_str = intrf->device_name && intrf->device_name[i] ? intrf->device_name[i] : "-";

        if (intrf->tx_rate_kibs) format_rate(tx_str, sizeof(tx_str), intrf->tx_rate_kibs[i]);
        else strcpy(tx_str, "-");

        if (intrf->rx_rate_kibs) format_rate(rx_str, sizeof(rx_str), intrf->rx_rate_kibs[i]);
        else strcpy(rx_str, "-");

        if (intrf->mtu_interf) snprintf(mtu_str, sizeof(mtu_str), "%d", intrf->mtu_interf[i]);
        else strcpy(mtu_str, "-");

        if (intrf->mac_address && intrf->mac_address[i]) {
            snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                    intrf->mac_address[i][0],
                    intrf->mac_address[i][1],
                    intrf->mac_address[i][2],
                    intrf->mac_address[i][3],
                    intrf->mac_address[i][4],
                    intrf->mac_address[i][5]);
        } else {
            strcpy(mac_str, "-");
        }

        if (display_pos == focused_row) attron(A_REVERSE);
        if (!active) attron(COLOR_PAIR(1));
        mvprintw(
            table_y,
            frame_left + 2,
            "%-6s | %-10s | %-7s | %-12s | %-11s | %-11s | %-5s | %-17s",
            dev_str,
            intrf->device_type && intrf->device_type[i] ? intrf->device_type[i] : "-",
            status,
            intrf->conn_name && intrf->conn_name[i] ? intrf->conn_name[i] : "-",
            tx_str,
            rx_str,
            mtu_str,
            mac_str
        );
        if (!active) attroff(COLOR_PAIR(1));
        if (display_pos == focused_row) attroff(A_REVERSE);
        
        table_y += 1;
    }
}

int draw_top(INTRF *intrf) {
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    int y_top = 3;
    
    if (y_top >= max_y) return y_top;
    
    draw_interfaces_table(intrf, y_top, 2);
    return y_top + 2 + ((intrf && intrf->count > 0) ? ((intrf->count < (max_y - 18) ? intrf->count : (max_y - 18)) + 4) : 4) - 1;
}

void draw_intrf(void *ptr, int y, int x){
    INTRF snapshot = INTRF_get_data((INTRF *)ptr);
    INTRF *intrf = &snapshot;
    int h = getmaxy(stdscr);
    int l = getmaxx(stdscr);
    int top_end = draw_top(intrf);
    draw_bottom(top_end, intrf);
    global_intrf = NULL;
    INTRF_free_copy(&snapshot);
}

void draw_bottom(int top_end, INTRF *intrf) {
    if (!intrf || focused_row < 0 || focused_row >= intrf->count) {
        return;
    }
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    int y_start = top_end + 2;
    int y_status = max_y - 2;
    int y_bottom_limit = y_status - 1;
    
    int i;
    if (sorted) {
        if (focused_row >= 0 && focused_row < intrf->count) {
            i = sort_indices[focused_row];
        } else {
            i = focused_row;
        }
    } else {
        i = focused_row;
    }
    
    if (i < 0 || i >= intrf->count) {
        return;
    }
    
    if (y_start >= y_bottom_limit) {
        return;
    }
    
    int panel_height = 5;
    int panel_width = max_x - 6;
    if (y_start + panel_height >= y_bottom_limit) return;

    for (int row = y_start; row < y_start + panel_height; row++) {
        mvhline(row, 2, ' ', max_x - 4);
    }

    char details_title[96];
    snprintf(
        details_title,
        sizeof(details_title),
        " IFACE DETAILS: %s ",
        intrf->device_name && intrf->device_name[i] ? intrf->device_name[i] : "?"
    );

    draw_box_panel(
        y_start,
        2,
        panel_width,
        panel_height,
        details_title
    );

    int y = y_start + 1;

    if (intrf->ipv4_address && intrf->ipv4_address[i]) {
        mvprintw(y, 4, "IPv4: %d.%d.%d.%d/24",
                intrf->ipv4_address[i][0],
                intrf->ipv4_address[i][1],
                intrf->ipv4_address[i][2],
                intrf->ipv4_address[i][3]);
    } else {
        mvprintw(y, 4, "IPv4: -");
    }
    
    if (intrf->ipv6_address && intrf->ipv6_address[i]) {
        mvprintw(y, 28, "IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x::/64",
                intrf->ipv6_address[i][0], intrf->ipv6_address[i][1],
                intrf->ipv6_address[i][2], intrf->ipv6_address[i][3],
                intrf->ipv6_address[i][4], intrf->ipv6_address[i][5],
                intrf->ipv6_address[i][6], intrf->ipv6_address[i][7]);
    } else {
        mvprintw(y, 30, "IPv6: -");
    }
    
    if (intrf->gw_ipv4_address && intrf->gw_ipv4_address[i]) {
        mvprintw(y, 62, "GW: %d.%d.%d.%d",
                intrf->gw_ipv4_address[i][0],
                intrf->gw_ipv4_address[i][1],
                intrf->gw_ipv4_address[i][2],
                intrf->gw_ipv4_address[i][3]);
    } else {
        mvprintw(y, 62, "GW: -");
    }

    y++;
    if (y >= y_bottom_limit) return;

    mvprintw(y, 4, "RX: bytes %s  pkts %s  drop %d  err %d",
             format_bytes(intrf->rx_total_bytes ? intrf->rx_total_bytes[i] : 0),
             format_packets(intrf->rx_total_packs ? intrf->rx_total_packs[i] : 0),
             intrf->rx_total_drops ? intrf->rx_total_drops[i] : 0,
             intrf->rx_total_errors ? intrf->rx_total_errors[i] : 0);

    mvprintw(y, 52, "TX: bytes %s  pkts %s  err %d",
             format_bytes(intrf->tx_total_bytes ? intrf->tx_total_bytes[i] : 0),
             format_packets(intrf->tx_total_packs ? intrf->tx_total_packs[i] : 0),
             intrf->tx_total_errors ? intrf->tx_total_errors[i] : 0);

    y++;
    if (y >= y_bottom_limit) return;

    char link_str[32] = "-";
    if (intrf->device_link) {
        int speed_mbps = intrf->device_link[i];
        if (speed_mbps >= 1000) {
            snprintf(link_str, sizeof(link_str), "%dG", speed_mbps / 1000);
        } else {
            snprintf(link_str, sizeof(link_str), "%dM", speed_mbps);
        }
    }
    
    char duplex_str[16] = "-";
    if (intrf->duplex_mode && intrf->duplex_mode[i]) {
        strncpy(duplex_str, intrf->duplex_mode[i], sizeof(duplex_str) - 1);
    }
    
    char operstate_str[16] = "-";
    if (intrf->operstate_mode && intrf->operstate_mode[i]) {
        strncpy(operstate_str, intrf->operstate_mode[i], sizeof(operstate_str) - 1);
    }
    
    mvprintw(y, 4, "Link: %s  %s-duplex  operstate:%s",
             link_str,
             duplex_str,
             operstate_str);
}

static const char* format_bytes(int bytes) {
    static char buffer[32];
    if (bytes < 1024) {
        snprintf(buffer, sizeof(buffer), "%dB", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%.1fKiB", bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buffer, sizeof(buffer), "%.1fMiB", bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, sizeof(buffer), "%.1fGiB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
    return buffer;
}

static const char* format_packets(int packets) {
    static char buffer[32];
    if (packets < 1000) {
        snprintf(buffer, sizeof(buffer), "%d", packets);
    } else if (packets < 1000000) {
        snprintf(buffer, sizeof(buffer), "%.1fK", packets / 1000.0);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1fM", packets / 1000000.0);
    }
    return buffer;
}

static void format_rate(char *buffer, size_t buffer_sz, float kib_speed) {
    if (kib_speed / 1024.0f > 1.0f) {
        snprintf(buffer, buffer_sz, "%.1fMiB/s", kib_speed / 1024.0f);
    } else {
        snprintf(buffer, buffer_sz, "%.1f", kib_speed);
    }
}

static void draw_box_panel(int top, int left, int width, int height, const char *title)
{
    if (width < 4 || height < 3) return;

    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left + width - 1, ACS_URCORNER);
    mvaddch(top + height - 1, left, ACS_LLCORNER);
    mvaddch(top + height - 1, left + width - 1, ACS_LRCORNER);

    mvhline(top, left + 1, ACS_HLINE, width - 2);
    mvhline(top + height - 1, left + 1, ACS_HLINE, width - 2);

    for (int row = top + 1; row < top + height - 1; ++row)
    {
        mvaddch(row, left, ACS_VLINE);
        mvaddch(row, left + width - 1, ACS_VLINE);
    }

    if (title != NULL)
    {
        mvprintw(top, left + 2, "%s", title);
    }
}

void line_to_end(void) {
    int y, x;
    getyx(stdscr, y, x); 
    
    int max_x = getmaxx(stdscr) - 1;  
    int chars_left = max_x - x;   
    
    if (chars_left > 0) {
        hline(ACS_HLINE, chars_left);
    }
}
