#ifndef CART_H
#define CART_H

typedef unsigned short u16;
typedef unsigned char u8;
typedef signed char s8;

class cart {
public:
    cart();
    virtual ~cart();

    virtual u8 readMemory(u16 addr) = 0;
    virtual void writeMemory(u16 addr, u8 v) = 0;
};

class allsuite_cart : public cart {
public:
    virtual u8 readMemory(u16 addr);
    virtual void writeMemory(u16 addr, u8 v);
    allsuite_cart();

private:
    u16 memSize;
    u8* memory;
};

#endif
