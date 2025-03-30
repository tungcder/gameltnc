#ifndef MAP_H
#define MAP_H

#include "enums.h"
#include <SDL.h>

extern int map[15][20]; // Khai báo biến map

void renderMap(SDL_Renderer* renderer);
bool isTileBlocked(int row, int col);

#endif
