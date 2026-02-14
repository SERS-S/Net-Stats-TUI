#include <locale.h>
#include <wchar.h>
#include <ncursesw/curses.h>

int main(void) 
{
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    addwstr(L"Привет, net-htop (MVP)!");
    mvaddwstr(2, 0, L"Нажми любую клавишу для выхода...");
    refresh();

    getch();
    endwin();
    return 0;
}
