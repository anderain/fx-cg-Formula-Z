#ifndef _INPUT_PROVIDER_H_
#   define _INPUT_PROVIDER_H_
#   include "platform.h"
#   if defined(APP_MSVC)
#       include "SDL.h"
#       define KEY_CTRL_EXIT 0x0
#       define KEY_CTRL_LEFT 0x114
#       define KEY_CTRL_RIGHT 0x113
#       define KEY_CTRL_UP 0x111
#       define KEY_CTRL_DOWN 0x112
#       define KEY_CHAR_0 0x100
#       define KEY_CHAR_1 0x101
#       define KEY_CHAR_2 0x102
#       define KEY_CHAR_3 0x103
#       define KEY_CHAR_4 0x104
#       define KEY_CHAR_5 0x105
#       define KEY_CHAR_6 0x106
#       define KEY_CHAR_7 0x107
#       define KEY_CHAR_8 0x108
#       define KEY_CHAR_9 0x109
#       define KEY_CHAR_A 'a'
#       define KEY_CHAR_B 'b'
#       define KEY_CHAR_C 'c'
#       define KEY_CHAR_D 'd'
#       define KEY_CHAR_E 'e'
#       define KEY_CHAR_F 'f'
#       define KEY_CHAR_G 'g'
#       define KEY_CHAR_H 'h'
#       define KEY_CHAR_I 'i'
#       define KEY_CHAR_J 'j'
#       define KEY_CHAR_K 'k'
#       define KEY_CHAR_L 'l'
#       define KEY_CHAR_M 'm'
#       define KEY_CHAR_N 'n'
#       define KEY_CHAR_I 'i'
#       define KEY_CHAR_P 'p'
#       define KEY_CHAR_Q 'q'
#       define KEY_CHAR_R 'r'
#       define KEY_CHAR_S 's'
#       define KEY_CHAR_T 't'
#       define KEY_CHAR_U 'u'
#       define KEY_CHAR_V 'v'
#       define KEY_CHAR_W 'w'
#       define KEY_CHAR_X 'x'
#       define KEY_CHAR_Y 'y'
#       define KEY_CHAR_Z 'z'
#       define KEY_CHAR_Y 'y'
#       define KEY_CHAR_Z 'z'
#       define KEY_CHAR_PLUS        270
#       define KEY_CHAR_MINUS       269
#       define KEY_CHAR_MULT        268
#       define KEY_CHAR_DIV         267
#       define KEY_CHAR_LPAR        290 /* F9 */
#       define KEY_CHAR_RPAR        291 /* F10 */
#       define KEY_CHAR_COMMA       ','
#       define KEY_CHAR_DP          '.'
#       define KEY_CTRL_EXE         0xd
#       define KEY_CTRL_DEL         127
#       define KEY_CHAR_POW         292 /* F11 */ 
#       define KEY_CHAR_EQUAL       '='
#   elif defined(APP_FXCG)
#       include <keyboard_syscalls.h>
#       include <keyboard.hpp>
#   endif
#endif

unsigned int wait_key();