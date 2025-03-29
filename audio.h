#ifndef AUDIO_H
#define AUDIO_H

#include <SDL_mixer.h>

extern Mix_Music* gMusic;
extern Mix_Chunk* gBulletHitSound;
extern Mix_Chunk* gGameOverSound;

// Khai báo hàm
bool loadMedia();

#endif
