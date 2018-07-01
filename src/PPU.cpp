#include "PPU.h"

PPU::PPU() {
    oam = new u8[0x100];
    oam2 = new u8[8];
    vram = new u8[0x800];
    paletteRam = new u8[0x20];
}

void PPU::powerOn() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);
    oamAddr = 0x00;
    vramAddr = 0x0000;

    w = false;

    scrollX = 0x00;
    scrollY = 0x00;

    inVblank = false;
    s0hit = false;
    sprOverflow = false;

    frameCount = 0;
}

void PPU::reset() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);

    w = false;

    scrollX = 0x00;
    scrollY = 0x00;

    frameCount = 0;
}

void PPU::incAddrX(u16 &addr) {
    if ((addr & 0x001F) == 31) {
        addr &= ~0x001F;
        addr ^= 0x0400;
    } else {
        addr++;
    }
}

void PPU::incAddrY(u16 &addr) {
    if ((addr & 0x7000) != 0x7000) {
        addr += 0x1000;
    } else {
        addr &= ~0x7000;
        u8 coarse = (addr & 0x03E0) >> 5;
        if (coarse == 29) {
            coarse = 0;
            addr ^= 0x0800;
        } else if (coarse == 31) {
            coarse = 0;
        } else {
            coarse += 1;
        }
        addr = (addr & ~0x03E0) | (coarse << 5);
    }
}

void PPU::incAddrX(u16 &addr) {
    if ((addr & 0x001F) == 31) {
        addr &= ~0x001F;
        addr ^= 0x0400;
    } else {
        addr++;
    }
}

void PPU::tick() {
    // Visible scanlines
    if (scanline >= 0 && scanline < 240) {
        if (scanline == 0 && dot == 0 && frameCount % 2 == 1) dot++;
        if (dot != 0) {
            // Regular NT, AT fetch, rendering
            if (dot <= 256) {
                u8 tilePos = (dot - 1) % 8;
                u8 bgPattern = ((highBGPattern >> (7 - tilePos)) & 0x01) << 1 |
                               ((highBGPattern >> (7 - tilePos)) & 0x01);
                if (dot % 8 == 0) {
                    // Fetch nametable byte from current nametable at position
                    u8 ntByte = readMemory(0x2000 | (vramAddr & 0x0FFF));
                    // Fetch attribute table byte from nametable at current address
                    u8 attrByte = readMemory(0x23C0 | (vramAddr & 0x0C00) | ((vramAddr >> 4) & 0x38) | ((vramAddr >> 2) & 0x07));

                    // Determine which quadrant this nametable byte occupies
                    u8 attrQuad = ((vramAddr & 0x20) >> 5) | ((vramAddr >> 1) & 0x01);
                    // Get the 2 bit attribute information for the current quadrant
                    attr = attrByte >> (attrQuad * 2) & 0x03;

                    // Fine y position for fetching information from pattern table
                    u8 fineY = (vramAddr & 0x7000) >> 24;
                    // Address into pattern table
                    u16 patternAddr = (bgPatternTable << 12) | (ntByte << 4) | fineY;

                    // Background tile pattern info
                    lowBGPattern = readMemory(patternAddr);
                    highBGPattern = readMemory(patternAddr | 0x0008);

                    incAddrX(vramAddr);
                }
            }
        }
    }
}

void PPU::fillOam2() { // Cheaty, not entirely accurate
    u8 pos = 0;
    // scan oam
    for (int i = 0x00; i < 0xFF && pos < 8; i += 0x04) {
        if (scanline >= oam[i] && scanline < oam[i] + 8) {
            oam2[pos++] = i;
        }
    }
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
            tempAddr = (v & 0x03) << 10;
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
            if (w) {
                tempAddr = (tempAddr & 0x0C1F) | ((v & 0x07) << 12) | ((v & 0xF8) << 2);
            } else {
                tempAddr = (tempAddr & 0xFFE0) | ((v & 0xF8) >> 3);
                fineX = v & 0x07;
            }
            w = !w;
            break;
        case 0x06: // PPUADDR
            if (w) {
                tempAddr = (tempAddr & 0x7F00) | v;
                vramAddr = tempAddr;
            } else {
                tempAddr = (tempAddr & 0x00FF) | ((v & 0x3F) << 8);
            }
            w = !w;
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
            w = false;
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
