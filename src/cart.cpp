#include <cstdio>
#include <fstream>
#include "cart.h"

cart* cart::fromFile(char* filename) {
    std::ifstream file(filename, std::ios::in | std::ios::ate | std::ios::binary);
    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    char b;
    file.get(b);
    if (b != 'N') printf("Warning: file format may be invalid\n");
    file.get(b);
    if (b != 'E') printf("Warning: file format may be invalid\n");
    file.get(b);
    if (b != 'S') printf("Warning: file format may be invalid\n");
    file.get(b);
    if (b != 0x1A) printf("Warning: file format may be invalid\n");

    char prgSize; 
    file.get(prgSize);
    char chrSize; 
    file.get(chrSize);

    char b6, b7;
    file.get(b6);
    file.get(b7);

    u8 mapper = (b7 & 0xF0) | (b6 >> 4);

    for (int i = 0; i < 8; i++) {
        file.get();
    }

    if (mapper == 0) {
        u8* prgRom = new u8[prgSize];
        u8* chrRom = new u8[prgSize];

        file.read((char*) (prgRom), prgSize * 0x4000);
        file.read((char*) (chrRom), chrSize * 0x2000);

        return new cart000(prgRom, chrRom);
    }

    return nullptr;
}

cart::cart() {}
cart::~cart() {}

cart000::cart000(u8* prg, u8* chr) {
    this->prg = prg;
    this->chr = chr;
}
cart000::~cart000() {
    delete[] prg;
    delete[] chr;
}

u8 cart000::readMemoryCPU(u16 addr) {
    if (addr < 0x8000) return 0x00;
    return prg[addr & 0x3FFF];
}

void cart000::writeMemoryCPU(u16 addr, u8 v) {
    if (addr < 0x8000) return;
    prg[addr & 0x3FFF] = v;
}

u8 cart000::readMemoryPPU(u16 addr) {
    if (addr >= 0x2000) return 0x00;
    return chr[addr];
}

void cart000::writeMemoryPPU(u16 addr, u8 v) {
    if (addr >= 0x2000) return;
    chr[addr] = v;
}

allsuite_cart::allsuite_cart() {
    std::ifstream testBin("test/AllSuiteA/AllSuiteA.bin", std::ios::in | std::ios::ate | std::ios::binary);
    int size = testBin.tellg();
    memSize = (u16) size;
    testBin.seekg(0, std::ios::beg);
    memory = new u8[size];
    char* readData = new char[size];
    testBin.read(readData, size);
    
    for (int i = 0; i < size; i++) {
        memory[i] = readData[i];
    }
}

allsuite_cart::~allsuite_cart() {
    delete[] memory;
}

u8 allsuite_cart::readMemoryCPU(u16 addr) {
    if (addr >= 0x4000) {
        if (addr - 0x4000 < memSize) {
            return memory[addr - 0x4000];
        } else if (addr == 0xFFFC) {
            return 0x00;
        } else if (addr == 0xFFFD) {
            return 0x40;
        } else if (addr == 0xFFFE) {
            return 0xA4;
        } else if (addr == 0xFFFF) {
            return 0xF5;
        }
    }
    return 0;
}

void allsuite_cart::writeMemoryCPU(u16 addr, u8 v) {
    if (addr >= 0x4000 && addr < 0x4000 + memSize) {
        memory[addr] = v;
    }
}

u8 allsuite_cart::readMemoryPPU(u16 addr) {
    return 0x00;
}

void allsuite_cart::writeMemoryPPU(u16 addr, u8 v) {}
