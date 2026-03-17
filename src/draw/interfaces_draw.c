#include "draw/interfaces_draw.h"
#include "data/interfaces_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Объявляем внешнюю переменную из main
extern int last_key;

static int focused_row = 0;
static int sort_ascending = 1;  // 1 - по возрастанию, 0 - по убыванию
static int sorted = 0;  // флаг, что массив отсортирован
static int *sort_indices = NULL;  // массив индексов для сортировки
static int sort_indices_size = 0;  // размер массива индексов
static INTRF *global_intrf = NULL;  // глобальный указатель для функций сравнения

static const char* format_bytes(int bytes);
static const char* format_packets(int packets);

// Функция сравнения для qsort (по возрастанию)
static int compare_asc(const void *a, const void *b) {
    int idx_a = *(int *)a;
    int idx_b = *(int *)b;
    
    if (!global_intrf || !global_intrf->device_name) return 0;
    
    const char *name_a = global_intrf->device_name[idx_a] ? global_intrf->device_name[idx_a] : "";
    const char *name_b = global_intrf->device_name[idx_b] ? global_intrf->device_name[idx_b] : "";
    
    return strcmp(name_a, name_b);
}

// Функция сравнения для qsort (по убыванию)
static int compare_desc(const void *a, const void *b) {
    return -compare_asc(a, b);
}

// Инициализация массива индексов для сортировки
static void init_sort_indices(INTRF *intrf) {
    if (!intrf || intrf->count <= 0) return;
    
    // Если размер изменился, пересоздаём массив
    if (sort_indices_size != intrf->count) {
        if (sort_indices) {
            free(sort_indices);
        }
        sort_indices = (int *)malloc(intrf->count * sizeof(int));
        sort_indices_size = intrf->count;
    }
    
    // Заполняем индексами по порядку
    for (int i = 0; i < intrf->count; i++) {
        sort_indices[i] = i;
    }
    
    sorted = 0;
}

