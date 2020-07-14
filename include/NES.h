#ifndef NES_H
#define NES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include "controller.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "cart.h"
#include "types.h"

class NES {
private:
    CPU cpu;
    PPU ppu;
    APU apu;
    cart* gameCart;

    controller ctrl1;
    controller ctrl2;

public:
    NES(SDL_Renderer* renderer);
    ~NES();

    void start();
};

#endif
