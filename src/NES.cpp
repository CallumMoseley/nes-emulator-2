#include "NES.h"

NES::NES(SDL_Renderer* renderer) :
    cpu(&ppu, &apu, &ctrl1, &ctrl2),
    ppu(renderer),
    ctrl1(1),
    ctrl2(2) {}

NES::~NES() {
    delete gameCart;
}

void NES::start() {
    //gameCart = cart::fromFile("roms/test_roms/branch_timing_tests/1.Branch_Basics.nes");
    gameCart = cart::fromFile("roms/smb.nes");
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
