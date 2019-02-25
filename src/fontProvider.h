#ifndef _FONT_PROVIDER_H_
#define _FONT_PROVIDER_H_

#include "graphProvider.h"

//#define FONT_06x08_001
#define FONT_08x16_001

#if defined(FONT_06x08_001)
#   define FONT_WIDTH_PX    6
#   define FONT_HEIGHT_PX   8
#   define FONT_BYTE_SIZE   8
#elif defined(FONT_08x16_001)
#   define FONT_WIDTH_PX    8
#   define FONT_HEIGHT_PX   16
#   define FONT_BYTE_SIZE   16
#endif


void disp_bkt(int x, int y, int height, int is_left);
void disp_char(int x, int y, int c);
void disp_string(int x, int y, const char *str);
void disp_set_color(color_t color);
void disp_aa_line(int x1, int y1, int x2, int y2);
void disp_line(int x1, int y1, int x2, int y2);
void disp_writegraph(int x, int y, int width, int height, const unsigned char *data);

#endif