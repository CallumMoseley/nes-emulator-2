#include "NES.h"

NES::NES(SDL_Renderer* renderer) : cpu(&ppu), ppu(renderer) {}

void NES::start() {
//    gameCart = new allsuite_cart();
    gameCart = cart::fromFile("roms/donkey_kong.nes");
    cpu.gameCart = gameCart;
    ppu.gameCart = gameCart;
    cpu.powerOn();
    ppu.powerOn();

    for (;;) {
        cpu.op();
    }
}
