#include "draw/connections_sockets_draw.h"
#include "data/connections_sockets_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

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
    
    CONSOCK snapshot = *(CONSOCK *)ptr;
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y - 3 || x >= max_x) return;

    int current_y = y + 2;
    
    if (current_y < max_y - 3) {
        attron(A_BOLD);
        mvprintw(current_y, x, "| State     | Recv-Q | Send-Q | Local Address:Port     | Peer Address:Port      | Proc/Info");
        attroff(A_BOLD);
    }
    current_y++;
    
    if (current_y < max_y - 3) {
        mvhline(current_y, x, ACS_HLINE, max_x - x);
    }
    current_y++;
    
    int max_rows = max_y - current_y - 4;
    int rows_to_show = (snapshot.count < max_rows) ? snapshot.count : max_rows;
    
    for (int i = 0; i < rows_to_show && current_y < max_y - 4; i++) {
        char state_buf[12];
        const char *state = snapshot.state && snapshot.state[i] ? snapshot.state[i] : "?";
        snprintf(state_buf, sizeof(state_buf), "%-10s", filter_state(state));
        mvprintw(current_y, x, "| %s", state_buf);
        
        if (snapshot.recv_q) {
            mvprintw(current_y, x + 12, " %-6lu", snapshot.recv_q[i]);
        } else {
            mvprintw(current_y, x + 12, " %-6s", "0");
        }
        
        if (snapshot.send_q) {
            mvprintw(current_y, x + 20, " %-6lu", snapshot.send_q[i]);
        } else {
            mvprintw(current_y, x + 20, " %-6s", "0");
        }
        
        if (snapshot.local_ip && snapshot.local_ip[i]) {
            if (snapshot.local_port) {
                mvprintw(current_y, x + 28, " %-15s:%-5u", 
                         snapshot.local_ip[i], snapshot.local_port[i]);
            } else {
                mvprintw(current_y, x + 28, " %-15s:%-5s", snapshot.local_ip[i], "*");
            }
        } else {
            mvprintw(current_y, x + 28, " %-21s", "*:*");
        }
        
        if (snapshot.peer_ip && snapshot.peer_ip[i]) {
            if (snapshot.peer_port) {
                mvprintw(current_y, x + 52, " %-15s:%-5u", 
                         snapshot.peer_ip[i], snapshot.peer_port[i]);
            } else {
                mvprintw(current_y, x + 52, " %-15s:%-5s", snapshot.peer_ip[i], "*");
            }
        } else {
            mvprintw(current_y, x + 52, " %-21s", "*:*");
        }
        
        if (snapshot.proc_name && snapshot.proc_name[i]) {
            mvprintw(current_y, x + 76, " %-20s", snapshot.proc_name[i]);
        } else if (snapshot.pid && snapshot.pid[i] > 0) {
            mvprintw(current_y, x + 76, " %-20s", "?");
        } else {
            mvprintw(current_y, x + 76, " %-20s", "-");
        }
        
        current_y++;
    }
    
    if (snapshot.count > rows_to_show && current_y < max_y - 4) {
        mvprintw(current_y, x, "... и ещё %d соединений", snapshot.count - rows_to_show);
        current_y++;
    }
}