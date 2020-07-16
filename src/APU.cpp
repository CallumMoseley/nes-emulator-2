#include <cstdio>
#include "APU.h"

void audioCallback(void* userdata, u8* stream, int len) {
    APU* apu = (APU*) userdata;

    SDL_memset(stream, 0, len);
}

APU::APU() : volume{128} {
    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 22050;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 4096;
    want.callback = audioCallback;
    want.userdata = this;

    this->dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (this->dev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    } else {
        //SDL_PauseAudioDevice(dev, 0);
    }
}

APU::~APU() {
    SDL_CloseAudioDevice(this->dev);
}

float APU::pulseTable[31];
float APU::tndTable[203];

void APU::initTables() {
    for (int i = 0; i < 31; i++) {
        pulseTable[i] = 95.52f / (8128.0f / i + 100.0f);
    }
    for (int i = 0; i < 203; i++) {
        tndTable[i] = 163.67f / (24329.0f / i + 100.0f);
    }
}

float APU::calcOutput(int pulse1, int pulse2, int triangle, int noise, int dmc) {
    float pulseOutput = pulseTable[pulse1 + pulse2];
    float tndOutput = tndTable[3 * triangle + 2 * noise + dmc];

    return pulseOutput + tndOutput;
}

void APU::tick() {
}

u8 APU::getVolume() {
    return this->volume;
}

void APU::setVolume(u8 vol) {
    if (vol > 128) {
        vol = 128;
    }
    this->volume = vol;
}


u8 PulseChannel::pulseWaveforms[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0}
};
