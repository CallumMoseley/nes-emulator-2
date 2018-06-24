#ifndef CPU_H
#define CPU_H

#include <ppu.h>

#define ushort unsigned short

struct status {
    unsigned char c : 1;
    unsigned char z : 1;
    unsigned char i : 1;
    unsigned char d : 1;
    unsigned char s : 2;
    unsigned char v : 1;
    unsigned char n : 1;
};

class cpu {

    private ppu ppu;

    private char a, x, y, s;
    private ushort pc;

    private status p;

    private char* ram;

    public bool nmiLow;
    public bool lastNmiLow;
    public bool nmiEdgeDetected;
    public bool nmiActive;

    public bool irqLow;
    public bool irqLevelDetected
    public bool irqActive;
    
    public cpu();
    public ~cpu();

    public void powerOn();
    
    public void op();

    public char readMemory(ushort addr);
    public void writeMemory(ushort addr, char v);

    private void tick(int n);
    private void tick();

    // Addressing mode tools
    private ushort immAddr();
    private ushort zpAddr();
    private ushort zpxAddr();
    private ushort zpyAddr();
    private ushort absAddr();
    private ushort absxAddr(char opcode);
    private ushort absyAddr(char opcode);
    private ushort indirxAddr();
    private ushort indiryAddr();

    private void setZN(char val);
};

#endif
