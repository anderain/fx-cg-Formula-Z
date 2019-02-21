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
#   elif defined(APP_FXCG)
#       include <keyboard_syscalls.h>
#       include <keyboard.hpp>
#   endif
#endif

unsigned int wait_key();