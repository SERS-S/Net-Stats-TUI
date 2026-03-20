#include "draw/wifi_draw.h"
#include "data/wifi_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

void draw_wifi(void *ptr, int y, int x) {
    if (!ptr) {
        mvprintw(y, x, "WI-FI: нет данных");
        return;
    }
    
    WIFI snapshot = WIFI_get_data((WIFI *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y - 3 || x >= max_x) return;
    
    int current_y = y + 2;
    
    if (current_y < max_y - 3) {
        mvprintw(current_y, x, "count: %d", snapshot.count);
    }
    current_y++;
    
    if (snapshot.count == 0) {
        if (current_y < max_y - 3) {
            mvprintw(current_y, x, "Нет Wi-Fi интерфейсов");
        }
        return;
    }
    
    int max_interfaces = (snapshot.count < 5) ? snapshot.count : 5;
    
    for (int i = 0; i < max_interfaces && current_y < max_y - 5; i++) {
        if (snapshot.iface && snapshot.iface[i]) {
            mvprintw(current_y, x, "iface: %s", snapshot.iface[i]);
            
            if (snapshot.state && snapshot.state[i]) {
                mvprintw(current_y, x + 25, "state: %s", snapshot.state[i]);
            }
        } else {
            mvprintw(current_y, x, "iface: -");
        }
        current_y++;
        
        if (snapshot.ssid && snapshot.ssid[i]) {
            mvprintw(current_y, x, "SSID: %s", snapshot.ssid[i]);
        } else {
            mvprintw(current_y, x, "SSID: -");
        }
        
        if (snapshot.bssid && snapshot.bssid[i]) {
            mvprintw(current_y, x + 35, "BSSID: %s", snapshot.bssid[i]);
        }
        current_y++;
        
        if (snapshot.rssi_dbm) {
            mvprintw(current_y, x, "RSSI: %d dBm", snapshot.rssi_dbm[i]);
        } else {
            mvprintw(current_y, x, "RSSI: - dBm");
        }
        
        if (snapshot.quality_pct) {
            mvprintw(current_y, x + 20, "Quality: %d%%", snapshot.quality_pct[i]);
        }
        
        if (snapshot.tx_bitrate_mbps) {
            mvprintw(current_y, x + 40, "TX bitrate: %.0f Mb/s", snapshot.tx_bitrate_mbps[i]);
        }
        
        if (snapshot.rx_bitrate_mbps) {
            mvprintw(current_y, x + 65, "RX bitrate: %.0f Mb/s", snapshot.rx_bitrate_mbps[i]);
        }
        current_y++;
        
        if (snapshot.mcs) {
            mvprintw(current_y, x, "MCS: %d", snapshot.mcs[i]);
        }
        
        if (snapshot.retries_per_sec) {
            mvprintw(current_y, x + 15, "Retries/s: %.1f", snapshot.retries_per_sec[i]);
        }
        
        if (snapshot.beacon_loss_per_sec) {
            mvprintw(current_y, x + 40, "Beacon loss: %.0f", snapshot.beacon_loss_per_sec[i]);
        }
        current_y += 2;
    }
    
    if (snapshot.count > max_interfaces && current_y < max_y - 3) {
        mvprintw(current_y, x, "... и ещё %d интерфейсов", snapshot.count - max_interfaces);
        current_y++;
    }
    WIFI_free_copy(&snapshot);
}