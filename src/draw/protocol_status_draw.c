#include "draw/protocol_stats_draw.h"
#include "data/protocol_stats_data.h"
#include <curses.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

static void format_rate(char *buffer, size_t buf_size, float rate) {
    if (rate >= 1000000.0f) {
        snprintf(buffer, buf_size, "(%.1fM/s)", rate / 1000000.0f);
    } else if (rate >= 1000.0f) {
        snprintf(buffer, buf_size, "(%.1fk/s)", rate / 1000.0f);
    } else if (rate >= 1.0f) {
        snprintf(buffer, buf_size, "(%.0f/s)", rate);
    } else {
        snprintf(buffer, buf_size, "(%.1f/s)", rate);
    }
}

static void format_total(char *buffer, size_t buf_size, unsigned long long total) {
    if (total >= 1000000000ULL) {
        snprintf(buffer, buf_size, "%.1fG", total / 1000000000.0);
    } else if (total >= 1000000ULL) {
        snprintf(buffer, buf_size, "%.1fM", total / 1000000.0);
    } else if (total >= 1000ULL) {
        snprintf(buffer, buf_size, "%.1fk", total / 1000.0);
    } else {
        snprintf(buffer, buf_size, "%llu", total);
    }
}

void draw_protst(void *ptr, int y, int x) {
    if (!ptr) {
        mvprintw(y, x, "PROTOCOL STATS: нет данных");
        return;
    }
    
    PROTST snapshot = PROTST_get_data((PROTST *)ptr);
    
    int max_y = getmaxy(stdscr);
    int max_x = getmaxx(stdscr);
    
    if (y >= max_y - 3 || x >= max_x) return;
    
    int current_y = y;
    int line_count = 0;
    
    current_y++;
    line_count++;
    
    if (current_y < max_y - 3) {
        current_y++;
        line_count++;
    }
    
    char total_buf[16];
    char rate_buf[32];
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.tcp_insegs_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.tcp_insegs_rate);
        mvprintw(current_y, x, "TCP: InSegs %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.tcp_outsegs_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.tcp_outsegs_rate);
        mvprintw(current_y, x, "     OutSegs %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.tcp_retranssegs_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.tcp_retranssegs_rate);
        mvprintw(current_y, x, "     RetransSegs %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.tcp_estabresets_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.tcp_estabresets_rate);
        mvprintw(current_y, x, "     EstabResets %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.tcp_listenoverflows_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.tcp_listenoverflows_rate);
        mvprintw(current_y, x, "     ListenOverflows %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.ip_indelivers_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.ip_indelivers_rate);
        mvprintw(current_y, x, "IP : InDelivers %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.ip_indiscards_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.ip_indiscards_rate);
        mvprintw(current_y, x, "     InDiscards %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.ip_outdiscards_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.ip_outdiscards_rate);
        mvprintw(current_y, x, "     OutDiscards %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.icmp_inerrors_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.icmp_inerrors_rate);
        mvprintw(current_y, x, "ICMP: InErrors %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.icmp_outerrors_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.icmp_outerrors_rate);
        mvprintw(current_y, x, "      OutErrors %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.icmp_unreach_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.icmp_unreach_rate);
        mvprintw(current_y, x, "      Unreach %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.icmp_timeexcd_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.icmp_timeexcd_rate);
        mvprintw(current_y, x, "      TimeExcd %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.udp_indatagrams_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.udp_indatagrams_rate);
        mvprintw(current_y, x, "UDP: InDatagrams %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
    
    if (current_y < max_y - 3) {
        format_total(total_buf, sizeof(total_buf), snapshot.udp_noports_total);
        format_rate(rate_buf, sizeof(rate_buf), snapshot.udp_noports_rate);
        mvprintw(current_y, x, "     NoPorts %s %s", total_buf, rate_buf);
        current_y++;
        line_count++;
    }
}