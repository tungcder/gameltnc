#include "map.h"
#include "texture_manager.h"

int map[15][20] = {
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,2},
    {2,0,3,3,0,0,4,4,0,0,0,0,0,4,4,0,3,3,0,2},
    {2,0,3,3,0,1,1,4,0,2,2,2,0,4,1,1,0,3,3,2},
    {2,0,0,0,0,1,0,0,0,2,2,2,0,0,0,1,0,0,0,2},
    {2,1,1,1,0,1,4,4,0,0,0,0,0,4,4,1,0,1,1,2},
    {2,0,0,0,0,1,4,4,0,1,1,1,0,4,4,1,0,0,0,2},
    {2,0,3,3,0,0,0,0,0,1,0,1,0,0,0,0,0,3,3,2},
    {2,0,3,3,0,1,1,1,0,1,0,1,0,1,1,1,0,3,3,2},
    {2,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,2},
    {2,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,2},
    {2,0,0,0,0,1,4,4,4,4,4,4,4,4,4,1,0,0,0,2},
    {2,0,3,3,0,0,0,0,0,1,0,1,0,0,0,0,0,3,3,2},
    {2,0,3,3,0,1,1,1,0,1,0,1,0,1,1,1,0,3,3,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
};

void renderMap(SDL_Renderer* renderer) {
    for (int row = 0; row < 15; ++row) {
        for (int col = 0; col < 20; ++col) {
            SDL_Rect tileRect = {col * 40, row * 40, 40, 40};
            SDL_Texture* texture = nullptr;

            switch (map[row][col]) {
                case 1: texture = TextureManager::getTexture("image/wall1.png", renderer); break;
                case 2: texture = TextureManager::getTexture("image/wall2.png", renderer); break;
                case 3: texture = TextureManager::getTexture("image/water.png", renderer); break;
                case 4:  texture = TextureManager::getTexture("image/tree.png", renderer); break;
                default: break;
            }

            if (texture) {
                SDL_RenderCopy(renderer, texture, nullptr, &tileRect);
            }
        }
    }
}

bool isTileBlocked(int row, int col) {
    if (row < 0 || row >= 15 || col < 0 || col >= 20) {
        return true;
    }
    return map[row][col] != 0 && map[row][col] != 4;
}
