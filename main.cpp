#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <map>
#include <fstream>

using namespace std;

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int BOARD_X = 50;
const int BOARD_Y = 150;
const int winPoint = 2048;
const string SAVE_FILE = "2048_save.dat";

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
TTF_Font* menuFont = nullptr;
int board[GRID_SIZE][GRID_SIZE];
int score = 0;
int bestScore = 0;
bool showMenu = true;
bool gameOver = false;

int dirLine[] = {1, 0, -1, 0};
int dirColumn[] = {0, 1, 0, -1};
map<int, SDL_Color> tileColors = {
    {0, {205, 193, 180}}, {2, {238, 228, 218}}, {4, {237, 224, 200}},
    {8, {242, 177, 121}}, {16, {245, 149, 99}}, {32, {246, 124, 95}},
    {64, {246, 94, 59}}, {128, {237, 207, 114}}, {256, {237, 204, 97}},
    {512, {237, 200, 80}}, {1024, {237, 197, 63}}, {2048, {237, 194, 46}}
};

// Cấu trúc cho các nút menu
struct Button {
    SDL_Rect rect;
    string text;
    bool highlighted;
};

Button newGameButton = {{150, 250, 200, 50}, "New Game", false};
Button continueButton = {{150, 320, 200, 50}, "Continue", false};
Button exitButton = {{150, 390, 200, 50}, "Exit", false};

void renderText(const string &text, int x, int y, TTF_Font* usedFont = nullptr) {
    if (usedFont == nullptr) usedFont = font;
    SDL_Surface* textSurface = TTF_RenderText_Solid(usedFont, text.c_str(), {0, 0, 0});
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderBoard() {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            SDL_Rect tile = {BOARD_X + j * TILE_SIZE, BOARD_Y + i * TILE_SIZE, TILE_SIZE - 5, TILE_SIZE - 5};
            SDL_Color color = tileColors[board[i][j]];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(renderer, &tile);
            if (board[i][j] != 0) {
                renderText(to_string(board[i][j]), BOARD_X + j * TILE_SIZE + TILE_SIZE / 3, BOARD_Y + i * TILE_SIZE + TILE_SIZE / 3);
            }
        }
    }
}

void renderButton(Button &button) {
    // Màu nền nút
    if (button.highlighted) {
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    }
    SDL_RenderFillRect(renderer, &button.rect);

    // Viền nút
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &button.rect);

    // Văn bản nút
    int textX = button.rect.x + (button.rect.w - button.text.length() * 12) / 2;
    int textY = button.rect.y + (button.rect.h - 24) / 2;
    renderText(button.text, textX, textY, menuFont);
}

void renderMenu() {
    // Tiêu đề menu
    renderText("2048 Game", 180, 150, menuFont);

    // Vẽ các nút
    renderButton(newGameButton);
    renderButton(continueButton);
    renderButton(exitButton);
}

void renderScore() {
    renderText("Score: " + to_string(score), 50, 50);
    renderText("Best: " + to_string(bestScore), 250, 50);

    // Thêm nút Menu
    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderFillRect(renderer, &menuButtonRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &menuButtonRect);
    renderText("Menu", SCREEN_WIDTH - 85, 55);
}

pair<int, int> generateUnoccupiedPosition() {
    int line, column;
    do {
        line = rand() % GRID_SIZE;
        column = rand() % GRID_SIZE;
    } while (board[line][column] != 0);
    return {line, column};
}

void addPiece() {
    pair<int, int> pos = generateUnoccupiedPosition();
    board[pos.first][pos.second] = (rand() % 10 == 0) ? 4 : 2;
}

void newGame() {
    memset(board, 0, sizeof(board));
    score = 0;
    addPiece();
    gameOver = false;
    showMenu = false;
}

// Lưu trạng thái trò chơi
void saveGame() {
    ofstream saveFile(SAVE_FILE, ios::binary);
    if (saveFile.is_open()) {
        saveFile.write((char*)board, sizeof(board));
        saveFile.write((char*)&score, sizeof(score));
        saveFile.write((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
    }
}

// Tải trạng thái trò chơi đã lưu
bool loadGame() {
    ifstream saveFile(SAVE_FILE, ios::binary);
    if (saveFile.is_open()) {
        saveFile.read((char*)board, sizeof(board));
        saveFile.read((char*)&score, sizeof(score));
        saveFile.read((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
        gameOver = false;
        showMenu = false;
        return true;
    }
    return false;
}

bool canDoMove(int line, int column, int nextLine, int nextColumn) {
    if (nextLine < 0 || nextColumn < 0 || nextLine >= GRID_SIZE || nextColumn >= GRID_SIZE)
        return false;
    return (board[line][column] == board[nextLine][nextColumn] || board[nextLine][nextColumn] == 0);
}

// Kiểm tra xem còn nước đi không
bool movesAvailable() {
    // Kiểm tra xem còn ô trống không
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == 0) return true;
        }
    }

    // Kiểm tra xem có thể hợp nhất không
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            for (int dir = 0; dir < 4; dir++) {
                int nextI = i + dirLine[dir];
                int nextJ = j + dirColumn[dir];
                if (nextI >= 0 && nextI < GRID_SIZE && nextJ >= 0 && nextJ < GRID_SIZE) {
                    if (board[i][j] == board[nextI][nextJ]) return true;
                }
            }
        }
    }

    return false;
}

