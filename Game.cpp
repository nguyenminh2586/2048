#include "Game.h"
#include "Renderer.h"
#include "Constants.h"
#include "Button.h"
#include "Board.h"
#include "Animation.h"
#include "Audio.h"
#include "Util.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>

Uint32 lastFrameTime = 0;

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    window = SDL_CreateWindow("2048 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Sử dụng font đậm với kích thước lớn hơn
    font = TTF_OpenFont("ARLRDBD.TTF", 24);
    menuFont = TTF_OpenFont("ARLRDBD.TTF", 28);
    if (!font || !menuFont) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        // Thử dùng font thường nếu không có font đậm
        font = TTF_OpenFont("fonts/arial.ttf", 24);
        menuFont = TTF_OpenFont("fonts/arial.ttf", 28);
    }

    loadSounds();
}

void closeSDL() {
    closeSounds();
    TTF_CloseFont(font);
    TTF_CloseFont(menuFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void gameLoop() {
    bool running = true;
    SDL_Event e;

    // Phát nhạc nền khi ở menu ngay từ đầu
    if (showMenu) {
        controlBackgroundMusic(true);
    }

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                saveGame(); // Lưu game khi thoát
                running = false;
            }
            if (e.type == SDL_MOUSEMOTION) {
                int mouseX = e.motion.x;
                int mouseY = e.motion.y;

                // Kiểm tra di chuột trên các nút menu
                if (showMenu) {
                    newGameButton.highlighted = (mouseX >= newGameButton.rect.x && mouseX <= newGameButton.rect.x + newGameButton.rect.w &&
                                               mouseY >= newGameButton.rect.y && mouseY <= newGameButton.rect.y + newGameButton.rect.h);
                    continueButton.highlighted = (mouseX >= continueButton.rect.x && mouseX <= continueButton.rect.x + continueButton.rect.w &&
                                                mouseY >= continueButton.rect.y && mouseY <= continueButton.rect.y + continueButton.rect.h);
                    exitButton.highlighted = (mouseX >= exitButton.rect.x && mouseX <= exitButton.rect.x + exitButton.rect.w &&
                                            mouseY >= exitButton.rect.y && mouseY <= exitButton.rect.y + exitButton.rect.h);
                } else {
                    // Kiểm tra nút Menu trong game
                    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};
                    menuButtonHighlighted = (mouseX >= menuButtonRect.x && mouseX <= menuButtonRect.x + menuButtonRect.w &&
                                           mouseY >= menuButtonRect.y && mouseY <= menuButtonRect.y + menuButtonRect.h);
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if (showMenu) {
                    processMenuClick(mouseX, mouseY);
                } else {
                    // Kiểm tra nút Menu trong game
                    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};
                    if (mouseX >= menuButtonRect.x && mouseX <= menuButtonRect.x + menuButtonRect.w &&
                        mouseY >= menuButtonRect.y && mouseY <= menuButtonRect.y + menuButtonRect.h) {
                        showMenu = true;
                        // Bật nhạc nền khi quay lại menu
                        controlBackgroundMusic(true);
                    }
                }
            }
            if (e.type == SDL_KEYDOWN && !showMenu && !animating) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: applyMove(2); break;
                    case SDLK_DOWN: applyMove(0); break;
                    case SDLK_LEFT: applyMove(3); break;
                    case SDLK_RIGHT: applyMove(1); break;
                    case SDLK_n:
                        newGame();
                        gameOver = false; // Đảm bảo flag được reset
                        hasWon = false;   // Đảm bảo flag được reset
                        break;
                    case SDLK_ESCAPE:
                        showMenu = true;
                        // Bật nhạc nền khi nhấn ESC để quay lại menu
                        controlBackgroundMusic(true);
                        break;
                }
            }
        }

        // Cập nhật animation nếu đang có
        if (animating) {
            updateAnimations();
        }

        render();

        // Điều chỉnh FPS
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
}
