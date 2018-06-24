#include "cpu.h"

public cpu::cpu() {
    ram = new char[0x200];
}

public cpu::~cpu() {
    delete[] ram;

    nmiLow = false;
    lastNmiLow = false;
    nmiEdgeDetected = false;
    nmiActive = false;

    irqLow = false;
    irqLevelDetected = false;
    irqActive = false;
}

public void cpu::powerOn() {
    *((char*) &p) = 0x34;

    a = 0x00;
    x = 0x00;
    y = 0x00;

    s = 0xFD;

    nmiLow = false;
    lastNmiLow = false;

    for (int i = 0; i < 0x200; i++) {
        ram[i] = 0x00;
    }
}

public void cpu::reset() {
    pc
}

public char cpu::readMemory(ushort addr) {
    char mem = 0x00;
    if (addr < 0x200) {
        mem = ram[addr];
    }
    tick();
    return mem;
}

public void cpu::writeMemory(ushort addr, char v) {
    if (addr < 0x200) {
        ram[addr] = v;
    }
    tick();
}

public void cpu::tick() {
    for (int i = 0; i < 3; i++) {
        ppu.tick();
    }

    irqActive = false;

    if (irqLevelDetected) {
        irqActive = true;
        irqLevelDetected = false;
    }

    if (irqLow) {
        irqLevelDetected = true;
    }

    if (nmiEdgeDetected) {
        nmiActive = true;
        nmiEdgeDetected = false;
    }

    if (nmiLow && !lastNmiLow) {
        nmiEdgeDetected = true;
    }

    lastNmiLow = nmiLow;
}

public void cpu::tick(int n) {
    for (int i = 0; i < n; i++) {
        tick();
    }
}

