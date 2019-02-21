#include "inputProvider.h"

#if defined(APP_MSVC)

unsigned int wait_key() {
    SDL_Event event;
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                return event.key.keysym.sym;
            }
            else if (event.type == SDL_QUIT) {
                return 0;
            }
        }
    }
    return 0;
}

#elif defined(APP_FXCG)

unsigned int wait_key() {
    unsigned int key;
    GetKey(&key);
    return key;
}

#endif