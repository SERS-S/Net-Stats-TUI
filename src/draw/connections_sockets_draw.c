#include "draw/connections_sockets_draw.h"
#include "data/connections_sockets_data.h"
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

static const char* filter_state(const char *state) {
    if (!state) return "?";
    if (strcmp(state, "LISTEN") == 0) return "LISTEN";
    if (strcmp(state, "ESTAB") == 0) return "ESTAB";
    if (strcmp(state, "SYN_SENT") == 0) return "SYN-SENT";
    if (strcmp(state, "SYN_RECV") == 0) return "SYN-RECV";
    if (strcmp(state, "FIN_WAIT1") == 0) return "FIN-WAIT1";
    if (strcmp(state, "FIN_WAIT2") == 0) return "FIN-WAIT2";
    if (strcmp(state, "TIME_WAIT") == 0) return "TIME-WAIT";
    if (strcmp(state, "CLOSE") == 0) return "CLOSE";
    if (strcmp(state, "CLOSE_WAIT") == 0) return "CLOSE-WAIT";
    if (strcmp(state, "LAST_ACK") == 0) return "LAST-ACK";
    if (strcmp(state, "CLOSING") == 0) return "CLOSING";
    return state;
}

void draw_consock(void *ptr, int y, int x) {
    if (!ptr) {
        mvprintw(y, x, "SOCKETS: нет данных");
        return;
    }
    
    CONSOCK snapshot = CONSOCK_get_data((CONSOCK *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y - 3 || x >= max_x) {
        CONSOCK_free_copy(&snapshot);
        return;
    }

    int current_y = y + 2;
    int panel_left = x;
    int panel_width = max_x - x - 3;
    int max_rows = max_y - current_y - 8;
    if (max_rows < 1) max_rows = 1;
    int rows_to_show = (snapshot.count < max_rows) ? snapshot.count : max_rows;
    int panel_height = rows_to_show + 4;
    int col_state = panel_left + 2;
    int col_recv = panel_left + 16;
    int col_send = panel_left + 25;
    int col_local = panel_left + 34;
    int col_peer = panel_left + 60;
    int col_proc = panel_left + 86;

    draw_box_panel(current_y, panel_left, panel_width, panel_height, " SOCKETS ");
    mvhline(current_y + 2, panel_left + 1, ACS_HLINE, panel_width - 2);
    for (int row = current_y + 1; row < current_y + panel_height - 1; ++row) {
        mvaddch(row, col_recv - 2, ACS_VLINE);
        mvaddch(row, col_send - 2, ACS_VLINE);
        mvaddch(row, col_local - 2, ACS_VLINE);
        mvaddch(row, col_peer - 2, ACS_VLINE);
        mvaddch(row, col_proc - 2, ACS_VLINE);
    }

    attron(A_BOLD);
    mvprintw(current_y + 1, col_state, "%-10s", "State");
    mvprintw(current_y + 1, col_recv,  "%-7s", "Recv-Q");
    mvprintw(current_y + 1, col_send,  "%-7s", "Send-Q");
    mvprintw(current_y + 1, col_local, "%-24s", "Local Address:Port");
    mvprintw(current_y + 1, col_peer,  "%-24s", "Peer Address:Port");
    mvprintw(current_y + 1, col_proc,  "%-16s", "Proc/Info");
    attroff(A_BOLD);

    current_y += 3;
    
    for (int i = 0; i < rows_to_show && current_y < max_y - 4; i++) {
        char state_buf[16];
        char local_buf[32];
        char peer_buf[32];
        char proc_buf[20];
        const char *state = snapshot.state && snapshot.state[i] ? snapshot.state[i] : "?";
        snprintf(state_buf, sizeof(state_buf), "%s", filter_state(state));

        if (snapshot.local_ip && snapshot.local_ip[i]) {
            if (snapshot.local_port) {
                snprintf(local_buf, sizeof(local_buf), "%s:%u",
                         snapshot.local_ip[i], snapshot.local_port[i]);
            } else {
                snprintf(local_buf, sizeof(local_buf), "%s:*", snapshot.local_ip[i]);
            }
        } else {
            snprintf(local_buf, sizeof(local_buf), "*:*");
        }

        if (snapshot.peer_ip && snapshot.peer_ip[i]) {
            if (snapshot.peer_port) {
                snprintf(peer_buf, sizeof(peer_buf), "%s:%u",
                         snapshot.peer_ip[i], snapshot.peer_port[i]);
            } else {
                snprintf(peer_buf, sizeof(peer_buf), "%s:*", snapshot.peer_ip[i]);
            }
        } else {
            snprintf(peer_buf, sizeof(peer_buf), "*:*");
        }

        if (snapshot.proc_name && snapshot.proc_name[i]) {
            fit_cell_text(snapshot.proc_name[i], proc_buf, sizeof(proc_buf));
        } else if (snapshot.pid && snapshot.pid[i] > 0) {
            snprintf(proc_buf, sizeof(proc_buf), "?");
        } else {
            snprintf(proc_buf, sizeof(proc_buf), "-");
        }

        {
            char local_cell[32];
            char peer_cell[32];

            fit_cell_text(local_buf, local_cell, sizeof(local_cell));
            fit_cell_text(peer_buf, peer_cell, sizeof(peer_cell));

            mvprintw(
                current_y, col_state, "%-10s", state_buf
            );
            mvprintw(current_y, col_recv, "%-7lu", snapshot.recv_q ? snapshot.recv_q[i] : 0UL);
            mvprintw(current_y, col_send, "%-7lu", snapshot.send_q ? snapshot.send_q[i] : 0UL);
            mvprintw(current_y, col_local, "%-24s", local_cell);
            mvprintw(current_y, col_peer, "%-24s", peer_cell);
            mvprintw(current_y, col_proc, "%-16s", proc_buf);
        }
        current_y++;
    }
    
    if (snapshot.count > rows_to_show && current_y < max_y - 4) {
        mvprintw(current_y, panel_left + 2, "... и ещё %d соединений", snapshot.count - rows_to_show);
        current_y++;
    }
    CONSOCK_free_copy(&snapshot);
}
