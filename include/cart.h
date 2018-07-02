#ifndef CART_H
#define CART_H

typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

class cart {
public:
    cart();
    virtual ~cart();

    virtual u8 readMemoryCPU(u16 addr) = 0;
    virtual void writeMemoryCPU(u16 addr, u8 v) = 0;

    virtual u8 readMemoryPPU(u16 addr) = 0;
    virtual void writeMemoryPPU(u16 addr, u8 v) = 0;
};

class cart00 {

}

class allsuite_cart : public cart {
public:
    virtual u8 readMemoryCPU(u16 addr);
    virtual void writeMemoryCPU(u16 addr, u8 v);

    virtual u8 readMemoryPPU(u16 addr);
    virtual void writeMemoryPPU(u16 addr, u8 v);

    allsuite_cart();

private:
    u16 memSize;
    u8* memory;
};

#endif
