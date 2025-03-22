#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <map>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern TTF_Font* font;
extern TTF_Font* menuFont;

// Bảng màu cho các ô
extern std::map<int, SDL_Color> tileColors;

void renderText(const std::string &text, int x, int y, TTF_Font* usedFont = nullptr);
void renderRoundedRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, SDL_Color color);
void renderTile(int value, int x, int y, float scale = 1.0f);
void renderBoard();
void renderMenu();
void renderScore();
void render();

#endif // RENDERER_H 