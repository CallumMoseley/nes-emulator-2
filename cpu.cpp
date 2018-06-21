#include "cpu.h"

public void cpu::op() {
    char opcode = readMemory(++pc);

    // opcodes have a form 0b aaabbbcc
    // where aaa and cc determine opcode and bbb determines addressing mode

    switch (opcode & 0b11) {
        case 0b01: // first block of instructions
            short addr = 0;
            switch ((opcode & 0b11100) >> 2) {
                case 0b000: // (zp, x)
                    addr = indirxAddr();
                    break;
                case 0b001: // zp
                    addr = zpAddr();
                    break;
                case 0b010: // immediate
                    addr = immAddr();
                    break;
                case 0b011: // abs
                    addr = absAddr();
                    break;
                case 0b100: // (zp), y
                    addr = indiryAddr();
                    break;
                case 0b101: // zp, x
                    addr = zpxAddr();
                    break;
                case 0b110: // abs, y
                    addr = absyAddr();
                    break;
                case 0b111: // abs, x
                    addr = absxAddr();
                    break;
            }
            switch ((opcode & 0b11100000) >> 5) {
                case 0b000: // ORA
                    a |= readMemory(addr);
                    setZ(a);
                    setN(a);
                    break;
                case 0b001: // AND
                    a &= readMemory(addr);
                    setZ(a);
                    setN(a);
                    break;
                case 0b010: // EOR
                    a ^= readMemory(addr);
                    setZ(a);
                    setN(a);
                    break;
                case 0b011: // ADC
                    char m = readMemory(addr);
                    char oldCarry = p.c;
                    p.c = 0;
                    if ((int) (unsigned char) a + (int) (unsigned char) m > 255) p.c = 1;
                    
                    char n = a;
                    a = a + m + oldCarry;
                    p.v = 0;
                    if ((n^a) & (m^a) & 0x80 != 0) p.v = 1;
                    setZ(a);
                    setN(a);
                    break;
                case 0b100: // STA
                    writeMemory(addr, a);
                    break;
                case 0b101: // LDA
                    a = readMemory(addr);
                    setZ(a);
                    setN(a);
                    break;
                case 0b110: // CMP
                    char m = readMemory(addr);
                    p.c = 0;
                    if (a >= m) p.c = 1;
                    setZ(a - m);
                    setN(a - m);
                    break;
                case 0b111: // SBC
                    char m = readMemory(addr);
                    char n = a;
                    char oldBorrow = 1 - p.c;
                    p.c = 1;
                    if ((int) a < (int) m + (int) oldBorrow) p.c = 0;
                    a = a - m - oldBorrow;
                    p.v = 0;
                    if ((n^a) & ((~m)^a) & 0x80 != 0) p.v = 1;
                    setZ(a);
                    setN(a);
                    break;
            }
            break;
        case 0b10:
            short addr = 0;
            switch ((opcode & 0b11100) >> 2) {
                case 0b000: // immediate
                    addr = immAddr();
                    break;
                case 0b001: // zp
                    addr = zpAddr();
                    break;
                case 0b010: // accumulator
                    break;
                case 0b011: // abs
                    addr = absAddr();
                    break;
                case 0b101: // zp, x
                    if (opcode == 0x96 || opcode == 0xB6) {
                        addr = zpyAddr();
                    } else {
                        addr = zpxAddr();
                    }
                    break;
                case 0b111: // abs, x
                    if (opcode == 0xBE) {
                        addr = absyAddr();
                    } else {
                        addr = absxAddr();
                    }
                    break;
            }
            switch ((opcode & 0b11100000) >> 5) {
                
            }
            break;
    }
}

// Op code addressing modes
// Each returns the address in memory to look at

private ushort cpu::immAddr() {
    return ++pc;
}

private ushort zpAddr() {
    return (ushort) (readMemory(++pc));
}

private ushort zpxAddr() {
    char zpAddr = readMemory(++pc);
    return (ushort) (zpAddr + x);
}

private ushort zpyAddr() {
    char zpAddr = readMemory(++pc);
    return (ushort) (zpAddr + y);
}

private ushort absAddr() {
    ushort lsb = readMemory(++pc);
    ushort msb = readMemory(++pc);
    return (ushort) ((msb << 8 ) | lsb);
}

private ushort absxAddr() {
    return (ushort) (absAddr() + x);
}

private ushort absyAddr() {
    return (ushort) (absAddr() + y);
}

private ushort indirxAddr() {
    ushort zpAddr = readMemory(++pc);
    ushort lsb = readMemory(zpAddr + x);
    ushort msb = readMemory(zpAddr + x + 1);
    return (ushort) ((msb << 8) | lsb);
}

private ushort indiryAddr() {
    char zpAddr = readMemory(++pc);
    ushort lsb = readMemory(zpAddr);
    ushort msb = readMemory(zpAddr + 1);
    return (ushort) (((msb << 8) | lsb) + y);
}

// Flag setting utilities


