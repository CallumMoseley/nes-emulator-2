#include "cart.h"

cart::cart() {}
cart::~cart() {}

cart* cart::fromFile(char filename[]) {
    std::ifstream file(filename, std::ios::in |
                                 std::ios::ate |
                                 std::ios::binary);
    file.seekg(0, std::ios::beg);

    u8* header = new u8[16];

    for (int i = 0; i < 16; i++) {
        header[i] = file.get();
    }

    if (header[0] != 'N') printf("Warning: file format may be invalid\n");
    if (header[1] != 'E') printf("Warning: file format may be invalid\n");
    if (header[2] != 'S') printf("Warning: file format may be invalid\n");
    if (header[3] != 0x1A) printf("Warning: file format may be invalid\n");

    int mapper = (header[8] & 0x0F << 8) | (header[7] & 0xF0) | (header[6] >> 4);
    int submapper = header[8] >> 4;

    int prgRomSize = (header[9] & 0x0F << 8) | header[4]; 
    // Use exponential representation if header 9 is F
    if ((header[9] & 0x0F) == 0x0F) {
        prgRomSize = (1 << (header[4] >> 2)) * ((header[4] & 0x03) * 2 + 1);
    }
    int chrRomSize = (header[9] & 0xF0 << 4) | header[5]; 
    // Use exponential representation if header 9 is F
    if ((header[9] & 0xF0) == 0xF0) {
        chrRomSize = (1 << (header[5] >> 2)) * ((header[5] & 0x03) * 2 + 1);
    }

    int prgRamSize = 64 << (header[10] & 0x0F);
    int prgNvramSize = 64 << (header[10] & 0xF0 >> 4);

    int chrRamSize = 64 << (header[11] & 0x0F);
    int chrNvramSize = 64 << (header[11] & 0xF0 >> 4);

    if ((header[12] & 0x03) != 0) {
        printf("Warning: This emulator only supportsr NTSC");
    }

    if (mapper == 0) {
        u8* prgRom = new u8[prgRomSize * 0x4000];
        u8* chrRom = new u8[chrRomSize * 0x2000];

        file.read((char*) (prgRom), prgRomSize * 0x4000);
        file.read((char*) (chrRom), chrRomSize * 0x2000);

        cart* game = new cart000(prgRomSize,
                                 chrRomSize,
                                 prgRom,
                                 chrRom,
                                 header[6] & 0x01);
        delete[] prgRom;
        delete[] chrRom;
        return game;
    } else if (mapper == 1) {
        u8* prgRom = new u8[prgRomSize * 0x4000];
        u8* chrRom = new u8[chrRomSize * 0x2000];

        file.read((char*) (prgRom), prgRomSize * 0x4000);
        file.read((char*) (chrRom), chrRomSize * 0x2000);

        cart* game = new cart001(prgRamSize,
                                 prgRomSize,
                                 chrRomSize,
                                 prgRom,
                                 chrRom);
        delete[] prgRom;
        delete[] chrRom;
        return game;       
    } else {
        printf("Mapper %d is not supported yet\n", mapper);
    }

    return nullptr;
}
