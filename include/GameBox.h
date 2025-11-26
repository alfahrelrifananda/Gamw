#ifndef GAMEBOX_H
#define GAMEBOX_H

#include <SDL2/SDL.h>
#include <vector>

struct Platform {
    SDL_Rect rect;
    bool isBreakable;
    bool isBrick;
};

struct Enemy {
    float x, y;
    float vx;
    SDL_Rect rect;
    bool active;
};

struct Coin {
    int x, y;
    bool collected;
    float animPhase;
};

bool runGameBox(SDL_Renderer* renderer);

#endif
