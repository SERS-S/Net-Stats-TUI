#ifndef INTERFACES_DRAW_H
#define INTERFACES_DRAW_H

#include <curses.h>
#include "../data/interfaces_data.h"

extern int last_key;

void draw_intrf(void *ptr, int y, int x);
int draw_top(INTRF *intrf);
void draw_bottom(int top_end, INTRF *intrf);
void line_to_end(void);
int handle_intrf_input(int key, INTRF *intrf);

#endif