#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int BOARD_X = 50;
const int BOARD_Y = 150;
const int WIN_POINT = 2048;
const std::string SAVE_FILE = "2048_save.dat";
const int ANIMATION_SPEED = 50;  // Tốc độ animation (pixels per frame)
const int ANIMATION_DURATION = 4; // Số frame cho mỗi animation
const float SCALE_SPEED = 0.5f;  // Tốc độ hiệu ứng scale
const int FPS = 144;
const int FRAME_DELAY = 1000 / FPS;

// Mảng hướng di chuyển
extern int dirLine[];
extern int dirColumn[];

#endif // CONSTANTS_H 