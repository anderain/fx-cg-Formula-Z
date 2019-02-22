#include "fontProvider.h"

extern const char char6x8_dat[];
static color_t disp_color = RGB_24_TO_565(0, 0, 0);

void disp_bkt(int x, int y, int height, int is_left) {
    int my;
    if (is_left) {
        set_pixel(x + 2, y, disp_color);
        for (my = y + 1; my < y + height - 2; ++my)
            set_pixel(x + 1, my, disp_color);
        set_pixel(x + 2, y + height - 2, disp_color);
    }
    else {
        set_pixel(x - 2, y, disp_color);
        for (my = y + 1; my < y + height - 2; ++my)
            set_pixel(x - 1, my, disp_color);
        set_pixel(x - 2, y + height - 2, disp_color);
    }
}


void disp_line(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int x = x0;
    int y = y0;
    int stepX = 1;
    int stepY = 1;
    if (x0 > x1)
        stepX = -1;
    if (y0 > y1)
        stepY = -1;
    int i;

    if (dx > dy) {
        int e = dy * 2 - dx;
        for (i = 0; i <= dx; i++) {
            set_pixel(x, y, disp_color);
            x += stepX;
            e += dy;
            if (e >= 0) {
                y += stepY;
                e -= dx;
            }
        }
    }
    else {
        int e = 2 * dx - dy;
        for (i = 0; i <= dy; i++) {
            set_pixel(x, y, disp_color);
            y += stepY;
            e += dx;
            if (e >= 0) {
                x += stepX;
                e -= dy;
            }
        }
    }
}


void disp_char(int x, int y, int c) {
    int i, j, offset;
    
    if (c < '!' || c > '~') return;
    
    offset = 8 * (c - '!');

    for (i = 0; i < FONT_HEIGHT_PX; ++i) {
        unsigned char p = char6x8_dat[offset + i];
        for (j = 7; j >= 0; --j) {
            if (p & 1) {
                set_pixel(x + j, y + i, disp_color);
            }
            p = p >> 1;
        }
    }
}

void disp_string(int x, int y, const char *str) {
    int i;
    for (i = 0; str[i]; ++i) {
        disp_char(x, y, str[i]);
        x += FONT_WIDTH_PX;
    }
}

void disp_set_color(color_t color) {
    disp_color = color;
}