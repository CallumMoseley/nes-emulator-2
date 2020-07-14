#ifndef APU_H
#define APU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include "types.h"

void audioCallback(void* userdata, u8* stream, int len);

class PulseChannel {
private:
    static u8 pulseWaveforms[4][8];

    u16 timerSet;
    u16 timer;
public:
};

class APU {
private:
    SDL_AudioDeviceID dev;

    u8 volume;

    static float pulseTable[31];
    static float tndTable[203];

public:
    APU();
    ~APU();

    static void initTables();
    static float calcOutput(int, int, int, int, int);

    void powerOn();
    void reset();

    void tick();

    u8 getVolume();
    void setVolume(u8 vol);

    void writeRegister(u16 addr, u8 v);
    u8 readRegister(u16 addr);
};

#endif
