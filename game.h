#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <vector>
#include "enums.h"
#include "structs.h"

// Forward declarations
struct Tank;
struct Bullet;
struct Explosion;

extern std::vector<Explosion> explosions;
extern std::vector<Tank> enemyTanks;
extern Tank playerTank;
extern std::vector<Bullet> bullets;
extern Uint32 lastBulletTime;
extern int playerLives;

bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
bool isValidMove(float x, float y, int width, int height, Tank* currentTank = nullptr);
void createBullet(Tank& tank, SDL_Renderer* renderer);
void updateBullets(SDL_Renderer* renderer);
void createExplosion(float x, float y);
void updateExplosions();
void updateAITanks(SDL_Renderer* renderer);
void moveAITank(Tank& tank);
Direction getRandomDirection();
void resetPlayerPosition();
void updatePlayerTank();

#endif
