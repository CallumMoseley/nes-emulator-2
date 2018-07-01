#include "PPU.h"

PPU::PPU() {
    oam = new u8[0x100];
    oam2 = new u8[0x20];
    vram = new u8[0x800];
    paletteRam = new u8[0x20];
}

void PPU::powerOn() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);
    oamAddr = 0x00;
    vramAddr = 0x0000;

    scrollLatch = false;
    addrLatch = false;

    scrollX = 0x00;
    scrollY = 0x00;

    inVblank = false;
    s0hit = false;
    sprOverflow = false;
}

void PPU::reset() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);

    scrollLatch = false;
    addrLatch = false;

    scrollX = 0x00;
    scrollY = 0x00;
}

void PPU::tick() {
}

u8 PPU::readMemory(u16 addr) {
    if (addr < 0x2000) {
        return gameCart->readMemoryPPU(addr);
    } else if (addr < 0x3EFF) {
        addr &= 0x2FFF;
        // TODO nametable mirroring
        return vram[addr & 0x2FFF];
    } else if (addr < 0x4000) {
        addr &= 0xFF1F;
        if ((addr & 0x03) == 0) {
            addr &= 0xFFEF;
        }

        return paletteRam[addr & 0x001F] & (grey ? 0x30 : 0xFF);
    }

    return 0x00;
}

void PPU::writeMemory(u16 addr, u8 v) {
    if (addr < 0x2000) {
        gameCart->writeMemoryPPU(addr, v);
    } else if (addr < 0x3EFF) {
        addr &= 0x2FFF;
        // TODO nametable mirroring
        vram[addr & 0x2FFF] = v;
    } else if (addr < 0x4000) {
        addr &= 0xFF1F;
        if ((addr & 0x03) == 0) {
            addr &= 0xFFEF;
        }

        paletteRam[addr & 0x001F] = v;
    }
}

void PPU::writeRegister(u16 addr, u8 v) {
    // OAMDMA
    if (addr == 0x4014) {
        return;
    }
    addr &= 0x0F;
    switch (addr) {
        case 0x00: // PPUCTRL
            nmiEnabled = v >> 7;
            largeSprites = v >> 5 & 0x01;
            bgPatternTable = v >> 4 & 0x01;
            sprPatternTable = v >> 3 & 0x01;
            vramInc = (v >> 2) * 31 + 1;
            ntSelect = v & 0x03;
            break;
        case 0x01: // PPUMASK
            grey = v & 0x01;
            bgLeft = v >> 1 & 0x01;
            sprLeft = v >> 2 & 0x01;
            bgEn = v >> 3 & 0x01;
            sprEn = v >> 4 & 0x01;
            boostR = v >> 5 & 0x01;
            boostG = v >> 6 & 0x01;
            boostB = v >> 7 & 0x01;
            break;
        case 0x03: // OAMADDR TODO: deal with weird oamaddr bug
            oamAddr = v;
            break;
        case 0x04: // OAMDATA
            oam[oamAddr++] = v;
            break;
        case 0x05: // PPUSCROLL
            if (scrollLatch) {
                scrollY = v;
            } else {
                scrollX = v;
            }
            scrollLatch = !scrollLatch;
            break;
        case 0x06: // PPUADDR
            if (addrLatch) {
                vramAddr |= v;
            } else {
                vramAddr = v << 8;
            }
            addrLatch = !addrLatch;
            break;
        case 0x07: // PPUDATA
            writeMemory(vramAddr, v);
            vramAddr += vramInc;
            break;
        default:
            break;
    }
}

u8 PPU::readRegister(u16 addr) {
    addr &= 0x0F;
    u8 ret;
    switch (addr) {
        case 0x02: // PPUSTATUS
            ret = (inVblank << 7) | (s0hit << 6) | (sprOverflow << 5);
            inVblank = false;
            scrollLatch = false;
            return ret;
            break;
        case 0x04: // OAMDATA
            return oam[oamAddr];
            break;
        case 0x07: // PPUDATA
            readMemory(vramAddr);
            vramAddr += vramInc;
            break;
        default:
            break;
    }
}

void PPU::writeOam(u8 v) {
    oam[oamAddr++] = v;
}
