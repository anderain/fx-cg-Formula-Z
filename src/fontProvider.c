#include "fontProvider.h"

extern const char char6x8_dat[];
static color_t disp_color = RGB_24_TO_565(0, 0, 0);

void disp_line(int x1, int y1, int x2, int y2) {
    int x, y, dx, dy, d;
    y = y1;
    dx = x2 - x1;
    dy = y2 - y1;
    d = 2 * dy - dx;
    for (x = x1; x <= x2; x++) {
        set_pixel(x, y, disp_color);
        if (d < 0) {
            d += 2 * dy;
        }
        else {
            y++;
            d += 2 * dy - 2 * dx;
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