// Функция сортировки
static void sort_interfaces(INTRF *intrf) {
    if (!intrf || intrf->count <= 0) return;
    
    init_sort_indices(intrf);
    
    // Сортируем индексы
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
    
    // Устанавливаем глобальный указатель для функций сравнения
    global_intrf = intrf;
    
    // Инициализируем массив индексов при первом вызове
    if (sort_indices_size != intrf->count) {
        init_sort_indices(intrf);
    }
    
    // Обрабатываем нажатую клавишу
    if (last_key == 's' || last_key == 'S') {
        // Меняем направление сортировки при повторном нажатии
        if (sorted) {
            sort_ascending = !sort_ascending;
        }
        sort_interfaces(intrf);
        focused_row = 0;  // Сбрасываем выделение на первый элемент
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
    
    char *headers[] = {
        "DEVICE", "TYPE", "STATE", "CONN", 
        "TX", "RX", "MTU", "MAC", "IPv4", "LINK"
    };
    int cols = 10;
    int widths[] = {8, 8, 6, 8, 8, 8, 6, 18, 16, 6};
    
    int table_y = start_y;
    int x = start_x;
    
    if (start_y < max_y && start_x < max_x) {
        attron(A_BOLD);
        
        // Увеличиваем буфер для индикатора сортировки
        char sort_indicator[128] = "";
        
        if (sorted) {
            snprintf(sort_indicator, sizeof(sort_indicator), 
                     " [сортировка: %s, нажмите 's' для смены направления]", 
                     sort_ascending ? "A->Z" : "Z->A");
        } else {
            snprintf(sort_indicator, sizeof(sort_indicator), " [нажмите 's' для сортировки]");
        }
        
        mvprintw(table_y, start_x, "СЕТЕВЫЕ ИНТЕРФЕЙСЫ (всего: %d)%s", 
                 intrf->count, sort_indicator);
        attroff(A_BOLD);
    }
    
    table_y += 2;
    if (table_y >= max_y) return;
    
    if (table_y < max_y) {
        attron(A_BOLD);
        x = start_x;
        for (int i = 0; i < cols; i++) {
            if (x + widths[i] < max_x) {
                mvprintw(table_y, x, "%-*s", widths[i], headers[i]);
            }
            x += widths[i] + 1;
        }
        attroff(A_BOLD);
    }
    
    table_y += 1;
    if (table_y >= max_y) return;
    
    if (table_y < max_y) {
        mvhline(table_y, start_x - 1, ACS_HLINE, max_x - (start_x - 1));
    }
    
    table_y += 1;
    
    // Определяем, какой индекс использовать для отображения
    for (int display_pos = 0; display_pos < intrf->count; display_pos++) {
        if (table_y >= max_y) break;
        
        // Получаем реальный индекс в массиве данных
        int i = sorted ? sort_indices[display_pos] : display_pos;
        
        x = start_x;
        
        if (display_pos == focused_row) {
            attron(A_REVERSE);
        }
        
        int active = 0;
        if (intrf->active_status) {
            active = (intrf->active_status[i / 8] >> (i % 8)) & 1;
        }
        char *status = active ? "UP" : "DOWN";
        
        if (x < max_x) {
            mvprintw(table_y, x, "%-*s", widths[0], 
                     intrf->device_name && intrf->device_name[i] ? intrf->device_name[i] : "-");
        }
        x += widths[0] + 1;
        
        if (x < max_x) {
            mvprintw(table_y, x, "%-*s", widths[1], 
                     intrf->device_type && intrf->device_type[i] ? intrf->device_type[i] : "-");
        }
        x += widths[1] + 1;
        
        if (x < max_x) {
            if (!active) {
                attron(COLOR_PAIR(1));
                mvprintw(table_y, x, "%-*s", widths[2], status);
                attroff(COLOR_PAIR(1));
            } else {
                mvprintw(table_y, x, "%-*s", widths[2], status);
            }
        }
        x += widths[2] + 1;
        
        if (x < max_x) {
            mvprintw(table_y, x, "%-*s", widths[3], 
                     intrf->conn_name && intrf->conn_name[i] ? intrf->conn_name[i] : "-");
        }
        x += widths[3] + 1;
        
        if (x < max_x) {
            char tx_str[8];
            if (intrf->tx_rate_kibs) {
                snprintf(tx_str, sizeof(tx_str), "%.0f", intrf->tx_rate_kibs[i]);
            } else {
                strcpy(tx_str, "-");
            }
            mvprintw(table_y, x, "%-*s", widths[4], tx_str);
        }
        x += widths[4] + 1;
        
        if (x < max_x) {
            char rx_str[8];
            if (intrf->rx_rate_kibs) {
                snprintf(rx_str, sizeof(rx_str), "%.0f", intrf->rx_rate_kibs[i]);
            } else {
                strcpy(rx_str, "-");
            }
            mvprintw(table_y, x, "%-*s", widths[5], rx_str);
        }
        x += widths[5] + 1;
        
        if (x < max_x) {
            char mtu_str[8];
            if (intrf->mtu_interf) {
                snprintf(mtu_str, sizeof(mtu_str), "%d", intrf->mtu_interf[i]);
            } else {
                strcpy(mtu_str, "-");
            }
            mvprintw(table_y, x, "%-*s", widths[6], mtu_str);
        }
        x += widths[6] + 1;
        
        if (x < max_x) {
            char mac_str[18];
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
            mvprintw(table_y, x, "%-*s", widths[7], mac_str);
        }
        x += widths[7] + 1;
        
        if (x < max_x) {
            char ip_str[16];
            if (intrf->ipv4_address && intrf->ipv4_address[i]) {
                snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d",
                        intrf->ipv4_address[i][0],
                        intrf->ipv4_address[i][1],
                        intrf->ipv4_address[i][2],
                        intrf->ipv4_address[i][3]);
            } else {
                strcpy(ip_str, "-");
            }
            mvprintw(table_y, x, "%-*s", widths[8], ip_str);
        }
        x += widths[8] + 1;
        
        if (x < max_x) {
            char link_str[16] = "-";
            if (intrf->device_link) {
                int speed_mbps = intrf->device_link[i];
                if (speed_mbps >= 1000) {
                    int speed_g = speed_mbps / 1000;
                    if (speed_g > 999) {
                        strcpy(link_str, ">999G");
                    } else {
                        if (speed_g > 99) {
                            strcpy(link_str, ">99G");
                        } else {
                            snprintf(link_str, sizeof(link_str), "%dG", speed_g);
                        }
                    }
                } else {
                    if (speed_mbps > 999) {
                        strcpy(link_str, ">999M");
                    } else {
                        if (speed_mbps > 99) {
                            strcpy(link_str, ">99M");
                        } else {
                            snprintf(link_str, sizeof(link_str), "%dM", speed_mbps);
                        }
                    }
                }
            }
            mvprintw(table_y, x, "%-*s", widths[9], link_str);
        }
        
        if (display_pos == focused_row) {
            attroff(A_REVERSE);
        }
        
        table_y += 1;
    }
}

int draw_top(INTRF *intrf) {
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    int y_top = 3;
    
    if (y_top >= max_y) return y_top;
    
    int h = max_y;
    int y_bottom = y_top + (h - 5) / 3 * 2 - 2;
    
    if (y_bottom >= max_y) {
        y_bottom = max_y - 1;
    }
    
    draw_interfaces_table(intrf, y_top, 2);
    
    return y_bottom;
}

