#include "cart.h"

cart::cart() {}
cart::~cart() {}

cart* cart::fromFile(char* filename) {
    std::ifstream file(filename, std::ios::in | std::ios::ate | std::ios::binary);
//    int size = file.tellg();
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
        u8* prgRom = new u8[prgSize * 0x4000];
        u8* chrRom = new u8[chrSize * 0x2000];

        file.read((char*) (prgRom), prgSize * 0x4000);
        file.read((char*) (chrRom), chrSize * 0x2000);

        cart* game = new cart000(prgSize, chrSize, prgRom, chrRom);
        return game;
    }

    return nullptr;
}
