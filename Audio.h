#ifndef AUDIO_H
#define AUDIO_H

#include <SDL_mixer.h>

// Âm thanh
extern Mix_Chunk* moveSound;
extern Mix_Chunk* mergeSound;
extern Mix_Chunk* winSound;
extern Mix_Chunk* loseSound;
extern Mix_Music* backgroundMusic;

bool loadSounds();
void closeSounds();
void controlBackgroundMusic(bool playMusic);

#endif // AUDIO_H 