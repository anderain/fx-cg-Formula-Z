#ifndef _GRAPH_PROVIDER_H_
#   define LCD_WIDTH_PX 384
#   define LCD_HEIGHT_PX 216
#   define _GRAPH_PROVIDER_H_
#   include "platform.h"
#   if defined(APP_MSVC)
#       include "SDL.h"
#       define WIN_ZOOM 2
#   elif defined(APP_FXCG)
#       include <display.h>
#       include <display_syscalls.h>
#       define VRAM ((color_t*)0xA8000000)
#       define setPixel(x, y, color) (*(VRAM + (x) + LCD_WIDTH_PX * (y)) = (color_t)color)
#       define putDisp() Bdisp_PutDisp_DD()
#   endif

typedef unsigned short color_t;

#define RGB_24_TO_565(r, g, b) (((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)
#define COLOR_ALPHA ((color_t)0xf81f)

void set_pixel(int x, int y, color_t color);
void init_graph_app();
void all_clr();
void put_disp();

#endif
