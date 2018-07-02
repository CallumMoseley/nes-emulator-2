#include "NES.h"

NES::NES(SDL_Renderer* renderer) : cpu(&ppu), ppu(renderer) {}

void NES::start() {
//    gameCart = new allsuite_cart();
    gameCart = cart::fromFile("roms/donkey_kong.nes");
    cpu.gameCart = gameCart;
    ppu.gameCart = gameCart;
    cpu.powerOn();
    ppu.powerOn();

    SDL_Event event;

    bool quit = false;
    for (;;) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
                break;
            }
        }
        if (quit) break;
        cpu.op();
    }
}
