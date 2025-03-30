#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL_mixer.h>
#include <string>

class AudioManager {
public:
    static bool loadMedia();
    static void freeMedia();
    static Mix_Music* gMusic;
    static Mix_Chunk* gBulletHitSound;
    static Mix_Chunk* gGameOverSound;
};

#endif
