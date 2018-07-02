#include <cstdio>
#include <unistd.h>
#include "CPU.h"

CPU::CPU(PPU* ppu) {
    this->ppu = ppu;
    ram = new u8[0x800];

    lastNmiLow = false;
    nmiEdgeDetected = false;
    nmiActive = false;

    irqLow = false;
    irqLevelDetected = false;
    irqActive = false;
}

CPU::~CPU() {
    delete[] ram;
}

void CPU::powerOn() {
    tickCount = 0;
    lastTickCount = 0;

    *((u8*) &p) = 0x34;

    a = 0x00;
    x = 0x00;
    y = 0x00;

    s = 0xFD;

    lastNmiLow = false;

    for (int i = 0; i < 0x800; i++) {
        ram[i] = 0x00;
    }

    u8 lsb = readMemory(0xFFFC);
    u8 msb = readMemory(0xFFFD);

    pc = (msb << 8) | lsb;
}

void CPU::reset() {
    tickCount = 0;
    lastTickCount = 0;

    s -= 3;
    p.i = 1;

    u8 lsb = readMemory(0xFFFC);
    u8 msb = readMemory(0xFFFD);

    pc = (msb << 8) | lsb;
}

u8 CPU::readMemory(u16 addr) {
    u8 mem = 0x00;
    if (addr < 0x2000) {
        mem = ram[addr & 0x7FF];
    } else if (addr < 0x4000) {
        mem = ppu->readRegister(addr);
    } else if (addr >= 0x4020) {
        mem = gameCart->readMemoryCPU(addr);
    }
    tick();
    return mem;
}

void CPU::writeMemory(u16 addr, u8 v) {
    if (addr < 0x2000) {
        ram[addr & 0x7FF] = v;
    } else if (addr < 0x4000) {
        ppu->writeRegister(addr, v);
    } else if (addr == 0x4014) {
        tick(2);
        if (tickCount % 2 == 1) {
            tick();
        }
        
        u16 tempAddr = v << 8;

        for (int i = 0; i < 256; i++) {
            u8 val = readMemory(tempAddr++);
            ppu->writeOam(val);
            tick();
        }
    } else if (addr >= 0x4020) {
        gameCart->writeMemoryCPU(addr, v);
    }
    tick();
}

