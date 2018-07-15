#include "controller.h"

controller::controller(int num) {
    if (num == 1) {
        mapping[0] = SDL_SCANCODE_SLASH;
        mapping[1] = SDL_SCANCODE_PERIOD;
        mapping[2] = SDL_SCANCODE_COMMA;
        mapping[3] = SDL_SCANCODE_RETURN;
        mapping[4] = SDL_SCANCODE_UP;
        mapping[5] = SDL_SCANCODE_DOWN;
        mapping[6] = SDL_SCANCODE_LEFT;
        mapping[7] = SDL_SCANCODE_RIGHT;
    } else if (num == 2) {
        mapping[0] = SDL_SCANCODE_C;
        mapping[1] = SDL_SCANCODE_X;
        mapping[2] = SDL_SCANCODE_Z;
        mapping[3] = SDL_SCANCODE_S;
        mapping[4] = SDL_SCANCODE_T;
        mapping[5] = SDL_SCANCODE_G;
        mapping[6] = SDL_SCANCODE_F;
        mapping[7] = SDL_SCANCODE_H;
    }
}

void controller::grabState() {
    shift = 0;
    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    for (int i = 0; i < 8; i++) {
        shift |= state[mapping[i]] << i;
    }
}

u8 controller::readBit() {
    u8 bit = shift & 0x01;
    shift >>= 1;
    shift |= 0x80;
    return 0x40 | bit;
}
