#include "cart.h"

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
