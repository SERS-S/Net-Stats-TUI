#include "draw/arp_route_draw.h"
#include "data/arp_route_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

void draw_arprt(void *ptr, int y, int x) {
    if (!ptr) return;
    
    ARPRT snapshot = ARPRT_get_data((ARPRT *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y || x >= max_x) return;
    
    int current_y = y + 2;
    

    
    if (current_y < max_y) {
        attron(A_BOLD);
        mvprintw(current_y, x, "Table main");
        attroff(A_BOLD);
    }
    current_y++;
    
    int routes_shown = 0;
    for (int i = 0; i < snapshot.count && current_y < max_y - 10; i++) {
        uint8_t type = snapshot.row_type ? snapshot.row_type[i] : 0;
        
        if (type == 1) { // ROUTE
            if (snapshot.route_is_default && snapshot.route_is_default[i]) {
                mvprintw(current_y, x, "default via %s dev %s metric %d",
                         snapshot.route_gateway && snapshot.route_gateway[i] ? snapshot.route_gateway[i] : "?",
                         snapshot.route_dev && snapshot.route_dev[i] ? snapshot.route_dev[i] : "?",
                         snapshot.route_metric ? snapshot.route_metric[i] : 0);
            } else {
                mvprintw(current_y, x, "%s/%d dev %s proto kernel scope link",
                         snapshot.route_dst && snapshot.route_dst[i] ? snapshot.route_dst[i] : "?",
                         snapshot.route_prefix_len ? snapshot.route_prefix_len[i] : 0,
                         snapshot.route_dev && snapshot.route_dev[i] ? snapshot.route_dev[i] : "?");
            }
            current_y++;
            routes_shown++;
        }
    }
    
    if (routes_shown == 0 && current_y < max_y) {
        mvprintw(current_y, x, "No routes");
        current_y++;
    }
    
    current_y += 2;
    
    if (current_y < max_y) {
        attron(A_BOLD | A_UNDERLINE);
        mvprintw(current_y, x, "NEIGHBORS (ARP/NDP)");
        attroff(A_BOLD | A_UNDERLINE);
    }
    current_y += 2;
    
    if (current_y < max_y) {
        attron(A_BOLD);
        mvprintw(current_y, x, "IP");
        mvprintw(current_y, x + 20, "MAC");
        mvprintw(current_y, x + 45, "Dev");
        mvprintw(current_y, x + 55, "State");
        mvprintw(current_y, x + 70, "Last seen");
        attroff(A_BOLD);
    }
    current_y++;
    
    if (current_y < max_y) {
        mvhline(current_y, x, ACS_HLINE, max_x - x);
    }
    current_y++;
    
    int neighbors_shown = 0;
    for (int i = 0; i < snapshot.count && current_y < max_y - 3; i++) {
        uint8_t type = snapshot.row_type ? snapshot.row_type[i] : 0;
        
        if (type == 2) { 
            mvprintw(current_y, x, "| %s",
                     snapshot.neighbor_ip && snapshot.neighbor_ip[i] ? snapshot.neighbor_ip[i] : "?");
            
            if (snapshot.neighbor_mac && snapshot.neighbor_mac[i]) {
                if (strlen(snapshot.neighbor_mac[i]) > 8) {
                    mvprintw(current_y, x + 20, "%.2s:%.2s:%.2s:..:..",
                             snapshot.neighbor_mac[i],
                             snapshot.neighbor_mac[i] + 3,
                             snapshot.neighbor_mac[i] + 6);
                } else {
                    mvprintw(current_y, x + 20, "%s", snapshot.neighbor_mac[i]);
                }
            }
            
            mvprintw(current_y, x + 45, "%s",
                     snapshot.neighbor_dev && snapshot.neighbor_dev[i] ? snapshot.neighbor_dev[i] : "?");
            
            mvprintw(current_y, x + 55, "%s",
                     snapshot.neighbor_state && snapshot.neighbor_state[i] ? snapshot.neighbor_state[i] : "?");
            
            if (snapshot.neighbor_last_seen_sec) {
                if (snapshot.neighbor_last_seen_sec[i] < 1.0) {
                    mvprintw(current_y, x + 70, "%.2f", snapshot.neighbor_last_seen_sec[i]);
                } else {
                    mvprintw(current_y, x + 70, "%.0fs", snapshot.neighbor_last_seen_sec[i]);
                }
            }
            
            current_y++;
            neighbors_shown++;
        }
    }
    
    if (neighbors_shown == 0 && current_y < max_y) {
        mvprintw(current_y, x, "No neighbors");
        current_y++;
    }

    ARPRT_free_copy(&snapshot);
}