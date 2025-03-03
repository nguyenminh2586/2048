#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <map>

using namespace std;

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 400;
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int winPoint = 2048;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
int board[GRID_SIZE][GRID_SIZE];

int dirLine[] = {1, 0, -1, 0};
int dirColumn[] = {0, 1, 0, -1};

map<int, SDL_Color> tileColors = {
    {0, {205, 193, 180}}, {2, {238, 228, 218}}, {4, {237, 224, 200}},
    {8, {242, 177, 121}}, {16, {245, 149, 99}}, {32, {246, 124, 95}},
    {64, {246, 94, 59}}, {128, {237, 207, 114}}, {256, {237, 204, 97}},
    {512, {237, 200, 80}}, {1024, {237, 197, 63}}, {2048, {237, 194, 46}}
};

void initSDL() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow("2048 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("arial.ttf", 24);
}

void closeSDL() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
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
    board[pos.first][pos.second] = ( rand() % 10 == 0 ) ? 4 : 2;
}

void newGame() {
    memset(board, 0, sizeof(board));
    addPiece();
}

bool canDoMove(int line, int column, int nextLine, int nextColumn) {
    if (nextLine < 0 || nextColumn < 0 || nextLine >= GRID_SIZE || nextColumn >= GRID_SIZE)
        return false;
    return (board[line][column] == board[nextLine][nextColumn] || board[nextLine][nextColumn] == 0);
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
                        canAddPiece = 1; // Đánh dấu có di chuyển hợp lệ
                    } else if (board[nextI][nextJ] == board[i][j]) {
                        board[nextI][nextJ] += board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1; // Đánh dấu có hợp nhất
                    }
                }
            }
    } while (movePossible);
    if (canAddPiece)
        addPiece(); // Sinh số ngẫu nhiên nếu có di chuyển hợp lệ
}

void render() {
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            SDL_Rect tile = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE - 5, TILE_SIZE - 5};
            SDL_Color color = tileColors[board[i][j]];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderFillRect(renderer, &tile);

            if (board[i][j] != 0) {
                SDL_Surface* textSurface = TTF_RenderText_Solid(font, to_string(board[i][j]).c_str(), {0, 0, 0});
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_Rect textRect = {j * TILE_SIZE + TILE_SIZE / 4, i * TILE_SIZE + TILE_SIZE / 4, textSurface->w, textSurface->h};
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

bool isGameOver() {
    for (int i = 0; i < GRID_SIZE; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            if (board[i][j] == 0)
                return false;
    for (int i = 0; i < GRID_SIZE; i++)
        for(int j = 0; j < GRID_SIZE; j++)
            for(int dir = 0; dir < GRID_SIZE; dir++)
            {
                int nextI = i + dirLine[dir];
                int nextJ = j + dirColumn[dir];
                if(nextI > 0 && nextJ > 0 && nextI < GRID_SIZE && nextJ < GRID_SIZE)
                {
                    if(board[i][j]==board[nextI][nextJ])
                        return false;
                }
            }
    return true;
}

bool hasWon() {
    for (int i = 0; i < GRID_SIZE; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            if (board[i][j] == winPoint)
                return true;
    return false;
}

void gameLoop() {
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w: applyMove(2); break;
                    case SDLK_UP: applyMove(2); break;
                    case SDLK_s: applyMove(0); break;
                    case SDLK_DOWN: applyMove(0); break;
                    case SDLK_a: applyMove(3); break;
                    case SDLK_LEFT: applyMove(3); break;
                    case SDLK_d: applyMove(1); break;
                    case SDLK_RIGHT: applyMove(1); break;
                    case SDLK_n: newGame(); break;

                }
            }
        }
        render();
        if (isGameOver()) {
            cout << "Game Over!" << endl;
            running = false;
        }
        if (hasWon()) {
            cout << "You Win!" << endl;
            running = false;
        }
    }
}

int main() {
    srand(time(0));
    initSDL();
    newGame();
    gameLoop();
    closeSDL();
    return 0;
}
