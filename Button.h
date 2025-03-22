#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <SDL.h>

// Cấu trúc cho các nút menu
struct Button {
    SDL_Rect rect;
    std::string text;
    bool highlighted;
};

extern Button newGameButton;
extern Button continueButton;
extern Button exitButton;
extern bool menuButtonHighlighted;

void renderButton(Button &button);
void processMenuClick(int x, int y);

#endif // BUTTON_H 