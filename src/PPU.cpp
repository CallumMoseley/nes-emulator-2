#include "PPU.h"
#include <cstdio>

PPU::PPU(SDL_Renderer* renderer) {
    this->renderer = renderer;
    this->texture = SDL_CreateTexture(renderer,
                                      SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      256,
                                      240);

    oam = new u8[0x100];
    oam2 = new u8[8];
    vram = new u8[0x800];
    paletteRam = new u8[0x20];

    initializeMemory();

    pixels = nullptr;

    oamAddr = 0x00;
    vramAddr = 0x0000;

    w = false;

    nmiOccurred = false;
    s0hit = false;
    sprOverflow = false;

    frameCount = 0;

    scanline = 0;
    dot = 0;
    
    fineX = 0;

    oamAddr = 0x00;
    vramAddr = 0x00;
    tempAddr = 0x00;

    lowBGShift1 = 0;
    highBGShift1 = 0;
    lowBGShift2 = 0;
    highBGShift2 = 0;
    attr = 0;
}

PPU::~PPU() {
    delete[] oam;
    delete[] oam2;
    delete[] vram;
    delete[] paletteRam;
}

void PPU::initializeMemory() {
    for (int i = 0; i < 0x100; i++) {
        oam[i] = 0x00;
    }
    for (int i = 0; i < 8; i++) {
        oam2[i] = 0x00;
    }
    for (int i = 0; i < 0x800; i++) {
        vram[i] = 0x00;
    }
    for (int i = 0; i < 0x20; i++) {
        paletteRam[i] = 0x00;
    }
}

void PPU::powerOn() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);
    oamAddr = 0x00;
    vramAddr = 0x0000;

    w = false;

    nmiOccurred = false;
    s0hit = false;
    sprOverflow = false;

    frameCount = 0;

    scanline = 0;
    dot = 0;
    
    fineX = 0;

    oamAddr = 0x00;
    vramAddr = 0x00;
    tempAddr = 0x00;

    lowBGShift1 = 0;
    highBGShift1 = 0;
    lowBGShift2 = 0;
    highBGShift2 = 0;
    attr = 0;
}

