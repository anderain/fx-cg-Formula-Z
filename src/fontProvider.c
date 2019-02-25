#include "fontProvider.h"

#if defined(FONT_06x08_001)
#   include "font/font_06x08_001.h"
#elif defined(FONT_08x16_001)
#   include "font/font_08x16_001.h"
#endif

static color_t disp_color = RGB_24_TO_565(0, 0, 0);

void disp_bkt(int x, int y, int height, int is_left) {
    int my;
    if (is_left) {
        //set_pixel(x + 2, y, disp_color);
        //for (my = y + 1; my < y + height - 2; ++my)
        //    set_pixel(x + 1, my, disp_color);
        //set_pixel(x + 2, y + height - 2, disp_color);

        disp_line(x + 2, y,
                  x + 1, y + 1);
        disp_line(x + 1, y + 1,
                  x + 1, y + height - 2);
        disp_line(x + 1, y + height - 1,
                  x + 2, y + height);
    }
    else {
        //set_pixel(x - 2, y, disp_color);
        //for (my = y + 1; my < y + height - 2; ++my)
        //    set_pixel(x - 1, my, disp_color);
        //set_pixel(x - 2, y + height - 2, disp_color);
        disp_line(x - 3, y,
                  x - 2, y + 1);
        disp_line(x - 2, y + 1,
                  x - 2, y + height - 2);
        disp_line(x - 2, y + height - 1,
                  x - 3, y + height);
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
    int offset;
#if defined(FONT_06x08_001)
    if (c < '!' || c > '~') return;
    offset = FONT_BYTE_SIZE * (c - '!');
#else
    offset = FONT_BYTE_SIZE * c;
#endif
    disp_writegraph(x, y, FONT_WIDTH_PX, FONT_HEIGHT_PX, font_data + offset);
}

void disp_writegraph(int x, int y, int width, int height, const unsigned char *data) {
    int i, j;
    int pixel_width;

    pixel_width = width / 8;
    if (width % 8 > 0)
        pixel_width++;

    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            unsigned char p = *(data + i * pixel_width + j / 8);
            if ((p >> (7 - j % 8)) & 1) {
                set_pixel(x + j, y + i, disp_color);
            }
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
// swaps two numbers 
static void swap(int* a, int*b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// returns absolute value of number 
static float absolute(float x)
{
    if (x < 0) return -x;
    else return x;
}

//returns integer part of a floating point number 
static int iPartOfNumber(float x)
{
    return (int)x;
}

//rounds off a number 
static int roundNumber(float x)
{
    return iPartOfNumber(x + 0.5);
}

//returns fractional part of a number 
static float fPartOfNumber(float x)
{
    if (x > 0) return x - iPartOfNumber(x);
    else return x - (iPartOfNumber(x) + 1);

}

//returns 1 - fractional part of number 
static float rfPartOfNumber(float x)
{
    return 1 - fPartOfNumber(x);
}

// draws a pixel on screen of given brightness 
// 0<=brightness<=1. We can use your own library 
// to draw on screen 
static void drawAAPixel(int x, int y, float brightness)
{
    int c = 255 * brightness;
    set_pixel(x, y, RGB_24_TO_565(c, c, c));
}

void disp_aa_line(int x0, int y0, int x1, int y1)
{
    int steep = absolute(y1 - y0) > absolute(x1 - x0);

    // swap the co-ordinates if slope > 1 or we 
    // draw backwards 
    if (steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    //compute the slope 
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dy / dx;
    if (dx == 0.0)
        gradient = 1;

    int xpxl1 = x0;
    int xpxl2 = x1;
    float intersectY = y0;

    // main loop 
    if (steep)
    {
        int x;
        for (x = xpxl1; x <= xpxl2; x++)
        {
            // pixel coverage is determined by fractional 
            // part of y co-ordinate 
            drawAAPixel(iPartOfNumber(intersectY), x,
                rfPartOfNumber(intersectY));
            drawAAPixel(iPartOfNumber(intersectY) - 1, x,
                fPartOfNumber(intersectY));
            intersectY += gradient;
        }
    }
    else
    {
        int x;
        for (x = xpxl1; x <= xpxl2; x++)
        {
            // pixel coverage is determined by fractional 
            // part of y co-ordinate 
            drawAAPixel(x, iPartOfNumber(intersectY),
                rfPartOfNumber(intersectY));
            drawAAPixel(x, iPartOfNumber(intersectY) - 1,
                fPartOfNumber(intersectY));
            intersectY += gradient;
        }
    }
}