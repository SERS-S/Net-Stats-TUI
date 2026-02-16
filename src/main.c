#include <locale.h>
#include <string.h>
#include <ncursesw/curses.h>

static const char *TABS[] = 
{
    "Overall", "Interfaces",
    "Addr/DNS", "ARP/Route",
    "Sockets", "Proto",
    "Wi-Fi", "Profiles"
};

enum { TAB_COUNT = sizeof(TABS) / sizeof(TABS[0]) };

static void draw_ui(int active_tab) 
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    erase();
    box(stdscr, 0, 0);

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
    mvprintw(y_content, 2, "(контент активной вкладки: %s)", TABS[active_tab]);

    int y_status = rows - 2;
    mvhline(y_status - 1, 1, ACS_HLINE, cols - 2);

    attron(A_REVERSE);
    mvprintw(y_status, 2, "Tab Next  Shift+Tab Prev  s Sort  q Quit");
    for (int i = (int) strlen("Tab Next  Shift+Tab Prev  s Sort  q Quit") + 2; i < cols - 2; i++) 
    {
        mvaddch(y_status, i, ' ');
    }
    attroff(A_REVERSE);

    refresh();
}

int main(void) 
{
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int active = 0;
    draw_ui(active);

    while (1) 
    {
        int ch = getch();

        if (ch == 'q' || ch == 'Q') break;

        if (ch == '\t') 
        {
            active = (active + 1) % TAB_COUNT;
            draw_ui(active);
        } 
        else if (ch == KEY_BTAB) 
        { 
            active = (active - 1 + TAB_COUNT) % TAB_COUNT;
            draw_ui(active);
        } 
        else if (ch == KEY_RESIZE) 
        {
            draw_ui(active);
        }
    }

    endwin();
    return 0;
}