void PPU::reset() {
    writeRegister(0x2000, 0x00);
    writeRegister(0x2001, 0x00);

    w = false;
    frameCount = 0;
    nmiOccurred = false;

    scanline = 0;
    dot = 0;

    fineX = 0;

    oamAddr = 0x00;
    vramAddr = 0x00;
    tempAddr = 0x00;

    bgPatternTable = 0;
    sprPatternTable = 0;
    vramInc = 1;

    lowBGShift1 = 0;
    highBGShift1 = 0;
    lowBGShift2 = 0;
    highBGShift2 = 0;
    attr = 0;
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

void PPU::shiftRegisters() {
    lowBGShift2 <<= 1;
    highBGShift2 <<= 1;

    lowBGShift2 |= (lowBGShift1 >> 7);
    highBGShift2 |= (highBGShift1 >> 7);

    lowBGShift1 <<= 1;
    highBGShift1 <<= 1;
}

void PPU::loadTile() {
    // Fetch nametable byte from current nametable at position
    u8 ntByte = readMemory(0x2000 | (vramAddr & 0x0FFF));
    // Fetch attribute table byte from nametable at current address
    u8 attrByte = readMemory(0x23C0 | (vramAddr & 0x0C00) | ((vramAddr >> 4) & 0x38) | ((vramAddr >> 2) & 0x07));

    // Determine which quadrant this nametable byte occupies
    u8 attrQuad = ((vramAddr & 0x20) >> 5) | ((vramAddr >> 1) & 0x01);
    // Get the 2 bit attribute information for the current quadrant
    attr = attrByte >> (attrQuad * 2) & 0x03;

    // Fine y position for fetching information from pattern table
    u8 fineY = (vramAddr & 0x7000) >> 12;
    // Address into pattern table
    u16 patternAddr = (bgPatternTable << 12) | (ntByte << 4) | fineY;

    // Background tile pattern info
    lowBGShift1 = readMemory(patternAddr);
    highBGShift1 = readMemory(patternAddr | 0x0008);

    if (dot != 256) {
        incAddrX(vramAddr);
    } else {
        incAddrY(vramAddr);
    }
}

void PPU::tick() {
    // Visible scanlines
    if (scanline >= 0 && scanline < 240) {
        if (bgEn || sprEn) {
            if (dot == 0) {
                SDL_LockTexture(texture, nullptr, (void**) &pixels, &pitch);
            }
            if (scanline == 0 && dot == 0 && frameCount % 2 == 1) dot++;
            // Regular NT, AT fetch, rendering
            if (dot > 0 && dot <= 256) {
                if (dot >= 2) {
                    shiftRegisters();
                }
                u8 bgPattern = ((highBGShift2 >> (7 - fineX)) & 0x01) << 1 |
                               ((lowBGShift2 >> (7 - fineX)) & 0x01);
                
                u8 bgColour = readMemory(0x3F00 | (attr << 2) | bgPattern);

                renderPixel(bgColour);

                // Load next tile (cheating by doing it all at once)
                if (dot % 8 == 0) {
                    loadTile();
                }
            } else if (dot == 257) {
                vramAddr = (vramAddr & ~0x41F) | (tempAddr & 0x41F);
                shiftRegisters();
            } else if (dot >= 332 && dot <= 337) {
                shiftRegisters();
                if (dot % 8 == 0) {
                    loadTile();
                }
            }
        }
    } else if (scanline == 240) {
        if ((bgEn || sprEn) && dot == 0) {
            SDL_UnlockTexture(texture);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
    } else if (scanline == 241) {
        if (dot == 1) {
            nmiOccurred = true;
        }
    } else if (scanline == 261) {
        if (dot == 1) {
            nmiOccurred = false;
            s0hit = false;
            sprOverflow = false;
        } else if (dot == 304) {
            if (sprEn || bgEn) {
                vramAddr = (vramAddr & ~0x7BE0) | (tempAddr & 0x7BE0);
            }
        } else if (dot == 257) {
            if (sprEn || bgEn) {
                vramAddr = (vramAddr & ~0x41F) | (tempAddr & 0x41F);
                shiftRegisters();
            }
        } else if (dot == 328 || dot == 336) {
            if (sprEn || bgEn) {
                loadTile();
            }
        } else if (dot == 340) {
            frameCount++;
        }
    }

    dot++;
    if (dot == 341) {
        scanline++;
    }
    dot %= 341;
    scanline %= 262;
}

void PPU::renderPixel(u8 colour) {
    pixels[scanline * 256 + dot - 1] = colours[colour & 0x3F];
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

        if (gameCart->vMirror) {
            addr &= ~0x0800;
        } else {
            u8 bit11 = (addr & 0x0800) >> 11;
            addr &= ~0x0C00;
            addr |= bit11 << 10;
        }

        return vram[addr & 0x7FF];
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

        if (gameCart->vMirror) {
            addr &= ~0x0800;
        } else {
            u8 bit11 = (addr & 0x0800) >> 11;
            addr &= ~0x0C00;
            addr |= bit11 << 10;
        }

        vram[addr & 0x7FF] = v;
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
            vramInc = ((v >> 2) & 0x01) * 31 + 1;
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
    u8 ret = 0x00;
    switch (addr) {
        case 0x02: // PPUSTATUS
            ret = (nmiOccurred << 7) | (s0hit << 6) | (sprOverflow << 5);
            nmiOccurred = false;
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
    return ret;
}

void PPU::writeOam(u8 v) {
    oam[oamAddr++] = v;
}

bool PPU::nmiLow() {
    return nmiOccurred && nmiEnabled;
}

void PPU::printNametable(u16 addr) {
    u16 addrCopy = addr;

    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 32; j++) {
            printf("+------");
        }
        printf("+\n");

        for (int j = 0; j < 32; j++) {
            printf("|      ");
        }
        printf("|\n");

        for (int j = 0; j < 32; j++) {
            printf("| 0x%02x ", readMemory(addrCopy));
            addrCopy++;
        }
        printf("|\n");

        for (int j = 0; j < 32; j++) {
            printf("|      ");
        }
        printf("|\n");
    }
}

void PPU::printPattern(u8 table, u8 pattern) {
    for (u8 i = 0; i < 8; i++) {
        u16 addr = (table << 12) | (pattern << 4) | i;
        u8 lowByte = readMemory(addr);
        u8 highByte = readMemory(addr | 0x8);
        for (int j = 7; j >= 0; j--) {
            u8 bit = (((highByte >> j) & 0x01) << 1) | ((lowByte >> j) & 0x01);
            if (bit == 0) {
                printf(".");
            } else {
                printf("%d", bit);
            }
        }
        printf("\n");
    }
}
