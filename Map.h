#ifndef MAP_H
#define MAP_H

#include <SDL.h>
#include <SDL_image.h>

const int TILE_SIZE = 40;
const int MAP_ROWS = 15;
const int MAP_COLS = 20;

extern SDL_Renderer* gRenderer;
extern SDL_Texture* gWallTexture;
extern SDL_Texture* gSteelWallTexture;
extern SDL_Texture* gWaterTexture;
extern SDL_Texture* gTreeTexture;

void renderMap();

#endif
