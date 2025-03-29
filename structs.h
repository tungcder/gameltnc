#ifndef STRUCTS_H
#define STRUCTS_H

#include <SDL.h>

struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
};

struct Tank {
    float x, y;
    Direction direction;
    int frame;
    SDL_Texture* texture;
    Direction lastValidDirection;
    float targetX, targetY;
    bool isPlayer;
    Uint32 lastShotTime;
    int moveDelayCounter;
    bool active;
};

struct Bullet {
    float x, y;
    Direction direction;
    SDL_Texture* texture;
    bool active;
    bool isPlayerBullet;
};

struct Explosion {
    float x, y;
    int frame;
    int frameTimer;
    bool active;
};

#endif
