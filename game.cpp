#include "game.h"
#include "graphics.h"
#include <algorithm>

bool isValidMove(float x, float y, int width, int height, Tank* currentTank) {
    int topLeftX = (int)floor(x / TILE_SIZE);
    int topLeftY = (int)floor(y / TILE_SIZE);
    int topRightX = (int)floor((x + width - 1) / TILE_SIZE);
    int topRightY = (int)floor(y / TILE_SIZE);
    int bottomLeftX = (int)floor(x / TILE_SIZE);
    int bottomLeftY = (int)floor((y + height - 1) / TILE_SIZE);
    int bottomRightX = (int)floor((x + width - 1) / TILE_SIZE);
    int bottomRightY = (int)floor((y + height - 1) / TILE_SIZE);

    if (isTileBlocked(topLeftY, topLeftX) ||
        isTileBlocked(topRightY, topRightX) ||
        isTileBlocked(bottomLeftY, bottomLeftX) ||
        isTileBlocked(bottomRightY, bottomRightX)) {
        return false;
    }

    // Kiểm tra va chạm với xe tăng khác
    if (currentTank->isPlayer) {
        for (auto& enemyTank : enemyTanks) {
            if (!enemyTank.active) continue;
            if (&enemyTank == currentTank) continue; // Bỏ qua kiểm tra với chính nó

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                return false;
            }
        }
    } else {
        if (playerTank.active && checkCollision(x, y, width, height, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
            return false;
        }
        for (auto& enemyTank : enemyTanks) {
            if (!enemyTank.active) continue;
            if (&enemyTank == currentTank) continue; // Bỏ qua kiểm tra với chính nó

            if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                return false;
            }
        }
    }
    return true;
}

bool isTileBlocked(int row, int col) {
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) {
        return true;
    }
    return map[row][col] != EMPTY && map[row][col] != TREE;
}
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

void updateGame() {
    if (gameState == GAME) {
        updatePlayerTank();
        updateAITanks();
        updateBullets();
        if (enemyTanks.empty()) {
            gameState = WIN;
            //Mix_HaltMusic();
            //Mix_PlayChannel(-1, gGameOverSound, 0);
        }
        if(!playerTank.active){
            gameState = LOSE;
            //Mix_HaltMusic();
            //Mix_PlayChannel(-1, gGameOverSound, 0);
        }
    }
}

void updatePlayerTank() {
    if (playerTank.direction == NONE) return;

    float newX = playerTank.x;
    float newY = playerTank.y;

    switch (playerTank.direction) {
        case UP:    newY -= TANK_SPEED; break;
        case DOWN:  newY += TANK_SPEED; break;
        case LEFT:  newX -= TANK_SPEED; break;
        case RIGHT: newX += TANK_SPEED; break;
        case NONE:  break;
    }

    if (isValidMove(newX, newY, TANK_SIZE, TANK_SIZE, &playerTank)) {
        playerTank.x = newX;
        playerTank.y = newY;
    } else {
        playerTank.direction = NONE;
    }
    playerTank.frame = (playerTank.frame + 1) % 2;
}

void createExplosion(float x, float y) {
    Explosion newExplosion;
    newExplosion.x = x;
    newExplosion.y = y;
    newExplosion.frame = 0;
    newExplosion.frameTimer = 0;
    newExplosion.active = true;
    explosions.push_back(newExplosion);
}

void updateExplosions() {
    for (auto& explosion : explosions) {
        if (explosion.active) {
            explosion.frameTimer++;

            if (explosion.frameTimer >= EXPLOSION_DELAY) {
                explosion.frameTimer = 0;
                explosion.frame++;

                if (explosion.frame >= EXPLOSION_FRAMES) {
                    explosion.active = false;
                }
            }
        }
    }
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& e) { return !e.active; }), explosions.end());
}

void createBullet(Tank& tank) {
    Bullet newBullet;
    newBullet.x = tank.x + TANK_SIZE / 2 - BULLET_WIDTH / 2;
    newBullet.y = tank.y + TANK_SIZE / 2 - BULLET_HEIGHT / 2;
    newBullet.direction = (tank.direction == NONE) ? tank.lastValidDirection : tank.direction;
    newBullet.texture = getTexture("image/bullet.png");
    newBullet.active = true;
    newBullet.isPlayerBullet = tank.isPlayer;

    bullets.push_back(newBullet);
}

void updateBullets() {
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet& bullet = *it;
        if (!bullet.active) {
            it = bullets.erase(it);
            continue;
        }
        switch (bullet.direction) {
            case UP: bullet.y -= BULLET_SPEED; break;
            case DOWN: bullet.y += BULLET_SPEED; break;
            case LEFT: bullet.x -= BULLET_SPEED; break;
            case RIGHT: bullet.x += BULLET_SPEED; break;
            case NONE: bullet.active = false; break;
        }
        if (bullet.x < 0 || bullet.x > SCREEN_WIDTH || bullet.y < 0 || bullet.y > SCREEN_HEIGHT) {
            bullet.active = false;
        }
        int tileX = (int)(bullet.x / TILE_SIZE);
        int tileY = (int)(bullet.y / TILE_SIZE);

        if (tileX >= 0 && tileX < MAP_COLS && tileY >= 0 && tileY < MAP_ROWS) {
            if (map[tileY][tileX] == BRICK) {
                map[tileY][tileX] = EMPTY;
                bullet.active = false;
                createExplosion(tileX * TILE_SIZE, tileY * TILE_SIZE);
                //Mix_PlayChannel(-1, gBulletHitSound, 0);

            } else if (map[tileY][tileX] != EMPTY && map[tileY][tileX] != TREE) {
                bullet.active = false;
                createExplosion(bullet.x - BULLET_WIDTH / 2, bullet.y - BULLET_HEIGHT / 2);
               // Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
        if (bullet.isPlayerBullet) {
            for (auto& enemyTank : enemyTanks) {
                if (!enemyTank.active) continue;
                if (checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                    bullet.active = false;
                    createExplosion(enemyTank.x, enemyTank.y);
                    enemyTank.active = false;
                    //Mix_PlayChannel(-1, gBulletHitSound, 0);
                    break;
                }
            }
        } else {
            if (playerTank.active && checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                bullet.active = false;
                createExplosion(playerTank.x, playerTank.y);
                playerTank.active = false;
                //Mix_PlayChannel(-1, gBulletHitSound, 0);
            }
        }
         ++it;
    }
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());
    enemyTanks.erase(std::remove_if(enemyTanks.begin(), enemyTanks.end(), [](const Tank& t) { return !t.active; }), enemyTanks.end());
}

Direction getRandomDirection() {
    int randDir = rand() % 4;
    switch (randDir) {
        case 0: return UP;
        case 1: return DOWN;
        case 2: return LEFT;
        case 3: return RIGHT;
        default: return NONE;
    }
}

void updateAITanks() {
    for (auto& tank : enemyTanks) {
        if (!tank.active) continue;

        if (tank.moveDelayCounter > 0) {
            tank.moveDelayCounter--;
        } else {
            tank.moveDelayCounter = AI_MOVE_DELAY;
            tank.direction = getRandomDirection();
        }
        moveAITank(tank);

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - tank.lastShotTime > AI_SHOOT_COOLDOWN) {
            tank.lastShotTime = currentTime;
            createBullet(tank);
        }
    }
}
