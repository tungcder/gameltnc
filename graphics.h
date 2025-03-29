#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <unordered_map>
#include <string>
#include "enum.h"
#include "structs.h"

extern SDL_Renderer* gRenderer;  // Khai báo extern
extern std::unordered_map<std::string, SDL_Texture*> gTextureCache;
extern GameState gameState;
extern std::vector<Button> buttons;
extern Tank playerTank;
extern std::vector<Tank> enemyTanks;
extern std::vector<Bullet> bullets;
extern std::vector<Explosion> explosions;

// Khai báo các hàm
SDL_Texture* loadTexture(const std::string& path);
SDL_Texture* getTexture(const std::string& path);
void renderMap();
void renderMenu();
void renderWin();
void renderLose();
void renderGame();
void renderBullets();
void renderExplosions();

#endif
