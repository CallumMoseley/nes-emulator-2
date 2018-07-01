#ifndef PPU_H
#define PPU_H

#include "cart.h"

typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

class PPU {
private:

    bool inVblank;
    bool s0hit;
    bool sprOverflow;

    bool grey;

    bool bgLeft, sprLeft;
    bool bgEn, sprEn;

    bool nmiEnabled;
    bool largeSprites;

    bool boostR, boostG, boostB;

    u8 bgPatternTable;
    u8 sprPatternTable;
    u8 vramInc;
    u8 ntSelect;

    u8* oam;
    u8* oam2;
    u8* vram;
    u8* paletteRam;

    bool scrollLatch;
    u8 scrollX;
    u8 scrollY;

    bool addrLatch;

    u8 oamAddr;
    u16 vramAddr;

    cart* gameCart;

    u8 readMemory(u16 addr);
    void writeMemory(u16 addr, u8 v);
public:

    PPU();

    void powerOn();
    void reset();

    void tick();
    void writeRegister(u16 addr, u8 v);
    u8 readRegister(u16 addr);

    void writeOam(u8 v);
};

#endif
