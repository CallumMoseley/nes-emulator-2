#ifndef NES_H
#define NES_H

#include "CPU.h"
#include "PPU.h"

class NES {
private:
    CPU cpu;
    PPU ppu;

public:
    NES();

    void start();
};

#endif
