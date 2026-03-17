#ifndef INTERFACES_DRAW_H
#define INTERFACES_DRAW_H

#include "data/interfaces_data.h"

int draw_top(INTRF *intrf);
void draw_intrf(void *ptr, int y, int x);
void draw_bottom(int top_end, INTRF *intrf);
void line_to_end(void);


#endif