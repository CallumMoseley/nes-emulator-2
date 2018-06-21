#include <ppu.h>

#define ushort unsigned short

private struct status {
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

    private char a, x, y;
    private ushort pc;

    private status p;

    private char* ram;
    
    public cpu();
    
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
    private ushort absxAddr();
    private ushort absyAddr();
    private ushort indirxAddr();
    private ushort indiryAddr();

    private void setZ(char val);
    private void setN(char val);
};
