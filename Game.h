#ifndef GAME_H
#define GAME_H
#include <SDL.h>
extern Uint32 lastFrameTime;

void initSDL();
void closeSDL();
void gameLoop();

#endif // GAME_H
