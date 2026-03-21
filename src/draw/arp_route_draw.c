#include "draw/arp_route_draw.h"
#include "data/arp_route_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

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

static void format_last_seen(double seconds, char *buffer, size_t buffer_sz)
{
    if (seconds < 0.0) {
        snprintf(buffer, buffer_sz, "-");
    } else if (seconds < 1.0) {
        snprintf(buffer, buffer_sz, "%.1fs", seconds);
    } else {
        snprintf(buffer, buffer_sz, "%.0fs", seconds);
    }
}

static void fit_cell_text(const char *src, char *dst, size_t dst_sz)
{
    if (dst_sz == 0) return;

    if (src == NULL || *src == '\0') {
        snprintf(dst, dst_sz, "-");
        return;
    }

    size_t max_len = dst_sz - 1;
    size_t src_len = strlen(src);

    if (src_len <= max_len) {
        snprintf(dst, dst_sz, "%s", src);
        return;
    }

    if (max_len <= 3) {
        snprintf(dst, dst_sz, "%.*s", (int) max_len, src);
        return;
    }

    snprintf(dst, dst_sz, "%.*s...", (int) (max_len - 3), src);
}

void draw_arprt(void *ptr, int y, int x) {
    if (!ptr) return;
    
    ARPRT snapshot = ARPRT_get_data((ARPRT *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y || x >= max_x) {
        ARPRT_free_copy(&snapshot);
        return;
    }

    int route_count = 0;
    int neighbor_count = 0;
    for (int i = 0; i < snapshot.count; ++i) {
        uint8_t type = snapshot.row_type ? snapshot.row_type[i] : 0;
        if (type == 1) ++route_count;
        else if (type == 2) ++neighbor_count;
    }

    int panel_left = x;
    int panel_width = max_x - x - 3;
    int current_y = y + 2;
    int routes_height = (route_count > 0 ? route_count : 1) + 3;

    draw_box_panel(current_y, panel_left, panel_width, routes_height, " ROUTES ");
    mvprintw(current_y + 1, panel_left + 2, "Table main");

    int row_y = current_y + 2;
    int routes_shown = 0;
    for (int i = 0; i < snapshot.count && row_y < current_y + routes_height - 1; i++) {
        uint8_t type = snapshot.row_type ? snapshot.row_type[i] : 0;

        if (type != 1) continue;

        if (snapshot.route_is_default && snapshot.route_is_default[i]) {
            mvprintw(row_y, panel_left + 2, "default via %s dev %s metric %u",
                     snapshot.route_gateway && snapshot.route_gateway[i] ? snapshot.route_gateway[i] : "?",
                     snapshot.route_dev && snapshot.route_dev[i] ? snapshot.route_dev[i] : "?",
                     snapshot.route_metric ? snapshot.route_metric[i] : 0U);
        } else {
            mvprintw(row_y, panel_left + 2, "%s/%u dev %s proto kernel scope link",
                     snapshot.route_dst && snapshot.route_dst[i] ? snapshot.route_dst[i] : "?",
                     snapshot.route_prefix_len ? snapshot.route_prefix_len[i] : 0U,
                     snapshot.route_dev && snapshot.route_dev[i] ? snapshot.route_dev[i] : "?");
        }
        row_y++;
        routes_shown++;
    }

    if (routes_shown == 0) {
        mvprintw(row_y, panel_left + 2, "No routes");
    }

    current_y += routes_height + 2;
    int neighbors_height = (neighbor_count > 0 ? neighbor_count : 1) + 4;
    draw_box_panel(current_y, panel_left, panel_width, neighbors_height, " NEIGHBORS (ARP/NDP) ");

    attron(A_BOLD);
    mvprintw(current_y + 1, panel_left + 2, "%-34s %-18s %-8s %-12s %-10s",
             "IP", "MAC", "Dev", "State", "Last seen");
    attroff(A_BOLD);

    int neighbors_y = current_y + 2;
    int neighbors_shown = 0;
    for (int i = 0; i < snapshot.count && neighbors_y < current_y + neighbors_height - 1; i++) {
        uint8_t type = snapshot.row_type ? snapshot.row_type[i] : 0;
        if (type != 2) continue;

        char mac_str[32] = "-";
        char dev_str[9];
        char state_str[13];
        char last_seen_str[16];

        fit_cell_text(
            snapshot.neighbor_mac && snapshot.neighbor_mac[i] ? snapshot.neighbor_mac[i] : "?",
            mac_str,
            19
        );

        fit_cell_text(
            snapshot.neighbor_dev && snapshot.neighbor_dev[i] ? snapshot.neighbor_dev[i] : "?",
            dev_str,
            sizeof(dev_str)
        );

        fit_cell_text(
            snapshot.neighbor_state && snapshot.neighbor_state[i] ? snapshot.neighbor_state[i] : "?",
            state_str,
            sizeof(state_str)
        );

        format_last_seen(
            snapshot.neighbor_last_seen_sec ? snapshot.neighbor_last_seen_sec[i] : -1.0,
            last_seen_str,
            sizeof(last_seen_str)
        );

        mvprintw(neighbors_y, panel_left + 2, "%-34s %-18s %-8s %-12s %-10s",
                 snapshot.neighbor_ip && snapshot.neighbor_ip[i] ? snapshot.neighbor_ip[i] : "?",
                 mac_str,
                 dev_str,
                 state_str,
                 last_seen_str);

        neighbors_y++;
        neighbors_shown++;
    }

    if (neighbors_shown == 0) {
        mvprintw(neighbors_y, panel_left + 2, "No neighbors");
    }

    ARPRT_free_copy(&snapshot);
}
