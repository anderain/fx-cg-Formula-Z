#include "graphProvider.h"

#if defined(APP_MSVC)

SDL_Surface * screen;

void init_graph_app() {
    printf("Initializing SDL.\n");
    /* Initialize Video */
    if ((SDL_Init(SDL_INIT_VIDEO) == -1)) {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }
    printf("SDL initialized.\n");
    atexit(SDL_Quit);

    // screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
    screen = SDL_SetVideoMode(LCD_WIDTH_PX, LCD_HEIGHT_PX, 16, SDL_SWSURFACE);
    if (screen == NULL) {
        fprintf(stderr, "Couldn't set %dx%dx16 video mode: %s\n", LCD_WIDTH_PX, LCD_HEIGHT_PX, SDL_GetError());
        exit(1);
    }

    SDL_WM_SetCaption("fx-CG Project SDL Wrapper", 0);
}

void SDL_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

void put_disp() {
    SDL_UpdateRect(screen, 0, 0, LCD_WIDTH_PX, LCD_HEIGHT_PX);
}

void set_pixel(int x, int y, color_t color) {
    SDL_PutPixel(screen, x, y, color);
}

void all_clr() {
    SDL_FillRect(screen, NULL, 0xffff);
}

#elif defined(APP_FXCG)

#define _set_pixel(x, y, color) (*(VRAM + (x) + LCD_WIDTH_PX * (y)) = (color_t)color)

void init_graph_app() {
    Bdisp_EnableColor(1);
    DisplayStatusArea();
}

void all_clr() {
    Bdisp_AllClr_VRAM();
}

void put_disp() {
    Bdisp_PutDisp_DD();
}

void set_pixel(int x, int y, color_t color) {
    _set_pixel(x, y, color);
}

#endif
