#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL2/SDL.h>
#include "types.h"

#define BUTTON_A 0
#define BUTTON_B 1
#define BUTTON_SELECT 2
#define BUTTON_START 3
#define BUTTON_UP 4
#define BUTTON_DOWN 5
#define BUTTON_LEFT 6
#define BUTTON_RIGHT 7

class controller {
private:
    int mapping[8];
    u8 shift;

public:
    controller(int port);

    void grabState();
    u8 readBit();
};

#endif
