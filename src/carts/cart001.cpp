#include "cart.h"

cart001::cart001(int prgRamSize, int prgBanks, int chrBanks, u8* prg, u8* chr) {
    this->prgBanks = prgBanks;
    this->chrBanks = chrBanks;
    this->prgRom = new u8[prgBanks * 0x4000];
    this->chrRom = new u8[chrBanks * 0x2000];
    this->prgRam = new u8[prgRamSize];
    memcpy(this->prgRom, prg, prgBanks * 0x4000);
    memcpy(this->chrRom, chr, chrBanks * 0x2000);

    mirroring = 0;

    prgRomMode = 3;
    chrRomMode = 0;

    chrBank0 = 0;
    chrBank1 = 0;
    prgBank = 0;

    prgRamEnable = 1;
    sr = 1 << 4;
}
cart001::~cart001() {
    delete[] prgRam;
    delete[] prgRom;
    delete[] chrRom;
}

u8 cart001::readMemoryCPU(u16 addr) {
    if (addr < 0x6000) return 0x00;
    if (addr >= 0x6000 && addr < 0x8000) {
        return prgRam[addr - 0x6000];
    }
    if (addr < 0xC000) {
        if (prgRomMode >> 1 == 0) {
            return prgRom[addr - 0x8000 + 0x8000 * (prgBank & 0xE)];
        } else if (prgRomMode == 2) {
            return prgRom[addr - 0x8000];
        } else {
            return prgRom[addr - 0x8000 + 0x4000 * prgBank];
        }
    } else  {
        if (prgRomMode >> 1 == 0) {
            return prgRom[addr - 0x8000 + 0x8000 * (prgBank & 0xE)];
        } else if (prgRomMode == 2) {
            return prgRom[addr - 0xC000 + 0x4000 * prgBank];
        } else {
            return prgRom[addr - 0xC000 + 0x4000 * (prgBanks - 1)];
        }
    }
    return 0x00;
}

void cart001::writeMemoryCPU(u16 addr, u8 v) {
    if (addr >= 0x6000 && addr < 0x8000) {
        prgRam[addr - 0x6000] = v;
    } else {
        if (v >> 7 == 1) {
            prgRomMode = 3;
            sr = 1 << 4;
        } else {
            u8 b0 = sr & 1;
            sr >>= 1;
            sr |= (v & 1 << 4);
            if (b0) {
                if (addr < 0xA000) {
                    chrRomMode = sr >> 4;
                    prgRomMode = sr >> 2 & 0x03;
                    mirroring = sr & 0x03;
                } else if (addr < 0xC000) {
                    chrBank0 = v & 0x1F;
                } else if (addr < 0xE000) {
                    chrBank1 = v & 0x1F;
                } else {
                    prgBank = v & 0x0F;
                    prgRamEnable = v >> 4 & 1;
                }
                sr = 1 << 4;
            }
        }
    }
}

u8 cart001::readMemoryPPU(u16 addr) {
    if (chrRomMode == 0) {
        return chrRom[addr + (chrBank0 & 0x1E) * 0x2000];
    } else {
        if (addr < 0x1000) {
            return chrRom[addr + chrBank0 * 0x1000];
        } else if (addr < 0x2000) {
            return chrRom[addr - 0x1000 + chrBank1 * 0x1000];
        }
    }
    return 0x00;
}

void cart001::writeMemoryPPU(u16 addr, u8 v) {
}
