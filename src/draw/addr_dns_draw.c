#include "draw/addr_dns_draw.h"
#include "data/addr_dns_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

static void format_ipv6_short(uint8_t addr[16], char *buffer, size_t buf_size) {
    if (!addr) {
        snprintf(buffer, buf_size, "-");
        return;
    }
    
    snprintf(buffer, buf_size, "%02x%02x:%02x%02x:%02x%02x:%02x%02x...",
             addr[0], addr[1], addr[2], addr[3],
             addr[4], addr[5], addr[6], addr[7]);
}

void draw_addrdns(void *ptr, int y, int x) {
    if (!ptr) {
        mvprintw(y, x, "ADDR/DNS: нет данных");
        return;
    }
    
    ADDRDNS snapshot = *(ADDRDNS *)ptr;
    
    int max_y = getmaxy(stdscr) - 1;
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y || x >= max_x) return;
    
    int current_y = y;
    
    current_y += 2;
    
    if (current_y < max_y) {
        attron(A_BOLD);
        mvprintw(current_y, x, "Interfaces:");
        attroff(A_BOLD);
    }
    current_y++;
    
    int max_interfaces = (snapshot.count < 8) ? snapshot.count : 8;
    
    for (int i = 0; i < max_interfaces && current_y < max_y - 8; i++) {
        if (snapshot.interf_name && snapshot.interf_name[i]) {
            mvprintw(current_y, x + 2, "%s", snapshot.interf_name[i]);
        } else {
            mvprintw(current_y, x + 2, "unknown");
        }
        current_y++;
        
        if (snapshot.ipv4_address && snapshot.ipv4_address[i]) {
            mvprintw(current_y, x + 4, "IPv4 %d.%d.%d.%d/%d",
                     snapshot.ipv4_address[i][0],
                     snapshot.ipv4_address[i][1],
                     snapshot.ipv4_address[i][2],
                     snapshot.ipv4_address[i][3],
                     snapshot.ipv4_mask ? snapshot.ipv4_mask[i] : 24);
            
            if (snapshot.ipv4_mask && snapshot.ipv4_mask[i] < 32) {
                mvprintw(current_y, x + 30, "brd %d.%d.%d.255",
                         snapshot.ipv4_address[i][0],
                         snapshot.ipv4_address[i][1],
                         snapshot.ipv4_address[i][2]);
            }
            current_y++;
        }
        
        if (snapshot.ipv6_address && snapshot.ipv6_address[i]) {
            char ipv6_str[48];
            format_ipv6_short(snapshot.ipv6_address[i], ipv6_str, sizeof(ipv6_str));
            mvprintw(current_y, x + 4, "IPv6 %s/64 (preferred)", ipv6_str);
            current_y++;
        }
    }
    
    if (snapshot.count > 8 && current_y < max_y) {
        mvprintw(current_y, x + 2, "... и ещё %d интерфейсов", snapshot.count - 8);
        current_y++;
    }
    
    current_y++;
    
    if (current_y < max_y) {
        attron(A_BOLD);
        mvprintw(current_y, x, "DNS:");
        attroff(A_BOLD);
    }
    current_y++;
    
    if (current_y < max_y) {
        mvprintw(current_y, x + 2, "manager = %s",
                 snapshot.manager ? snapshot.manager : "unknown");
        
        if (snapshot.manager && strstr(snapshot.manager, "resolved")) {
            mvprintw(current_y, x + 30, "(stub)");
        }
    }
    current_y++;
    
    if (current_y < max_y) {
        mvprintw(current_y, x + 2, "servers: %s",
                 snapshot.servers_list ? snapshot.servers_list : "none");
    }
    current_y++;
    
    if (current_y < max_y) {
        mvprintw(current_y, x + 2, "search : %s",
                 snapshot.search_list ? snapshot.search_list : "none");
    }
    current_y++;
    
    if (current_y < max_y) {
        mvprintw(current_y, x + 2, "resolv.conf: %s",
                 snapshot.resolv_path ? snapshot.resolv_path : "/etc/resolv.conf");
        
        if (snapshot.manager && strstr(snapshot.manager, "resolved")) {
            mvprintw(current_y, x + 40, "→ (stub)");
        }
    }
    current_y += 2;
}