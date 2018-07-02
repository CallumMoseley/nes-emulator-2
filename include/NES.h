#ifndef NES_H
#define NES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include "CPU.h"
#include "PPU.h"
#include "cart.h"

class NES {
private:
    CPU cpu;
    PPU ppu;
    cart* gameCart;

public:
    NES(SDL_Renderer* renderer);

    void start();
};

#endif
