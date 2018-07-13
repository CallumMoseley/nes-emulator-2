#include "cart.h"

cart000::cart000(int prgBanks, int chrBanks, u8* prg, u8* chr, bool v) {
    this->prgBanks = prgBanks;
    this->chrBanks = chrBanks;
    this->prg = new u8[prgBanks * 0x4000];
    this->chr = new u8[chrBanks * 0x2000];
    memcpy(this->prg, prg, prgBanks * 0x4000);
    memcpy(this->chr, chr, chrBanks * 0x2000);

    vMirror = v;
}
cart000::~cart000() {
    delete[] prg;
    delete[] chr;
}

u8 cart000::readMemoryCPU(u16 addr) {
    if (addr < 0x8000) return 0x00;
    if (prgBanks == 1) {
        return prg[addr & 0x3FFF];
    } else if (prgBanks == 2) {
        return prg[addr & 0x7FFF];
    }
    return 0x00;
}

void cart000::writeMemoryCPU(u16 addr, u8 v) {
    if (addr < 0x8000) return;
    if (prgBanks == 1) {
        prg[addr & 0x3FFF] = v;
    } else if (prgBanks == 2) {
        prg[addr & 0x7FFF] = v;
    }
}

u8 cart000::readMemoryPPU(u16 addr) {
    if (addr >= 0x2000) return 0x00;
    return chr[addr];
}

void cart000::writeMemoryPPU(u16 addr, u8 v) {
    if (addr >= 0x2000) return;
    chr[addr] = v;
}