void CPU::tick() {
    tickCount++;
    for (int i = 0; i < 3; i++) {
        ppu->tick();
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

    if (ppu->nmiLow() && !lastNmiLow) {
        nmiEdgeDetected = true;
    }

    lastNmiLow = ppu->nmiLow();
}

void CPU::tick(int n) {
    for (int i = 0; i < n; i++) {
        tick();
    }
}

void CPU::op() {
    if (nmiActive) {
        readMemory(pc);
        writeMemory(0x100 | s--, pc >> 8);
        writeMemory(0x100 | s--, pc & 0xFF);

        u8 b = *((u8*) &p);
        b |= 0b00110000;
        writeMemory(0x100 | s--, b);

        nmiActive = false;

        u8 lsb = readMemory(0xFFFA);
        u8 msb = readMemory(0xFFFB);

        pc = (msb << 8) | lsb;
        
        return;
    } else if (irqActive) {
        readMemory(pc++);
        writeMemory(0x100 | s--, pc >> 8);
        writeMemory(0x100 | s--, pc & 0xFF);

        u16 intVector;
        if (nmiActive) {
            intVector = 0xFFFA;
            nmiActive = false;
        } else {
            intVector = 0xFFFE;
        }

        u8 b = *((u8*) &p);
        b |= 0b00110000;
        writeMemory(0x100 | s--, b);

        u8 lsb = readMemory(intVector);
        u8 msb = readMemory(intVector + 1);

        pc = (msb << 8) | lsb;       
        return;
    }

    lastTickCount = tickCount;

    if (pc == 0xC85F) {
        printf("break\n");
    }

    u8 opcode = readMemory(pc++);
    printf("Op: %s\n", opcodeNames[opcode]);

    // declare a bunch of stuff up here just for convenience (and because it's reused over cases)
    
    u16 addr = 0;
    bool opcodeComplete = true;
    u8 msb = 0;
    u8 lsb = 0;

    u8 m = 0;
    u8 n = 0;
    u8 b = 0;

    u8 oldCarry = 0;

    // opcodes have a form 0b aaabbbcc
    // where aaa and cc determine opcode and bbb determines addressing mode

    switch (opcode & 0b11) {
    case 0b01: // first block of instructions
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
            addr = indiryAddr(opcode);
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
            m = readMemory(addr);
            oldCarry = p.c;
            p.c = 0;
            if ((int) (u8) a + (int) (u8) m > 255) p.c = 1;
            
            n = a;
            a = a + m + oldCarry;
            p.v = 0;
            if (((n^a) & (m^a) & 0x80) != 0) p.v = 1;
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
            m = readMemory(addr);
            n = a;
            oldCarry = 1 - p.c;
            p.c = 1;
            if ((int) a < (int) m + (int) oldCarry) p.c = 0;
            a = a - m - oldCarry;
            p.v = 0;
            if (((n^a) & ((~m)^a) & 0x80) != 0) p.v = 1;
            setZN(a);
            break;
        }
        break;
    case 0b10:
        opcodeComplete = true;
        switch (opcode) {
            case 0x8A: // TXA
                a = x;
                setZN(a);
                tick();
                break;
            case 0x9A: // TXS
                s = x;
                tick();
                break;
            case 0xAA: // TAX
                x = a;
                setZN(x);
                tick();
                break;
            case 0xBA: // TSX
                x = s;
                setZN(x);
                tick();
                break;
            case 0xCA: // DEX
                tick();
                x--;
                setZN(x);
                break;
            case 0xEA: // NOP
                tick();
                break;
            default:
                opcodeComplete = false;
                break;
            }
            if (!opcodeComplete) {
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
            }
            switch (opcode >> 5) {
            case 0b000: // ASL
                if (opcode == 0x0A) {
                    tick();
                    p.c = a >> 7;
                    a <<= 1;
                    setZN(a);
                } else {
                    m = readMemory(addr);
                    tick();
                    p.c = m >> 7;
                    m <<= 1;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b001: // ROL
                if (opcode == 0x2A) {
                    tick();
                    oldCarry = p.c;
                    p.c = a >> 7;
                    a = (a << 1) | oldCarry;
                    setZN(a);
                } else {
                    m = readMemory(addr);
                    tick();
                    oldCarry = p.c;
                    p.c = m >> 7;
                    m = (m << 1) | oldCarry;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b010: // ASL
                if (opcode == 0x4A) {
                    tick();
                    p.c = a & 0x01;
                    a >>= 1;
                    setZN(a);
                } else {
                    m = readMemory(addr);
                    tick();
                    p.c = m & 0x01;
                    m >>= 1;
                    writeMemory(addr, m);
                    setZN(m);
                }
                break;
            case 0b011: // ROR
                if (opcode == 0x6A) {
                    tick();
                    oldCarry = p.c;
                    p.c = a & 0x01;
                    a = (a >> 1) | (oldCarry << 7);
                    setZN(a);
                } else {
                    m = readMemory(addr);
                    tick();
                    oldCarry = p.c;
                    p.c = m & 0x01;
                    m = (m >> 1) | (oldCarry << 7);
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
                m = readMemory(addr);
                tick();
                writeMemory(addr, m - 1);
                setZN(m - 1);
                break;
            case 0b111: // INC
                m = readMemory(addr);
                tick();
                writeMemory(addr, m + 1);
                setZN(m + 1);
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

            u16 intVector;
            if (nmiActive) {
                intVector = 0xFFFA;
                nmiActive = false;
            } else {
                intVector = 0xFFFE;
            }

            b = *((u8*) &p);
            b |= 0b00110000;
            writeMemory(0x100 | s--, b);

            lsb = readMemory(intVector);
            msb = readMemory(intVector + 1);

            pc = (msb << 8) | lsb;
            break;
        case 0x20: // JSR
            lsb = readMemory(pc++);
            tick();
            writeMemory(0x100 | s--, pc >> 8);
            writeMemory(0x100 | s--, pc & 0xFF);
            msb = readMemory(pc++);
            pc = (msb << 8) | lsb;
            break;
        case 0x40: // RTI
            readMemory(pc++);
            
            b = readMemory(0x100 | ++s);
            p = *((status*) &b);
            lsb = readMemory(0x100 | ++s);
            msb = readMemory(0x100 | ++s);
            tick();

            pc = (msb << 8) | lsb;

            break;
        case 0x60: // RTS
            tick();
            lsb = readMemory(0x100 | ++s);
            msb = readMemory(0x100 | ++s);
            tick();
            pc = ((msb << 8) | lsb) + 1;
            break;
        case 0x08: // PHP
            b = *((u8*) &p);
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
            b = readMemory(0x100 | ++s);
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
            bool isBranch = false;
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
                isBranch = true;
                break;
            }
            if (isBranch) {
                u8 flag = 0;
                u8 test = (opcode >> 5) & 0x01;
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
                    flag = p.z;
                    break;
                }
                branch(flag, test);
            } else {
                switch (opcode >> 5) {
                case 0b001: // BIT
                    m = a & readMemory(addr);
                    setZN(m);
                    p.v = (m >> 6) & 0x01;
                    break;
                case 0b010: // JMP
                    pc = addr;
                    break;
                case 0b011: // JMP()
                    lsb = readMemory(addr);
                    msb = readMemory(addr + 1);
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

void CPU::branch(u8 flag, u8 test) {
    u8 offset = readMemory(pc++);
    if (flag == test) {
        tick();
        u16 page = pc & 0xFF00;
        pc += (s8) offset;
        if ((pc & 0xFF00) != page) { // branch caused new page
            tick();
            pc = page | (pc & 0xFF);
        }
    }
}

void CPU::compare(u16 addr, u8 reg) {
    u8 m = readMemory(addr);
    p.c = 0;
    if (reg >= m) p.c = 1;
    setZN(reg - m);
}

// Op code addressing modes
// Each returns the address in memory to look at

u16 CPU::immAddr() {
    return pc++;
}

u16 CPU::zpAddr() {
    return readMemory(pc++);
}

u16 CPU::zpxAddr() {
    u8 zpAddr = readMemory(pc++);
    tick();
    return (zpAddr + x) & 0xFF;
}

u16 CPU::zpyAddr() {
    u8 zpAddr = readMemory(pc++);
    return (zpAddr + y) & 0xFF;
}

u16 CPU::absAddr() {
    u8 lsb = readMemory(pc++);
    u8 msb = readMemory(pc++);
    return (msb << 8) | lsb;
}

u16 CPU::absxAddr(u8 opcode) {
    u16 base = absAddr();
    u16 result = base + x;
    if ((opcode & 0xF) == 0xE ||
        opcode == 0x9D ||
        ((result ^ base) & 0xFF00) != 0) {
        tick();
    }
    return result;
}

u16 CPU::absyAddr(u8 opcode) {
    u16 base = absAddr();
    u16 result = base + y;
    if (opcode == 0x99 || ((result ^ base) & 0xFF00) != 0) {
        tick();
    }
    return result;
}

u16 CPU::indirxAddr() {
    u16 zpAddr = readMemory(pc++);
    tick();
    u8 lsb = readMemory(zpAddr + x);
    u8 msb = readMemory(zpAddr + x + 1);
    return (u16) ((msb << 8) | lsb);
}

u16 CPU::indiryAddr(u8 opcode) {
    u16 zpAddr = readMemory(pc++);
    u8 lsb = readMemory(zpAddr);
    u8 msb = readMemory(zpAddr + 1);

    u16 addr = (msb << 8) | lsb;
    u8 page = addr >> 8;
    addr += y;
    if ((addr >> 8) != page || opcode == 0x91) {
        tick();
    }

    return (u16) (((msb << 8) | lsb) + y);
}

// Flag setting utilities

void CPU::setZN(u8 val) {
    p.z = 0;
    if (val == 0) p.z = 1;
    p.n = val >> 7;
}
