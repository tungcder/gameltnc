#include <SDL.h>
#include <iomanip>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <sstream>

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
const int EXPLOSION_WIDTH = 20; // Adjusted width to match tank size better
const int EXPLOSION_HEIGHT = 20;// Adjusted height
const int EXPLOSION_FRAMES = 3;
const int BULLET_COOLDOWN = 300;
const int MAX_PLAYER_BULLETS = 3;
const int AI_MOVE_DELAY = 60;
const int AI_SHOOT_COOLDOWN = 1500;
const int EXPLOSION_DELAY = 5;
const int PLAYER_SPRITE_WIDTH = 40;
const int PLAYER_SPRITE_HEIGHT = 40;
const int ENEMY_SPRITE_WIDTH = 40;
const int ENEMY_SPRITE_HEIGHT = 40;
const int PLAYER_LIVES = 3;
const int RESPAWN_DELAY = 1500;

const float PLAYER1_START_X = (float)TILE_SIZE * (MAP_COLS / 2 - 3);
const float PLAYER1_START_Y = (float)TILE_SIZE * (MAP_ROWS - 3);
const float PLAYER2_START_X = (float)TILE_SIZE * (MAP_COLS / 2 + 1);
const float PLAYER2_START_Y = (float)TILE_SIZE * (MAP_ROWS - 3);

enum TileType { EMPTY, BRICK, STEEL, WATER, TREE };
enum GameState { MENU, GAME, WIN, LOSE };
enum Direction { DOWN, LEFT, RIGHT, UP, NONE };

struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
};

struct Tank;

struct Bullet {
    float x, y;
    Direction direction;
    SDL_Texture* texture;
    bool active;
    bool isPlayerBullet;
    Tank* owner;
};

struct Tank {
    float x, y;
    Direction direction;
    int frame;
    SDL_Texture* texture;
    Direction lastValidDirection;
    bool isPlayer;
    Uint32 lastShotTime;
    int moveDelayCounter;
    bool active;
    Uint32 deathTime;
    int currentBullets;
};

struct Explosion {
    float x, y;
    int frame;
    int frameTimer;
    bool active;
};

SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
std::unordered_map<std::string, SDL_Texture*> gTextureCache;
TTF_Font* gFont = nullptr;
SDL_Color gTextColor = { 0, 0, 0 }; // Changed to white

int map[MAP_ROWS][MAP_COLS] = {
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,2},
    {2,0,3,1,1,1,0,4,2,4,2,4,4,0,1,1,3,3,0,2},
    {2,0,3,3,0,0,0,4,0,2,2,0,4,0,0,0,3,3,0,2},
    {2,0,0,0,0,1,1,4,0,2,2,0,4,1,1,0,0,0,0,2},
    {2,1,1,1,0,1,0,0,0,0,0,0,0,0,1,0,1,1,1,2},
    {2,0,0,0,0,1,0,2,2,1,1,2,2,0,1,0,0,0,0,2},
    {2,0,3,3,0,0,0,1,1,1,1,1,1,0,0,0,3,3,0,2},
    {2,0,3,3,0,1,1,1,0,0,0,0,1,1,1,0,3,3,0,2},
    {2,0,0,0,0,1,4,0,0,1,1,0,0,4,1,0,0,0,0,2},
    {2,1,1,1,0,1,4,4,0,1,1,0,4,4,1,0,1,1,1,2},
    {2,0,0,0,0,0,4,4,0,0,0,0,4,4,0,0,0,0,0,2},
    {2,0,1,1,1,1,0,0,0,1,1,0,0,0,1,1,1,1,0,2},
    {2,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
};

std::vector<Explosion> explosions;
std::vector<Tank> enemyTanks;
std::vector<Bullet> bullets;

GameState gameState = MENU;
bool isTwoPlayerMode = false;

std::vector<Button> buttons = {
    {{300, 340, 200, 50}, nullptr},
    {{300, 410, 200, 50}, nullptr},
    {{300, 480, 200, 50}, nullptr}
};

