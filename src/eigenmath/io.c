#include "defs.h"

char    result_buffer[512];
char *  pbuf;

void lineclear () {
    pbuf = result_buffer;
    *pbuf = '\0';
}

void lineend() {
    *pbuf = '\0';
}

const char * lineread() {
    return result_buffer;
}

void printchar(int c)
{
    // putchar(c);
    *pbuf++ = c;
    *pbuf = '\0';
}

void printstr(const char * s)
{
    while (!*s) {
        *pbuf++ = *s;
        *pbuf = '\0';
        // printchar(*s);
        s++;
    }
}