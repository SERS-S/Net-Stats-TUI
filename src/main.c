#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "reactor.h"

#include "data/overall_data.h"
#include "data/interfaces_data.h"
#include "data/addr_dns_data.h"
#include "data/arp_route_data.h"
#include "data/connections_sockets_data.h"
#include "data/protocol_stats_data.h"
#include "data/wifi_data.h"
#include "data/network_profiles_data.h"

#include "draw/overall_draw.h"
#include "draw/interfaces_draw.h"
#include "draw/addr_dns_draw.h"
#include "draw/arp_route_draw.h"
#include "draw/connections_sockets_draw.h"
#include "draw/protocol_stats_draw.h"
#include "draw/wifi_draw.h"
#include "draw/network_profiles_draw.h"

static const char *TABS[] = 
{
    "Overall", "Interfaces",
    "Addr & DNS", "ARP & Route",
    "Sockets", "Protocol Stats",
    "Wi-Fi", "Network Profiles"
};

enum { TAB_COUNT = sizeof(TABS) / sizeof(TABS[0]) };

static void draw_ui(
    int active_tab, 
    OVRLL *ovrll, 
    INTRF *intrf, 
    ADDRDNS *addrdns,
    ARPRT *arprt,
    CONSOCK *consock,
    PROTST *protst,
    WIFI *wifi,
    NETPROF *netprof
) 
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    erase();
    box(stdscr, 0, 0);
    typedef void (*tab_draw_fn) (void *, int, int);

    tab_draw_fn TAB_DRAWERS[TAB_COUNT] = {
        draw_ovrll,
        draw_intrf,
        draw_addrdns,
        draw_arprt,
        draw_consock,
        draw_protst,
        draw_wifi,
        draw_netprof
    };

    void *TAB_DATA[TAB_COUNT] = {
        ovrll,
        intrf,
        addrdns,
        arprt,
        consock,
        protst,
        wifi,
        netprof
    };


    int x = 2;
    int y_tabs = 1;
    for (int i = 0; i < TAB_COUNT; i++) 
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "[%s]", TABS[i]);

        if (x + (int)strlen(buf) >= cols - 2) break;

        if (i == active_tab) attron(A_REVERSE);
        mvprintw(y_tabs, x, "%s", buf);
        if (i == active_tab) attroff(A_REVERSE);

        x += (int)strlen(buf) + 1;
    }

    int y_sep = 2;
    mvhline(y_sep, 1, ACS_HLINE, cols - 2);

    int y_content = y_sep + 2;
    int x_content = 2;

    mvprintw(y_content, x_content, "(контент активной вкладки: %s)", TABS[active_tab]);

    int y_status = rows - 2;
    mvhline(y_status - 1, 1, ACS_HLINE, cols - 2);

    y_content += 2;

    TAB_DRAWERS[active_tab](TAB_DATA[active_tab], y_content, x_content);

    attron(A_REVERSE);
    mvprintw(y_status, 2, "Tab Next | Shift+Tab Prev | s Sort | q Quit");
    for (int i = (int) strlen("Tab Next | Shift+Tab Prev | s Sort | q Quit") + 2; i < cols - 2; i++) 
    {
        mvaddch(y_status, i, ' ');
    }
    attroff(A_REVERSE);

    refresh();
}

int main(void) 
{
    OVRLL ovrll;
    INTRF intrf;
    ADDRDNS addrdns;
    ARPRT arprt;
    CONSOCK consock;
    PROTST protst;
    WIFI wifi;
    NETPROF netprof;

    OVRLL_init(&ovrll);
    INTRF_init(&intrf);
    ADDRDNS_init(&addrdns);
    ARPRT_init(&arprt);
    CONSOCK_init(&consock);
    PROTST_init(&protst);
    WIFI_init(&wifi);
    NETPROF_init(&netprof);

    event_loop(&ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);


    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int active = 0;
    draw_ui(active, &ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);

    while (1) 
    {
        int ch = getch();

        if (ch == 'q' || ch == 'Q') break;

        if (ch == '\t') 
        {
            active = (active + 1) % TAB_COUNT;
            draw_ui(active, &ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);
        } 
        else if (ch == KEY_BTAB) 
        { 
            active = (active - 1 + TAB_COUNT) % TAB_COUNT;
            draw_ui(active, &ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);
        } 
        else if (ch == KEY_RESIZE) 
        {
            draw_ui(active, &ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);
        }
        else draw_ui(active, &ovrll, &intrf, &addrdns, &arprt, &consock, &protst, &wifi, &netprof);
    }

    endwin();

    NETPROF_destroy(&netprof);
    WIFI_destroy(&wifi);
    PROTST_destroy(&protst);
    CONSOCK_destroy(&consock);
    ARPRT_destroy(&arprt);
    ADDRDNS_destroy(&addrdns);
    INTRF_destroy(&intrf);
    OVRLL_destroy(&ovrll);
    
    exit(0);
}