void draw_intrf(void *ptr, int y, int x){
    INTRF *intrf = (INTRF *)ptr;
    int h = getmaxy(stdscr);
    int l = getmaxx(stdscr);
    int top_end = draw_top(intrf);
    draw_bottom(top_end, intrf);
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
    
    // Определяем реальный индекс выбранного интерфейса с учётом сортировки
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
    
    // Очищаем область деталей
    for (int row = y_start; row < y_bottom_limit; row++) {
        move(row, 2);
        clrtoeol();
    }
    
    int y = y_start;
    
    // ------------------------------------------------------------------------
    // ЗАГОЛОВОК
    // ------------------------------------------------------------------------
    attron(A_BOLD | A_UNDERLINE);
    mvprintw(y, 2, "ДЕТАЛЬНАЯ ИНФОРМАЦИЯ: %s", 
             intrf->device_name && intrf->device_name[i] ? intrf->device_name[i] : "?");
    attroff(A_BOLD | A_UNDERLINE);
    
    y += 2;
    if (y >= y_bottom_limit) return;
    
    // ------------------------------------------------------------------------
    // IP АДРЕСА
    // ------------------------------------------------------------------------
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
        mvprintw(y, 30, "IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x::/64",
                intrf->ipv6_address[i][0], intrf->ipv6_address[i][1],
                intrf->ipv6_address[i][2], intrf->ipv6_address[i][3],
                intrf->ipv6_address[i][4], intrf->ipv6_address[i][5],
                intrf->ipv6_address[i][6], intrf->ipv6_address[i][7]);
    } else {
        mvprintw(y, 30, "IPv6: -");
    }
    
    y++;
    if (y >= y_bottom_limit) return;
    
    // ------------------------------------------------------------------------
    // RX СТАТИСТИКА
    // ------------------------------------------------------------------------
    mvprintw(y, 4, "RX:  bytes %s  pkts %s  drop %d  err %d",
             format_bytes(intrf->rx_total_bytes ? intrf->rx_total_bytes[i] : 0),
             format_packets(intrf->rx_total_packs ? intrf->rx_total_packs[i] : 0),
             intrf->rx_total_drops ? intrf->rx_total_drops[i] : 0,
             intrf->rx_total_errors ? intrf->rx_total_errors[i] : 0);
    
    y++;
    if (y >= y_bottom_limit) return;
    
    // ------------------------------------------------------------------------
    // TX СТАТИСТИКА
    // ------------------------------------------------------------------------
    mvprintw(y, 4, "TX:  bytes %s  pkts %s  drop %d  err %d",
             format_bytes(intrf->tx_total_bytes ? intrf->tx_total_bytes[i] : 0),
             format_packets(intrf->tx_total_packs ? intrf->tx_total_packs[i] : 0),
             intrf->tx_total_drops ? intrf->tx_total_drops[i] : 0,
             intrf->tx_total_errors ? intrf->tx_total_errors[i] : 0);
    
    y++;
    if (y >= y_bottom_limit) return;
    
    // ------------------------------------------------------------------------
    // LINK ИНФОРМАЦИЯ
    // ------------------------------------------------------------------------
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
    
    mvprintw(y, 4, "Link: %s  %s-duplex  carrier:%d  operstate: %s",
             link_str,
             duplex_str,
             intrf->device_link ? 1 : 0,
             operstate_str);
    
    y++;
    if (y >= y_bottom_limit) return;
    
    // ------------------------------------------------------------------------
    // GATEWAY
    // ------------------------------------------------------------------------
    if (intrf->gw_ipv4_address && intrf->gw_ipv4_address[i]) {
        mvprintw(y, 4, "GW:  %d.%d.%d.%d",
                intrf->gw_ipv4_address[i][0],
                intrf->gw_ipv4_address[i][1],
                intrf->gw_ipv4_address[i][2],
                intrf->gw_ipv4_address[i][3]);
    } else {
        mvprintw(y, 4, "GW:  -");
    }
    
    if (intrf->gw_ipv6_address && intrf->gw_ipv6_address[i]) {
        mvprintw(y, 30, "GW IPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x::",
                intrf->gw_ipv6_address[i][0], intrf->gw_ipv6_address[i][1],
                intrf->gw_ipv6_address[i][2], intrf->gw_ipv6_address[i][3],
                intrf->gw_ipv6_address[i][4], intrf->gw_ipv6_address[i][5],
                intrf->gw_ipv6_address[i][6], intrf->gw_ipv6_address[i][7]);
    }
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

void line_to_end(void) {
    int y, x;
    getyx(stdscr, y, x); 
    
    int max_x = getmaxx(stdscr) - 1;  
    int chars_left = max_x - x;   
    
    if (chars_left > 0) {
        hline(ACS_HLINE, chars_left);
    }
}