Tank playerTank = { PLAYER1_START_X, PLAYER1_START_Y, NONE, 0, nullptr, UP, true, 0, 0, false, 0, 0 };
Tank player2Tank = { PLAYER2_START_X, PLAYER2_START_Y, NONE, 0, nullptr, UP, true, 0, 0, false, 0, 0 };
int playerLives = PLAYER_LIVES;
int player2Lives = PLAYER_LIVES;

Mix_Music* gMusic = nullptr;
Mix_Chunk* gBulletHitSound = nullptr;
Mix_Chunk* gGameOverSound = nullptr;
Mix_Chunk* gPlayerShootSound = nullptr;
Mix_Chunk* gExplosionSound = nullptr;

bool init();
SDL_Texture* loadTexture(const std::string& path);
SDL_Texture* getTexture(const std::string& path);
bool loadMedia();
void close();

void handleMenuInput(const SDL_Event& e);
void handleGameInput(const SDL_Event& e);
void processGameInput();

void updateGame();
void updatePlayerTank(Tank& tank);
void updateAITanks();
void moveAITank(Tank& tank);
void updateBullets();
void updateExplosions();
void checkRespawn();

void createBullet(Tank& ownerTank);
void createExplosion(float x, float y);

void renderMenu();
void renderGame();
void renderWin();
void renderLose();
void renderMap();
void renderTrees();
void renderTank(const Tank& tank);
void renderBullets();
void renderExplosions();
void renderLives(SDL_Renderer* renderer, int p1Lives, int p2Lives, bool twoPlayers);

bool isValidMove(float x, float y, int width, int height, Tank* currentTank);
bool isTileBlocked(int row, int col);
bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2);
Direction getRandomDirection();
void resetPlayerPosition(Tank& tank, float startX, float startY);

SDL_Texture* getTexture(const std::string& path) {
    auto it = gTextureCache.find(path);
    if (it != gTextureCache.end()) {
        return it->second;
    } else {
        SDL_Texture* texture = loadTexture(path);
        if (texture) {
            gTextureCache[path] = texture;
        } else {
             gTextureCache[path] = nullptr;
             std::cerr << "Texture loaded as nullptr: " << path << std::endl;
        }
        return texture;
    }
}

bool init() {
     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gWindow = SDL_CreateWindow("Battle City Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gRenderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    gFont = TTF_OpenFont("font/Hack.ttf", 30); // Font size set to 30
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }

    srand(static_cast<unsigned int>(time(nullptr)));
    return true;
}

SDL_Texture* loadTexture(const std::string& path) {
     SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (!newTexture) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

bool loadMedia() {
    bool success = true;
    gMusic = Mix_LoadMUS("sound/intro.mp3");
    if (!gMusic) std::cerr << "Failed to load menu music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    gBulletHitSound = Mix_LoadWAV("sound/no.wav");
    if (!gBulletHitSound) std::cerr << "Failed to load bullet hit sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    gGameOverSound = Mix_LoadWAV("sound/end.wav");
    if (!gGameOverSound) std::cerr << "Failed to load game over sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    gPlayerShootSound = Mix_LoadWAV("sound/shoot.mp3");
    if (!gPlayerShootSound) std::cerr << "Failed to load player shoot sound! SDL_mixer Error: " << Mix_GetError() << std::endl;
    gExplosionSound = Mix_LoadWAV("sound/no.wav"); // Reusing hit sound for explosion
    if (!gExplosionSound) std::cerr << "Failed to load explosion sound! SDL_mixer Error: " << Mix_GetError() << std::endl;

    if (!getTexture("image/Background.png")) success = false;
    if (!getTexture("image/1player.png")) success = false;
    if (!getTexture("image/2player.png")) success = false;
    if (!getTexture("image/exit.png")) success = false;
    if (!getTexture("image/Tank.png")) success = false;
    if (!getTexture("image/Tank1.png")) success = false;
    if (!getTexture("image/Bullet.png")) success = false;
    if (!getTexture("image/No.png")) success = false;
    if (!getTexture("image/Win.png")) success = false;
    if (!getTexture("image/Lose.png")) success = false;
    if (!getTexture("image/Wall1.png")) success = false;
    if (!getTexture("image/Wall2.png")) success = false;
    if (!getTexture("image/Water.png")) success = false;
    if (!getTexture("image/Tree.png")) success = false;

    if (!success) {
        std::cerr << "Failed to load one or more essential textures!" << std::endl;
        return false;
    }

    buttons[0].texture = getTexture("image/1player.png");
    buttons[1].texture = getTexture("image/2player.png");
    buttons[2].texture = getTexture("image/exit.png");

    return true;
}

void close() {
     for (auto const& [key, val] : gTextureCache) {
        if (val) SDL_DestroyTexture(val);
    }
    gTextureCache.clear();
    if (gBulletHitSound) Mix_FreeChunk(gBulletHitSound);
    if (gGameOverSound) Mix_FreeChunk(gGameOverSound);
    if (gPlayerShootSound) Mix_FreeChunk(gPlayerShootSound);
    if (gExplosionSound) Mix_FreeChunk(gExplosionSound);
    if (gMusic) Mix_FreeMusic(gMusic);
    if (gFont) TTF_CloseFont(gFont);
    if (gRenderer) SDL_DestroyRenderer(gRenderer);
    if (gWindow) SDL_DestroyWindow(gWindow);
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void renderMap() {
    SDL_Texture* brickTexture = getTexture("image/Wall1.png");
    SDL_Texture* steelTexture = getTexture("image/Wall2.png");
    SDL_Texture* waterTexture = getTexture("image/Water.png");
    for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
            SDL_Rect tileRect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            SDL_Texture* textureToRender = nullptr;
            switch (map[row][col]) {
                case BRICK: textureToRender = brickTexture; break;
                case STEEL: textureToRender = steelTexture; break;
                case WATER: textureToRender = waterTexture; break;
            }
            if (textureToRender) {
                SDL_RenderCopy(gRenderer, textureToRender, nullptr, &tileRect);
            }
        }
    }
}

