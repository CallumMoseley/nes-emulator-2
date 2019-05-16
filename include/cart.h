#ifndef CART_H
#define CART_H

#include <cstdio>
#include <cstring>
#include <fstream>
#include "types.h"

class cart {
public:
    bool vMirror;

    static cart* fromFile(char filename[]);

    cart();
    virtual ~cart();

    virtual u8 readMemoryCPU(u16 addr) = 0;
    virtual void writeMemoryCPU(u16 addr, u8 v) = 0;

    virtual u8 readMemoryPPU(u16 addr) = 0;
    virtual void writeMemoryPPU(u16 addr, u8 v) = 0;
};

class cart000 : public cart {
public:
    cart000(int prgBanks, int chrBanks, u8* prg, u8* chr, bool v);
    ~cart000();

    virtual u8 readMemoryCPU(u16 addr);
    virtual void writeMemoryCPU(u16 addr, u8 v);

    virtual u8 readMemoryPPU(u16 addr);
    virtual void writeMemoryPPU(u16 addr, u8 v);
private:
    int prgBanks;
    int chrBanks;

    u8* prg;
    u8* chr;
};

class cart001 : public cart {
public:
    cart001(int prgRamSize, int prgBanks, int chrBanks, u8* prg, u8* chr);
    ~cart001();

    virtual u8 readMemoryCPU(u16 addr);
    virtual void writeMemoryCPU(u16 addr, u8 v);

    virtual u8 readMemoryPPU(u16 addr);
    virtual void writeMemoryPPU(u16 addr, u8 v);
private:
    int prgBanks;
    int chrBanks;

    u8* prgRam; // 0x6000-0x7FFF
    u8* prgRom; // 0x8000-0xBFFF and 0xC000-0xFFFF switchable banks
    u8* chrRom; // 0x0000-0x1FFF 1 or 2 switchable banks

    u8 chrBank0;
    u8 chrBank1;

    u8 prgBank;

    u8 mirroring;
    u8 prgRomMode;
    u8 chrRomMode;

    u8 prgRamEnable;

    u8 sr;
};

class allsuite_cart : public cart {
public:
    allsuite_cart();
    ~allsuite_cart();

    virtual u8 readMemoryCPU(u16 addr);
    virtual void writeMemoryCPU(u16 addr, u8 v);

    virtual u8 readMemoryPPU(u16 addr);
    virtual void writeMemoryPPU(u16 addr, u8 v);

private:
    u16 memSize;
    u8* memory;
};

#endif
