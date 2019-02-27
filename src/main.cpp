#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <cstdio>
#include "NES.h"
#include "constants.h"

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
                              SCREEN_WIDTH * SCALE,
                              SCREEN_HEIGHT * SCALE,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetScale(renderer, SCALE, SCALE);

    NES nes(renderer);
    nes.start();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