void renderTrees() {
     SDL_Texture* treeTexture = getTexture("image/Tree.png");
     if (!treeTexture) return;
     for (int row = 0; row < MAP_ROWS; ++row) {
        for (int col = 0; col < MAP_COLS; ++col) {
             if (map[row][col] == TREE) {
                 SDL_Rect tileRect = {col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                 SDL_RenderCopy(gRenderer, treeTexture, nullptr, &tileRect);
             }
        }
     }
}

void renderMenu() {
    SDL_RenderCopy(gRenderer, getTexture("image/Background.png"), nullptr, nullptr);
    for (const auto& button : buttons) {
        if (button.texture) {
            SDL_RenderCopy(gRenderer, button.texture, nullptr, &button.rect);
        }
    }
}

void renderWin() {
    SDL_RenderCopy(gRenderer, getTexture("image/Win.png"), nullptr, nullptr);
}

void renderLose() {
    SDL_RenderCopy(gRenderer, getTexture("image/Lose.png"), nullptr, nullptr);
}

void resetPlayerPosition(Tank& tank, float startX, float startY) {
    tank.x = startX;
    tank.y = startY;
    tank.direction = NONE;
    tank.lastValidDirection = UP;
    tank.active = true;
    tank.frame = 0;
    tank.lastShotTime = 0;
    tank.deathTime = 0;
    tank.currentBullets = 0;
}

void handleMenuInput(const SDL_Event& e) {
     if (e.type == SDL_MOUSEBUTTONDOWN) {
        if (e.button.button == SDL_BUTTON_LEFT) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point mousePoint = {x, y};
            for (size_t i = 0; i < buttons.size(); ++i) {
                if (buttons[i].texture && SDL_PointInRect(&mousePoint, &buttons[i].rect)) {
                    if (i == 0 || i == 1) {
                        gameState = GAME;
                        isTwoPlayerMode = (i == 1);
                        enemyTanks.clear();
                        bullets.clear();
                        explosions.clear();
                        SDL_Texture* playerTexture = getTexture("image/Tank.png");
                        SDL_Texture* enemyTexture = getTexture("image/Tank1.png");
                        if (!playerTexture || !enemyTexture) {
                            std::cerr << "CRITICAL: Missing tank textures! Cannot start game." << std::endl;
                            gameState = MENU; return;
                        }
                        playerTank.texture = playerTexture;
                        resetPlayerPosition(playerTank, PLAYER1_START_X, PLAYER1_START_Y);
                        playerLives = PLAYER_LIVES;
                        player2Tank.texture = playerTexture;
                        if (isTwoPlayerMode) {
                            resetPlayerPosition(player2Tank, PLAYER2_START_X, PLAYER2_START_Y);
                            player2Lives = PLAYER_LIVES;
                        } else {
                            player2Tank.active = false;
                        }
                        int numEnemies = 6;
                        for (int k = 0; k < numEnemies; k++) {
                            float enemyX, enemyY;
                            if (k % 3 == 0) enemyX = (float)TILE_SIZE;
                            else if (k % 3 == 1) enemyX = (float)TILE_SIZE * (MAP_COLS / 2 - 1);
                            else enemyX = (float)TILE_SIZE * (MAP_COLS - 2);
                            enemyY = (float)TILE_SIZE * (1 + (k / 3));
                            Tank enemyTank = {enemyX, enemyY, DOWN, 0, enemyTexture, DOWN, false, 0, 0, true, 0, 0};
                            enemyTanks.push_back(enemyTank);
                        }
                        Mix_HaltMusic();
                    } else if (i == 2) {
                        SDL_Event quitEvent; quitEvent.type = SDL_QUIT; SDL_PushEvent(&quitEvent);
                    }
                    break;
                }
            }
        }
    }
}

