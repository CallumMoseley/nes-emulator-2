#ifndef PPU_H
#define PPU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include "cart.h"

typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

class PPU {
private:

    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    uint32_t* pixels = nullptr;
    int pitch = 0;

    bool s0hit = false;
    bool sprOverflow = false;

    bool grey = false;

    bool bgLeft = false, sprLeft = false;
    bool bgEn = false, sprEn = false;

    bool nmiEnabled = false;
    bool nmiOccurred = false;
    bool largeSprites = false;

    bool boostR = false, boostG = false, boostB = false;

    u8 bgPatternTable = 0;
    u8 sprPatternTable = 0;
    u8 vramInc = 1;

    u8* oam = nullptr;
    u8* oam2 = nullptr;
    u8* vram = nullptr;
    u8* paletteRam = nullptr;

    bool w = false;
    u8 fineX = 0;

    u8 oamAddr = 0x00;
    u16 vramAddr = 0x00;
    u16 tempAddr = 0x00;

    u16 dot = 0;
    u16 scanline = 0;

    u8 lowBGShift1 = 0;
    u8 highBGShift1 = 0;
    u8 lowBGShift2 = 0;
    u8 highBGShift2 = 0;
    u8 attr = 0;

    int frameCount = 0;

    void fillOam2();

    u8 readMemory(u16 addr);
    void writeMemory(u16 addr, u8 v);

    void incAddrX(u16 &addr);
    void incAddrY(u16 &addr);
    void shiftRegisters();
    void loadTile();

    void renderPixel(u8 colour);
    void initializeMemory();

    uint32_t colours[64] = {
        0xff545454,
        0xff001e74,
        0xff081090,
        0xff300088,
        0xff440064,
        0xff5c0030,
        0xff540400,
        0xff3c1800,
        0xff202a00,
        0xff083a00,
        0xff004000,
        0xff003c00,
        0xff00323c,
        0xff000000,
        0xff000000,
        0xff000000,
        0xff989698,
        0xff084cc4,
        0xff3032ec,
        0xff5c1ee4,
        0xff8814b0,
        0xffa01464,
        0xff982220,
        0xff783c00,
        0xff545a00,
        0xff287200,
        0xff087c00,
        0xff007628,
        0xff006678,
        0xff000000,
        0xff000000,
        0xff000000,
        0xffeceeec,
        0xff4c9aec,
        0xff787cec,
        0xffb062ec,
        0xffe454ec,
        0xffec58b4,
        0xffec6a64,
        0xffd48820,
        0xffa0aa00,
        0xff74c400,
        0xff4cd020,
        0xff38cc6c,
        0xff38b4cc,
        0xff3c3c3c,
        0xff000000,
        0xff000000,
        0xffeceeec,
        0xffa8ccec,
        0xffbcbcec,
        0xffd4b2ec,
        0xffecaeec,
        0xffecaed4,
        0xffecb4b0,
        0xffe4c490,
        0xffccd278,
        0xffb4de78,
        0xffa8e290,
        0xff98e2b4,
        0xffa0d6e4,
        0xffa0a2a0,
        0xff000000,
        0xff000000
    };

public:
    cart* gameCart = nullptr;

    PPU(SDL_Renderer* renderer);
    ~PPU();

    void powerOn();
    void reset();

    void tick();
    void writeRegister(u16 addr, u8 v);
    u8 readRegister(u16 addr);

    void writeOam(u8 v);
    bool nmiLow();


    // Debug stuff

    void printNametable(u16 addr);
    void printPattern(u8 table, u8 pattern);
};

#endif
