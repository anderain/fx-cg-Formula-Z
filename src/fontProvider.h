#ifndef _FONT_PROVIDER_H_
#define _FONT_PROVIDER_H_

#include "graphProvider.h"

#define FONT_WIDTH_PX 6
#define FONT_HEIGHT_PX 8

void disp_bkt(int x, int y, int height, int is_left);
void disp_char(int x, int y, int c);
void disp_string(int x, int y, const char *str);
void disp_set_color(color_t color);
void disp_line(int x1, int y1, int x2, int y2);

#endif