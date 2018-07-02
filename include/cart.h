#ifndef CART_H
#define CART_H

#include <cstdio>
#include <fstream>

typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

class cart {
public:
    static cart* fromFile(char* filename);

    cart();
    virtual ~cart();

    virtual u8 readMemoryCPU(u16 addr) = 0;
    virtual void writeMemoryCPU(u16 addr, u8 v) = 0;

    virtual u8 readMemoryPPU(u16 addr) = 0;
    virtual void writeMemoryPPU(u16 addr, u8 v) = 0;
};

class cart000 : public cart {
public:
    bool vMirror;

    cart000(int prgBanks, int chrBanks, u8* prg, u8* chr);
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
