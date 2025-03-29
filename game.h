#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <vector>
#include <unordered_map>
#include "constants.h"
#include "enum.h"
#include "structs.h"

extern SDL_Renderer* gRenderer;  // Khai báo extern vì gRenderer được định nghĩa ở main.cpp
extern std::unordered_map<std::string, SDL_Texture*> gTextureCache;
extern int map[MAP_ROWS][MAP_COLS];
extern std::vector<Explosion> explosions;
extern std::vector<Tank> enemyTanks;
extern GameState gameState;
extern Tank playerTank;
extern std::vector<Bullet> bullets;
extern Uint32 lastBulletTime;

bool isValidMove(float x, float y, int width, int height, Tank* currentTank = nullptr);
bool isTileBlocked(int row, int col);
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
void updateGame();
void updatePlayerTank();
void createBullet(Tank& tank);
void updateBullets();
void createExplosion(float x, float y);
void updateExplosions();
Direction getRandomDirection();
void updateAITanks();

#endif
