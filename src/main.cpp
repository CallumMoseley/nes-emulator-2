#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <cstdio>
#include "NES.h"

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

int main() {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    int err = SDL_Init(SDL_INIT_VIDEO);
    if (err < 0) {
        printf("SDL couldn't initialize: %s\n", SDL_GetError());
        return err;
    }

    window = SDL_CreateWindow("NES Emulator",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH,
                              SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    NES nes(renderer);
    nes.start();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