void applyMove(int direction) {
    int startLine = 0, startColumn = 0, lineStep = 1, columnStep = 1;
    if (direction == 0) {
        startLine = GRID_SIZE - 1;
        lineStep = -1;
    }
    if (direction == 1) {
        startColumn = GRID_SIZE - 1;
        columnStep = -1;
    }
    int movePossible, canAddPiece = 0;
    do {
        movePossible = 0;
        for (int i = startLine; i >= 0 && i < GRID_SIZE; i += lineStep)
            for (int j = startColumn; j >= 0 && j < GRID_SIZE; j += columnStep) {
                int nextI = i + dirLine[direction];
                int nextJ = j + dirColumn[direction];
                if (board[i][j] && canDoMove(i, j, nextI, nextJ)) {
                    if (board[nextI][nextJ] == 0) {
                        board[nextI][nextJ] = board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1;
                    } else if (board[nextI][nextJ] == board[i][j]) {
                        board[nextI][nextJ] += board[i][j];
                        score += board[nextI][nextJ];
                        if (score > bestScore) bestScore = score;
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1;
                    }
                }
            }
    } while (movePossible);

    if (canAddPiece) {
        addPiece();

        // Kiểm tra trò chơi kết thúc
        if (!movesAvailable()) {
            gameOver = true;
        }
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);
    SDL_RenderClear(renderer);

    if (showMenu) {
        renderMenu();
    } else {
        renderScore();
        renderBoard();

        // Hiển thị thông báo game over nếu cần
        if (gameOver) {
            SDL_Rect gameOverRect = {50, 550, 400, 40};
            SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
            SDL_RenderFillRect(renderer, &gameOverRect);
            renderText("Game Over! Press 'N' for New Game", 60, 560);
        }
    }

    SDL_RenderPresent(renderer);
}

void processMenuClick(int x, int y) {
    // Kiểm tra click vào nút New Game
    if (x >= newGameButton.rect.x && x <= newGameButton.rect.x + newGameButton.rect.w &&
        y >= newGameButton.rect.y && y <= newGameButton.rect.y + newGameButton.rect.h) {
        newGame();
    }
    // Kiểm tra click vào nút Continue
    else if (x >= continueButton.rect.x && x <= continueButton.rect.x + continueButton.rect.w &&
             y >= continueButton.rect.y && y <= continueButton.rect.y + continueButton.rect.h) {
        if (!loadGame()) {
            newGame(); // Nếu không có game đã lưu, bắt đầu game mới
        }
    }
    // Kiểm tra click vào nút Exit
    else if (x >= exitButton.rect.x && x <= exitButton.rect.x + exitButton.rect.w &&
             y >= exitButton.rect.y && y <= exitButton.rect.y + exitButton.rect.h) {
        saveGame(); // Lưu game trước khi thoát
        exit(0);
    }
}

void gameLoop() {
    bool running = true;
    SDL_Event e;
    while (running) {
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
                    if (mouseX >= menuButtonRect.x && mouseX <= menuButtonRect.x + menuButtonRect.w &&
                        mouseY >= menuButtonRect.y && mouseY <= menuButtonRect.y + menuButtonRect.h) {
                        // Highlight nút Menu nếu cần
                    }
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
                    }
                }
            }
            if (e.type == SDL_KEYDOWN && !showMenu) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: applyMove(2); break;
                    case SDLK_DOWN: applyMove(0); break;
                    case SDLK_LEFT: applyMove(3); break;
                    case SDLK_RIGHT: applyMove(1); break;
                    case SDLK_n: newGame(); break;
                    case SDLK_ESCAPE: showMenu = true; break;
                }
            }
        }
        render();
    }
}

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow("2048 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("arial.ttf", 24);
    menuFont = TTF_OpenFont("arial.ttf", 28);
    if (!font || !menuFont) {
        cout << "Failed to load font: " << TTF_GetError() << endl;
    }
}

void closeSDL() {
    TTF_CloseFont(font);
    TTF_CloseFont(menuFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    srand(time(0));
    initSDL();
    memset(board, 0, sizeof(board));
    addPiece();
    gameLoop();
    closeSDL();
    return 0;
}