void handleGameInput(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
        std::cout << "Escape key pressed - potential pause point." << std::endl;
    }
}

void processGameInput() {
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    Uint32 currentTime = SDL_GetTicks();

    if (playerTank.active) {
        Direction desiredDirection = NONE;
        if (keystate[SDL_SCANCODE_W]) desiredDirection = UP;
        else if (keystate[SDL_SCANCODE_S]) desiredDirection = DOWN;
        else if (keystate[SDL_SCANCODE_A]) desiredDirection = LEFT;
        else if (keystate[SDL_SCANCODE_D]) desiredDirection = RIGHT;
        if (playerTank.direction != desiredDirection) playerTank.direction = desiredDirection;
        if (desiredDirection != NONE) playerTank.lastValidDirection = desiredDirection;
        if (keystate[SDL_SCANCODE_SPACE]) {
            if (currentTime - playerTank.lastShotTime > BULLET_COOLDOWN && playerTank.currentBullets < MAX_PLAYER_BULLETS) {
                createBullet(playerTank); playerTank.lastShotTime = currentTime;
                if(gPlayerShootSound) Mix_PlayChannel(-1, gPlayerShootSound, 0);
            }
        }
    }

    if (isTwoPlayerMode && player2Tank.active) {
        Direction desiredDirection2 = NONE;
        if (keystate[SDL_SCANCODE_UP]) desiredDirection2 = UP;
        else if (keystate[SDL_SCANCODE_DOWN]) desiredDirection2 = DOWN;
        else if (keystate[SDL_SCANCODE_LEFT]) desiredDirection2 = LEFT;
        else if (keystate[SDL_SCANCODE_RIGHT]) desiredDirection2 = RIGHT;
        if (player2Tank.direction != desiredDirection2) player2Tank.direction = desiredDirection2;
        if (desiredDirection2 != NONE) player2Tank.lastValidDirection = desiredDirection2;
        if (keystate[SDL_SCANCODE_RETURN] || keystate[SDL_SCANCODE_KP_ENTER]) {
             if (currentTime - player2Tank.lastShotTime > BULLET_COOLDOWN && player2Tank.currentBullets < MAX_PLAYER_BULLETS) {
                createBullet(player2Tank); player2Tank.lastShotTime = currentTime;
                if(gPlayerShootSound) Mix_PlayChannel(-1, gPlayerShootSound, 0);
            }
        }
    }
}

