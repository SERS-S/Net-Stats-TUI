#include "draw/network_profiles_draw.h"
#include "data/network_profiles_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

void draw_netprof(void *ptr, int y, int x) {
    if (!ptr) {
        mvprintw(y, x, "PROFILES: нет данных");
        return;
    }
    
    NETPROF snapshot = NETPROF_get_data((NETPROF *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y - 3 || x >= max_x) return;
    
    int current_y = y + 2;

    if (current_y < max_y - 3) {
        mvprintw(current_y, x, "Всего профилей: %d", snapshot.count);
    }
    current_y += 2;
    
    if (current_y < max_y - 3) {
        attron(A_BOLD);
        mvprintw(current_y, x, "NAME");
        mvprintw(current_y, x + 25, "UUID");
        mvprintw(current_y, x + 55, "TYPE");
        mvprintw(current_y, x + 75, "DEVICE");
        mvprintw(current_y, x + 90, "STATE");
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
        if (snapshot.name && snapshot.name[i]) {
            mvprintw(current_y, x, "%-25.25s", snapshot.name[i]);
        } else {
            mvprintw(current_y, x, "%-25s", "-");
        }
        
        if (snapshot.uuid && snapshot.uuid[i]) {
            if (strlen(snapshot.uuid[i]) > 20) {
                mvprintw(current_y, x + 25, "%-20.20s...", snapshot.uuid[i]);
            } else {
                mvprintw(current_y, x + 25, "%-23s", snapshot.uuid[i]);
            }
        } else {
            mvprintw(current_y, x + 25, "%-23s", "-");
        }
        
        if (snapshot.type && snapshot.type[i]) {
            mvprintw(current_y, x + 55, "%-20.20s", snapshot.type[i]);
        } else {
            mvprintw(current_y, x + 55, "%-20s", "-");
        }
        
        if (snapshot.device && snapshot.device[i]) {
            mvprintw(current_y, x + 75, "%-10.10s", snapshot.device[i]);
        } else {
            mvprintw(current_y, x + 75, "%-10s", "-");
        }
        
        if (snapshot.state && snapshot.state[i]) {
            if (strcmp(snapshot.state[i], "up") == 0) {
                attron(COLOR_PAIR(2));
                mvprintw(current_y, x + 90, "%-10s", snapshot.state[i]);
                attroff(COLOR_PAIR(2));
            } else if (strcmp(snapshot.state[i], "down") == 0) {
                attron(COLOR_PAIR(1));
                mvprintw(current_y, x + 90, "%-10s", snapshot.state[i]);
                attroff(COLOR_PAIR(1));
            } else {
                mvprintw(current_y, x + 90, "%-10s", snapshot.state[i]);
            }
        } else {
            mvprintw(current_y, x + 90, "%-10s", "-");
        }
        
        current_y++;
    }
    
    if (snapshot.count > rows_to_show && current_y < max_y - 4) {
        mvprintw(current_y, x, "... и ещё %d профилей", snapshot.count - rows_to_show);
        current_y++;
    }
    NETPROF_free_copy(&snapshot);
}