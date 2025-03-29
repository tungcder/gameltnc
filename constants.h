#ifndef CONSTANTS_H
#define CONSTANTS_H

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 40;
const int MAP_ROWS = SCREEN_HEIGHT / TILE_SIZE;
const int MAP_COLS = SCREEN_WIDTH / TILE_SIZE;
const int TANK_SIZE = 40;
const float TANK_SPEED = 2.0f;
const int FRAME_DELAY = 16;
const int BULLET_WIDTH = 10;
const int BULLET_HEIGHT = 10;
const float BULLET_SPEED = 6.0f;
const int EXPLOSION_WIDTH = 20;
const int EXPLOSION_HEIGHT = 20;
const int EXPLOSION_FRAMES = 3;
const int BULLET_COOLDOWN = 200;
const int AI_MOVE_DELAY = 100;
const int AI_SHOOT_COOLDOWN = 2000;
const int EXPLOSION_DELAY = 5;
const int PLAYER_SPRITE_WIDTH = 40;
const int PLAYER_SPRITE_HEIGHT = 40;
const int ENEMY_SPRITE_WIDTH = 40;
const int ENEMY_SPRITE_HEIGHT = 40;

#endif
