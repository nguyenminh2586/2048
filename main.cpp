#define SDL_MAIN_HANDLED
#include "Game.h"
#include "Board.h"
#include "Constants.h"
#include <ctime>
#include <cstdlib>
#include <cstring>

int main() {
    srand(time(0));
    initSDL();
    memset(board, 0, sizeof(board));
    memset(previousBoard, 0, sizeof(previousBoard));
    addPiece();
    gameLoop();
    closeSDL();
    return 0;
}
