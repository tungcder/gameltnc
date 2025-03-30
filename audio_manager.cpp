#include "audio_manager.h"
#include <iostream>

Mix_Music* AudioManager::gMusic = nullptr;
Mix_Chunk* AudioManager::gBulletHitSound = nullptr;
Mix_Chunk* AudioManager::gGameOverSound = nullptr;

bool AudioManager::loadMedia() {
    gMusic = Mix_LoadMUS("sound/intro.mp3");
    if (!gMusic) {
        std::cout << "Failed to load intro music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    gBulletHitSound = Mix_LoadWAV("sound/noo.wav");
    if (!gBulletHitSound) {
        std::cout << "Failed to load bullet hit sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    gGameOverSound = Mix_LoadWAV("sound/gameover.wav");
    if (!gGameOverSound) {
        std::cout << "Failed to load game over sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return false;
    }
    return true;
}

void AudioManager::freeMedia() {
    Mix_FreeChunk(gBulletHitSound);
    Mix_FreeChunk(gGameOverSound);
    gBulletHitSound = nullptr;
    gGameOverSound = nullptr;
    Mix_FreeMusic(gMusic);
    gMusic = nullptr;
}