public void cpu::op() {
    if (nmiActive) {
        readMemory(pc);
        writeMemory(0x100 | s--, pc >> 8);
        writeMemory(0x100 | s--, pc & 0xFF);

        char b = *((char*) &p);
        b |= 0b00110000;
        writeMemory(0x100 | s--, b);

        nmiActive = false;

        char lsb = readMemory(0xFFFA);
        char msb = readMemory(0xFFFB + 1);

        pc = (msb << 8) | lsb;
        
        return;
    } else if (irqActive) {
        readMemory(pc++);
        writeMemory(0x100 | s--, pc >> 8);
        writeMemory(0x100 | s--, pc & 0xFF);

        ushort intVector;
        if (nmiActive) {
            intVector = 0xFFFA;
            nmiActive = false;
        } else {
            intVector = 0xFFFE;
        }

        char b = *((char*) &p);
        b |= 0b00110000;
        writeMemory(0x100 | s--, b);

        char lsb = readMemory(intVector);
        char msb = readMemory(intVector + 1);

        pc = (msb << 8) | lsb;       
        return;
    }

    char opcode = readMemory(pc++);

    // opcodes have a form 0b aaabbbcc
    // where aaa and cc determine opcode and bbb determines addressing mode

    switch (opcode & 0b11) {
    case 0b01: // first block of instructions
        ushort addr = 0;
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
            addr = absyAddr(opcode);
            break;
        case 0b111: // abs, x
            addr = absxAddr(opcode);
            break;
        }
        switch (opcode >> 5) {
        case 0b000: // ORA
            a |= readMemory(addr);
            setZN(a);
            break;
        case 0b001: // AND
            a &= readMemory(addr);
            setZN(a);
            break;
        case 0b010: // EOR
            a ^= readMemory(addr);
            setZN(a);
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
            setZN(a);
            break;
        case 0b100: // STA
            writeMemory(addr, a);
            break;
        case 0b101: // LDA
            a = readMemory(addr);
            setZN(a);
            break;
        case 0b110: // CMP
            compare(addr, a);
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
            setZN(a);
            break;
        }
        break;
    case 0b10:
        ushort addr = 0;
        bool addrGot = true;
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
                addr = absyAddr(opcode);
            } else {
                addr = absxAddr(opcode);
            }
            break;
        default:
            addrGot = false;
            break;
        }
        if (addrGot) {
            switch (opcode >> 5) {
            case 0b000: // ASL
                if (opcode == 0x0A) {
                    p.c = a >> 7;
                    a <<= 1;
                    setZN(a);
                } else {
                    char m = readMemory(addr);
                    p.c = m >> 7;
                    m <<= 1;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b001: // ROL
                if (opcode == 0x2A) {
                    char old = p.c;
                    p.c = a >> 7;
                    a = (a << 1) | old;
                    setZN(a);
                } else {
                    char m = readMemory(addr);
                    char old = p.c;
                    p.c = m >> 7;
                    m = (m << 1) | old;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b010: // ASL
                if (opcode == 0x4A) {
                    p.c = a & 0x01;
                    a >>= 1;
                    setZN(a);
                } else {
                    char m = readMemory(addr);
                    p.c = m & 0x01;
                    m >>= 1;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b011: // ROR
                if (opcode == 0x6A) {
                    char old = p.c;
                    p.c = a & 0x01;
                    a = (a >> 1) | (old << 7);
                    setZN(a);
                } else {
                    char m = readMemory(addr);
                    char old = p.c;
                    p.c = m & 0x01;
                    m = (m >> 1) | (old << 7);
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b100: // STX
                writeMemory(addr, x);
                break;
            case 0b101: // LDX
                x = readMemory(addr);
                setZN(x);
                break;
            case 0b110: // DEC
                char m = readMemory(addr);
                tick();
                writeMemory(addr, m - 1);
                setZN(m - 1);
                break;
            case 0b111: // INC
                char m = readmemory(addr);
                tick();
                writememory(addr, m + 1);
                setZN(m + 1);
            }
        } else {
            switch (opcode) {
            case 0x8A: // TXA
                a = x;
                setZN(a);
                tick();
                break;
            case 0x9A: // TXS
                s = x;
                break;
            case 0xAA: // TAX
                x = a;
                setZN(x);
                break;
            case 0xBA: // TSX
                x = s;
                setZN(x);
                break;
            case 0xCA: // DEX
                tick();
                x--;
                setZN(x);
                break;
            case 0xEA: // NOP
                tick();
                break;
            }
        }
        break;
    case 0b00:
        bool opcodeComplete = true;
        switch (opcode) {
        case 0x00: // BRK
            readMemory(pc++);
            writeMemory(0x100 | s--, pc >> 8);
            writeMemory(0x100 | s--, pc & 0xFF);

            ushort intVector;
            if (nmiActive) {
                intVector = 0xFFFA;
                nmiActive = false;
            } else {
                intVector = 0xFFFE;
            }

            char b = *((char*) &p);
            b |= 0b00110000;
            writeMemory(0x100 | s--, b);

            char lsb = readMemory(intVector);
            char msb = readMemory(intVector + 1);

            pc = (msb << 8) | lsb;
            break;
        case 0x20: // JSR
            char msb = readMemory(pc++);
            tick();
            writeMemory(0x100 | s--, pc >> 8);
            writeMemory(0x100 | s--, pc & 0xFF);
            char lsb = readMemory(pc++);
            pc = (msb << 8) | lsb;
            break;
        case 0x40: // RTI
            readMemory(pc++);
            
            p = *((status*) &readMemory(0x100 | ++s));
            char lsb = readMemory(0x100 | ++s);
            char msb = readMemory(0x100 | ++s);
            tick();

            pc = (msb << 8) | lsb;

            break;
        case 0x60: // RTS
            tick();
            char lsb = readMemory(0x100 | ++s);
            char msb = readMemory(0x100 | ++s);
            tick();
            pc = ((msb << 8) | lsb) + 1;
            break;
        case 0x08: // PHP
            char b = *((char*) &p);
            b |= 0b00110000;
            tick();
            writeMemory(0x100 | s--, b);
            break;
        case 0x18: // CLC
            tick();
            p.c = 0;
            break;
        case 0x28: // PLP
            tick(2);
            char b = readMemory(0x100 | ++s);
            p = *((status*) &b);
            break;
        case 0x38: // SEC
            tick();
            p.c = 1;
            break;
        case 0x48: // PHA
            tick();
            writeMemory(0x100 | s--, a);
            break;
        case 0x58: // CLI
            tick();
            p.i = 0;
            break;
        case 0x68: // PLA
            tick(2);
            a = readMemory(0x100 | ++s);
            break;
        case 0x78: // SEI
            tick();
            p.i = 1;
            break;
        case 0x88: // DEY
            tick();
            y--;
            setZN(y);
            break;
        case 0x98: // TYA
            a = y;
            setZN(a);
            tick();
            break;
        case 0xA8: // TAY
            y = a;
            setZN(y);
            tick();
            break;
        case 0xB8: // CLV
            tick();
            p.v = 0;
            break;
        case 0xC8: // INY
            tick();
            y++;
            setZN(y);
            break;
        case 0xD8: // CLD
            tick();
            p.d = 0;
            break;
        case 0xE8: // INX
            tick();
            x++;
            setZN(x);
            break;
        case 0xF8: // SED
            tick();
            p.d = 1;
            break;
        default:
            opcodeComplete = false;
            break;
        }
        if (!opcodeComplete) {
            ushort addr = 0;
            bool branch = false;
            switch ((opcode & 0b11100) >> 2) {
            case 0b000:
                addr = immAddr();
                break;
            case 0b001:
                addr = zpAddr();
                break;
            case 0b011:
                addr = absAddr();
                break;
            case 0b101:
                addr = zpxAddr();
                break;
            case 0b111:
                addr = absxAddr(opcode);
                break;
            case 0b100: // Branch instructions
                branch = true;
                break;
            default:
                addrFound = false;
                break;
            }
            if (branch) {
                char flag = 0;
                char test = (opcode >> 5) & 0x01;
                switch (opcode >> 6) {
                case 0b00: // BMI, BPL
                    flag = p.n;
                    break;
                case 0b01: // BVS, BVC
                    flag = p.v;
                    break;
                case 0b10: // BCS, BCC 
                    flag = p.c;
                    break;
                case 0b11: // BEQ, BNE
                    glaf = p.z;
                    break;
                }
                branch(flag, test);
            } else if (addrFound) {
                switch (opcode >> 5) {
                case 0b001: // BIT
                    char res = a & readMemory(addr);
                    setZN(res);
                    p.v = (res >> 6) & 0x01;
                    break;
                case 0b010: // JMP
                    pc = addr;
                    break;
                case 0b011: // JMP()
                    char lsb = readMemory(addr);
                    char msb = readMemory(addr + 1);
                    pc = ((msb << 8) | lsb);
                    break;
                case 0b100: // STY
                    writeMemory(addr, y);
                    break;
                case 0b101: // LDY
                    y = readMemory(addr);
                    setZN(y);
                    break;
                case 0b110: // CPY
                    compare(addr, y);
                    break;
                case 0b111: // CPX
                    compare(addr, x);
                    break;
                }
            }   
        }
    }
}

private void cpu::branch(char flag, char test) {
    char offset = readMemory(pc++);
    if (flag == test) {
        tick();
        ushort page = pc & 0xFF00;
        pc += offset;
        if (pc & 0xFF00 != page) { // branch caused new page
            tick();
            pc = page | (pc & 0xFF);
        }
    }
}

private void cpu::compare(ushort addr, char reg) {
    char m = readMemory(addr);
    p.c = 0;
    if (reg >= m) p.c = 1;
    setZN(reg - m);
}

// Op code addressing modes
// Each returns the address in memory to look at

private ushort cpu::immAddr() {
    return pc++;
}

private ushort cpu::zpAddr() {
    return readMemory(pc++);
}

private ushort cpu::zpxAddr() {
    char zpAddr = readMemory(pc++);
    tick();
    return (zpAddr + x) & 0xFF;
}

private ushort cpu::zpyAddr() {
    char zpAddr = readMemory(pc++);
    return (zpAddr + y) & 0xFF;
}

private ushort cpu::absAddr() {
    ushort lsb = readMemory(pc++);
    ushort msb = readMemory(pc++);
    return (msb << 8) | lsb;
}

private ushort cpu::absxAddr(char opcode) {
    ushort base = absAddr();
    ushort result = base + x;
    if (opcode & 0xF == 0xE ||
        opcode == 0x9D ||
        (result ^ base) & 0xFF00 != 0) {
        tick();
    }
    return result;
}

private ushort cpu::absyAddr(char opcode) {
    ushort base = absAddr();
    ushort result = base + y;
    if (opcode == 0x99 || (result ^ base) & 0xFF00 != 0) {
        tick();
    }
    return result;
}

private ushort cpu::indirxAddr() {
    ushort zpAddr = readMemory(pc++);
    ushort lsb = readMemory(zpAddr + x);
    ushort msb = readMemory(zpAddr + x + 1);
    return (ushort) ((msb << 8) | lsb);
}

private ushort cpu::indiryAddr() {
    char zpAddr = readMemory(pc++);
    ushort lsb = readMemory(zpAddr);
    ushort msb = readMemory(zpAddr + 1);
    return (ushort) (((msb << 8) | lsb) + y);
}

// Flag setting utilities

private void cpu::setZN(char val) {
    p.z = 0;
    if (val == 0) p.z = 1;
    p.n = val >> 7;
}