bool isValidMove(float x, float y, int width, int height, Tank* currentTank) {
    if (x < 0 || x + width > SCREEN_WIDTH || y < 0 || y + height > SCREEN_HEIGHT) return false;
    int startCol = static_cast<int>(x / TILE_SIZE);
    int endCol = static_cast<int>((x + width - 1) / TILE_SIZE);
    int startRow = static_cast<int>(y / TILE_SIZE);
    int endRow = static_cast<int>((y + height - 1) / TILE_SIZE);
    for (int row = startRow; row <= endRow; ++row) {
        for (int col = startCol; col <= endCol; ++col) {
            if (isTileBlocked(row, col)) return false;
        }
    }
    if (currentTank != &playerTank && playerTank.active && checkCollision(x, y, width, height, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) return false;
    if (isTwoPlayerMode && currentTank != &player2Tank && player2Tank.active && checkCollision(x, y, width, height, player2Tank.x, player2Tank.y, TANK_SIZE, TANK_SIZE)) return false;
    for (const auto& enemyTank : enemyTanks) {
        if (!enemyTank.active) continue;
        if (!currentTank->isPlayer && currentTank == &enemyTank) continue;
        if (checkCollision(x, y, width, height, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) return false;
    }
    return true;
}

bool isTileBlocked(int row, int col) {
    if (row < 0 || row >= MAP_ROWS || col < 0 || col >= MAP_COLS) return true;
    TileType tile = static_cast<TileType>(map[row][col]);
    return tile == BRICK || tile == STEEL || tile == WATER;
}

bool checkCollision(float x1, float y1, int w1, int h1, float x2, float y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void updatePlayerTank(Tank& tank) {
    if (!tank.active || tank.direction == NONE) return;
    float newX = tank.x, newY = tank.y;
    switch (tank.direction) {
        case UP:    newY -= TANK_SPEED; break;
        case DOWN:  newY += TANK_SPEED; break;
        case LEFT:  newX -= TANK_SPEED; break;
        case RIGHT: newX += TANK_SPEED; break;
        case NONE:  break;
    }
    if (isValidMove(newX, newY, TANK_SIZE, TANK_SIZE, &tank)) {
        tank.x = newX; tank.y = newY;
    }
    tank.frame = (tank.frame + 1) % 2;
}

void updateGame() {
     if (gameState != GAME) return;
    processGameInput();
    if (playerTank.active) updatePlayerTank(playerTank);
    if (isTwoPlayerMode && player2Tank.active) updatePlayerTank(player2Tank);
    updateAITanks();
    updateBullets();
    updateExplosions();
    checkRespawn();

    bool enemiesRemain = false;
    if (!enemyTanks.empty()) {
         for (const auto& enemy : enemyTanks) {
             if (enemy.active) { enemiesRemain = true; break; }
         }
         if (!enemiesRemain) { gameState = WIN; Mix_HaltMusic(); std::cout << "--- GAME WIN ---" << std::endl; }
    }

    bool p1GameOver = !playerTank.active && playerLives <= 0;
    bool p2GameOver = isTwoPlayerMode && !player2Tank.active && player2Lives <= 0;
    if (p1GameOver && (!isTwoPlayerMode || p2GameOver)) {
        gameState = LOSE; Mix_HaltMusic();
        if (gGameOverSound) Mix_PlayChannel(-1, gGameOverSound, 0);
        std::cout << "--- GAME LOSE ---" << std::endl;
    }
}

void createExplosion(float x, float y) {
    Explosion explosion;
    explosion.x = x + TANK_SIZE / 2.0f - EXPLOSION_WIDTH / 2.0f;
    explosion.y = y + TANK_SIZE / 2.0f - EXPLOSION_HEIGHT / 2.0f;
    explosion.frame = 0; explosion.frameTimer = 0; explosion.active = true;
    explosions.push_back(explosion);
    if (gExplosionSound) Mix_PlayChannel(-1, gExplosionSound, 0);
}

void updateExplosions() {
     for (auto it = explosions.begin(); it != explosions.end(); ) {
        if (it->active) {
            it->frameTimer++;
            if (it->frameTimer >= EXPLOSION_DELAY) {
                it->frameTimer = 0; it->frame++;
                if (it->frame >= EXPLOSION_FRAMES) it->active = false;
            }
            ++it;
        } else {
            it = explosions.erase(it);
        }
    }
}

void renderExplosions() {
     SDL_Texture* explosionTexture = getTexture("image/No.png");
    if (!explosionTexture) return;
    for (const auto& explosion : explosions) {
        if (explosion.active) {
            SDL_Rect srcRect = { explosion.frame * EXPLOSION_WIDTH, 0, EXPLOSION_WIDTH, EXPLOSION_HEIGHT };
            SDL_Rect destRect = { (int)explosion.x, (int)explosion.y, EXPLOSION_WIDTH, EXPLOSION_HEIGHT };
            SDL_RenderCopy(gRenderer, explosionTexture, &srcRect, &destRect);
        }
    }
}

void createBullet(Tank& ownerTank) {
    if (ownerTank.currentBullets >= MAX_PLAYER_BULLETS && ownerTank.isPlayer) return;
    if (!ownerTank.isPlayer && ownerTank.currentBullets >= 1) return;

    Bullet newBullet;
    Direction fireDirection = (ownerTank.direction == NONE) ? ownerTank.lastValidDirection : ownerTank.direction;
    float startX = 0, startY = 0;
    float centerOffsetX = TANK_SIZE / 2.0f - BULLET_WIDTH / 2.0f;
    float centerOffsetY = TANK_SIZE / 2.0f - BULLET_HEIGHT / 2.0f;
    switch(fireDirection) {
        case UP: startX = ownerTank.x + centerOffsetX; startY = ownerTank.y - BULLET_HEIGHT; break;
        case DOWN: startX = ownerTank.x + centerOffsetX; startY = ownerTank.y + TANK_SIZE; break;
        case LEFT: startX = ownerTank.x - BULLET_WIDTH; startY = ownerTank.y + centerOffsetY; break;
        case RIGHT: startX = ownerTank.x + TANK_SIZE; startY = ownerTank.y + centerOffsetY; break;
        case NONE: return;
    }
    newBullet.x = startX; newBullet.y = startY;
    newBullet.direction = fireDirection;
    newBullet.texture = getTexture("image/Bullet.png");
    newBullet.active = true; newBullet.isPlayerBullet = ownerTank.isPlayer;
    newBullet.owner = &ownerTank;
    if (!newBullet.texture) { std::cerr << "ERROR: Bullet texture null!" << std::endl; return; }
    bullets.push_back(newBullet);
    ownerTank.currentBullets++;
}


void updateBullets() {
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if (!it->active) {
             if (it->owner) it->owner->currentBullets--; it = bullets.erase(it); continue;
        }
        Bullet& bullet = *it;
        switch (bullet.direction) {
            case UP: bullet.y -= BULLET_SPEED; break; case DOWN: bullet.y += BULLET_SPEED; break;
            case LEFT: bullet.x -= BULLET_SPEED; break; case RIGHT: bullet.x += BULLET_SPEED; break;
            case NONE: bullet.active = false; break;
        }
        if (bullet.x < -BULLET_WIDTH || bullet.x > SCREEN_WIDTH || bullet.y < -BULLET_HEIGHT || bullet.y > SCREEN_HEIGHT) bullet.active = false;
        if (bullet.active) {
            int tileCol = static_cast<int>((bullet.x + BULLET_WIDTH / 2.0f) / TILE_SIZE);
            int tileRow = static_cast<int>((bullet.y + BULLET_HEIGHT / 2.0f) / TILE_SIZE);
            if (tileRow >= 0 && tileRow < MAP_ROWS && tileCol >= 0 && tileCol < MAP_COLS) {
                TileType tile = static_cast<TileType>(map[tileRow][tileCol]);
                if (tile == BRICK) { map[tileRow][tileCol] = EMPTY; bullet.active = false; createExplosion((float)tileCol * TILE_SIZE, (float)tileRow * TILE_SIZE); if (gBulletHitSound) Mix_PlayChannel(-1, gBulletHitSound, 0); }
                else if (tile == STEEL) { bullet.active = false; createExplosion(bullet.x - EXPLOSION_WIDTH / 2.0f + BULLET_WIDTH / 2.0f, bullet.y - EXPLOSION_HEIGHT / 2.0f + BULLET_HEIGHT / 2.0f); if (gBulletHitSound) Mix_PlayChannel(-1, gBulletHitSound, 0); }
            }
        }
        if (bullet.active) {
             if (bullet.isPlayerBullet) {
                for (auto& enemyTank : enemyTanks) {
                    if (!enemyTank.active) continue;
                    if (checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, enemyTank.x, enemyTank.y, TANK_SIZE, TANK_SIZE)) {
                        bullet.active = false; enemyTank.active = false; enemyTank.deathTime = SDL_GetTicks(); createExplosion(enemyTank.x, enemyTank.y); if (gBulletHitSound) Mix_PlayChannel(-1, gBulletHitSound, 0); break;
                    }
                }
            } else {
                 if (playerTank.active && checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, playerTank.x, playerTank.y, TANK_SIZE, TANK_SIZE)) {
                     bullet.active = false; playerTank.active = false; playerTank.deathTime = SDL_GetTicks(); playerLives--; createExplosion(playerTank.x, playerTank.y); if (gBulletHitSound) Mix_PlayChannel(-1, gBulletHitSound, 0);
                 }
                 if (bullet.active && isTwoPlayerMode && player2Tank.active && checkCollision(bullet.x, bullet.y, BULLET_WIDTH, BULLET_HEIGHT, player2Tank.x, player2Tank.y, TANK_SIZE, TANK_SIZE)) {
                     bullet.active = false; player2Tank.active = false; player2Tank.deathTime = SDL_GetTicks(); player2Lives--; createExplosion(player2Tank.x, player2Tank.y); if (gBulletHitSound) Mix_PlayChannel(-1, gBulletHitSound, 0);
                 }
            }
        }
        if (!bullet.active) {
             if (bullet.owner) bullet.owner->currentBullets--; it = bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void renderBullets() {
    SDL_Texture* bulletTexture = getTexture("image/Bullet.png"); if (!bulletTexture) return;
    for (const auto& bullet : bullets) {
        if (bullet.active) {
            SDL_Rect destRect = { (int)bullet.x, (int)bullet.y, BULLET_WIDTH, BULLET_HEIGHT };
            int frameX = 0, spriteWidth = 10;
            switch (bullet.direction) {
                case DOWN: frameX = 0 * spriteWidth; break; case LEFT: frameX = 1 * spriteWidth; break;
                case RIGHT: frameX = 2 * spriteWidth; break; case UP: frameX = 3 * spriteWidth; break;
                case NONE: continue;
            }
            SDL_Rect srcRect = { frameX, 0, spriteWidth, 10 };
            SDL_RenderCopy(gRenderer, bulletTexture, &srcRect, &destRect);
        }
    }
}

Direction getRandomDirection() {
     int randDir = rand() % 4;
    switch (randDir) { case 0: return UP; case 1: return DOWN; case 2: return LEFT; case 3: return RIGHT; default: return DOWN; }
}

void moveAITank(Tank& tank) {
    if (!tank.active || tank.direction == NONE) return;
    float newX = tank.x, newY = tank.y, speed = TANK_SPEED * 0.8f;
    switch (tank.direction) {
        case UP: newY -= speed; break; case DOWN: newY += speed; break;
        case LEFT: newX -= speed; break; case RIGHT: newX += speed; break;
        case NONE: break;
    }
    if (isValidMove(newX, newY, TANK_SIZE, TANK_SIZE, &tank)) { tank.x = newX; tank.y = newY; }
    else { tank.direction = getRandomDirection(); }
    tank.frame = (tank.frame + 1) % 2;
}

void updateAITanks() {
     Uint32 currentTime = SDL_GetTicks();
     for (auto& tank : enemyTanks) {
        if (!tank.active) continue;
        if (tank.moveDelayCounter <= 0) {
            tank.direction = getRandomDirection(); tank.lastValidDirection = tank.direction;
            tank.moveDelayCounter = AI_MOVE_DELAY + (rand() % 60);
        } else { tank.moveDelayCounter--; }
        moveAITank(tank);
        if (currentTime - tank.lastShotTime > (AI_SHOOT_COOLDOWN + (rand() % 1000))) {
            createBullet(tank); tank.lastShotTime = currentTime;
        }
    }
}

void renderLives(SDL_Renderer* renderer, int p1Lives, int p2Lives, bool twoPlayers) {
     if (!gFont) return;
    std::stringstream ss1; ss1 << "P1: " << std::max(0, p1Lives);
    SDL_Surface* surf1 = TTF_RenderText_Solid(gFont, ss1.str().c_str(), gTextColor);
    if (surf1) { SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surf1); if (tex1) { SDL_Rect dst1 = {10, 5, surf1->w, surf1->h}; SDL_RenderCopy(renderer, tex1, nullptr, &dst1); SDL_DestroyTexture(tex1); } SDL_FreeSurface(surf1); }
    if (twoPlayers) { std::stringstream ss2; ss2 << "P2: " << std::max(0, p2Lives); SDL_Surface* surf2 = TTF_RenderText_Solid(gFont, ss2.str().c_str(), gTextColor); if (surf2) { SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surf2); if (tex2) { SDL_Rect dst2 = {SCREEN_WIDTH - surf2->w - 10, 5, surf2->w, surf2->h}; SDL_RenderCopy(renderer, tex2, nullptr, &dst2); SDL_DestroyTexture(tex2); } SDL_FreeSurface(surf2); } }
}

void renderTank(const Tank& tank) {
    if (!tank.active || !tank.texture) return;
    Direction renderDir = (tank.direction == NONE) ? tank.lastValidDirection : tank.direction;
    int spriteRow = 0;
    switch (renderDir) { case DOWN: spriteRow = 0; break; case LEFT: spriteRow = 1; break; case RIGHT: spriteRow = 2; break; case UP: spriteRow = 3; break; case NONE: spriteRow = 0; break; }
    int spriteWidth = tank.isPlayer ? PLAYER_SPRITE_WIDTH : ENEMY_SPRITE_WIDTH;
    int spriteHeight = tank.isPlayer ? PLAYER_SPRITE_HEIGHT : ENEMY_SPRITE_HEIGHT;
    SDL_Rect srcRect = { tank.frame * spriteWidth, spriteRow * spriteHeight, spriteWidth, spriteHeight };
    SDL_Rect destRect = { (int)round(tank.x), (int)round(tank.y), TANK_SIZE, TANK_SIZE };
    SDL_RenderCopy(gRenderer, tank.texture, &srcRect, &destRect);
}

void renderGame() {
    renderMap();
    if (playerTank.active) renderTank(playerTank);
    if (isTwoPlayerMode && player2Tank.active) renderTank(player2Tank);
    for (const auto& enemy : enemyTanks) if (enemy.active) renderTank(enemy);
    renderBullets();
    renderExplosions();
    renderTrees();
    renderLives(gRenderer, playerLives, player2Lives, isTwoPlayerMode);
}

void checkRespawn() {
    Uint32 currentTime = SDL_GetTicks();
    if (!playerTank.active && playerLives > 0 && playerTank.deathTime > 0 && currentTime - playerTank.deathTime >= RESPAWN_DELAY) {
        resetPlayerPosition(playerTank, PLAYER1_START_X, PLAYER1_START_Y);
    }
    if (isTwoPlayerMode && !player2Tank.active && player2Lives > 0 && player2Tank.deathTime > 0 && currentTime - player2Tank.deathTime >= RESPAWN_DELAY) {
        resetPlayerPosition(player2Tank, PLAYER2_START_X, PLAYER2_START_Y);
    }
}

int main(int argc, char* args[]) {
    if (!init()) { std::cerr << "Initialization failed!" << std::endl; SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Init Error", "Failed SDL init. Check console.", nullptr); return -1; }
    if (!loadMedia()) { std::cerr << "Failed to load media!" << std::endl; SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Media Error", "Failed media load. Check console/paths.", gWindow); close(); return -1; }
    if (gMusic) Mix_PlayMusic(gMusic, -1); else std::cerr << "No background music." << std::endl;

    bool quit = false; SDL_Event e;

    while (!quit) {

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            switch (gameState) {
                case MENU: handleMenuInput(e); break;
                case GAME: handleGameInput(e); break;
                case WIN: case LOSE:
                    if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
                        gameState = MENU; Mix_HaltMusic(); if (gMusic) Mix_PlayMusic(gMusic, -1);
                    } break;
            }
        }

        if (gameState == GAME) updateGame();

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255); SDL_RenderClear(gRenderer);
        switch (gameState) {
            case MENU: renderMenu(); break;
            case GAME: renderGame(); break;
            case WIN: renderWin(); break;
            case LOSE: renderLose(); break;
        }
        SDL_RenderPresent(gRenderer);
    }
    close(); return 0;
}
