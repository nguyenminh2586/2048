#include "Board.h"
#include "Constants.h"
#include "Animation.h"
#include "Audio.h"
#include <cstdlib>
#include <cstring>
#include <SDL.h>

int board[GRID_SIZE][GRID_SIZE];
int previousBoard[GRID_SIZE][GRID_SIZE];
int score = 0;
int bestScore = 0;
bool hasWon = false;
bool gameOver = false;
bool showMenu = true;

int dirLine[] = {1, 0, -1, 0};
int dirColumn[] = {0, 1, 0, -1};

std::pair<int, int> generateUnoccupiedPosition() {
    int line, column;
    do {
        line = rand() % GRID_SIZE;
        column = rand() % GRID_SIZE;
    } while (board[line][column] != 0);
    return {line, column};
}

void addPiece() {
    std::pair<int, int> pos = generateUnoccupiedPosition();
    board[pos.first][pos.second] = (rand() % 10 == 0) ? 4 : 2;

    // Thêm animation cho block mới
    Animation newTileAnim;
    newTileAnim.value = board[pos.first][pos.second];
    newTileAnim.startX = BOARD_X + pos.second * TILE_SIZE;
    newTileAnim.startY = BOARD_Y + pos.first * TILE_SIZE;
    newTileAnim.targetX = newTileAnim.startX;
    newTileAnim.targetY = newTileAnim.startY;
    newTileAnim.currentX = newTileAnim.startX;
    newTileAnim.currentY = newTileAnim.startY;
    newTileAnim.active = true;
    newTileAnim.isNew = true;
    newTileAnim.isMerging = false;
    newTileAnim.scale = 0.1f;
    newTileAnim.framesLeft = ANIMATION_DURATION;
    animations.push_back(newTileAnim);

    animating = true;
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
    if (animating) return; // Không cho phép di chuyển khi đang animation

    // Lưu trạng thái bảng trước khi di chuyển
    memcpy(previousBoard, board, sizeof(board));

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
    bool hasMerged = false; // Biến cờ kiểm tra xem đã có merge hay chưa

    // Map để theo dõi các ô đã hợp nhất trong lượt này
    bool merged[GRID_SIZE][GRID_SIZE] = {false};

    do {
        movePossible = 0;
        for (int i = startLine; i >= 0 && i < GRID_SIZE; i += lineStep)
            for (int j = startColumn; j >= 0 && j < GRID_SIZE; j += columnStep) {
                if (board[i][j] == 0) continue;

                int nextI = i + dirLine[direction];
                int nextJ = j + dirColumn[direction];

                if (nextI >= 0 && nextI < GRID_SIZE && nextJ >= 0 && nextJ < GRID_SIZE) {
                    if (board[nextI][nextJ] == 0) {
                        // Di chuyển
                        board[nextI][nextJ] = board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1;

                        // Thêm animation di chuyển
                        Animation moveAnim;
                        moveAnim.value = board[nextI][nextJ];
                        moveAnim.startX = BOARD_X + j * TILE_SIZE;
                        moveAnim.startY = BOARD_Y + i * TILE_SIZE;
                        moveAnim.targetX = BOARD_X + nextJ * TILE_SIZE;
                        moveAnim.targetY = BOARD_Y + nextI * TILE_SIZE;
                        moveAnim.currentX = moveAnim.startX;
                        moveAnim.currentY = moveAnim.startY;
                        moveAnim.active = true;
                        moveAnim.isNew = false;
                        moveAnim.isMerging = false;
                        moveAnim.scale = 1.0f;
                        moveAnim.framesLeft = ANIMATION_DURATION;
                        animations.push_back(moveAnim);

                    } else if (board[nextI][nextJ] == board[i][j] && !merged[nextI][nextJ]) {
                        // Hợp nhất
                        board[nextI][nextJ] *= 2;
                        score += board[nextI][nextJ];
                        if (score > bestScore) bestScore = score;
                        board[i][j] = 0;
                        merged[nextI][nextJ] = true;
                        movePossible = 1;
                        canAddPiece = 1;
                        hasMerged = true; // Đã có merge

                        // Thêm animation di chuyển và hợp nhất
                        Animation mergeAnim;
                        mergeAnim.value = board[nextI][nextJ] / 2; // Giá trị ban đầu (trước khi hợp nhất)
                        mergeAnim.startX = BOARD_X + j * TILE_SIZE;
                        mergeAnim.startY = BOARD_Y + i * TILE_SIZE;
                        mergeAnim.targetX = BOARD_X + nextJ * TILE_SIZE;
                        mergeAnim.targetY = BOARD_Y + nextI * TILE_SIZE;
                        mergeAnim.currentX = mergeAnim.startX;
                        mergeAnim.currentY = mergeAnim.startY;
                        mergeAnim.active = true;
                        mergeAnim.isNew = false;
                        mergeAnim.isMerging = true;
                        mergeAnim.scale = 1.0f;
                        mergeAnim.framesLeft = ANIMATION_DURATION;
                        animations.push_back(mergeAnim);
                    }
                }
            }
    } while (movePossible);

    if (canAddPiece) {
        animating = true;

        // Phát âm thanh di chuyển
        if (moveSound) {
            Mix_PlayChannel(-1, moveSound, 0);
        }

        // Phát âm thanh hợp nhất nếu có
        if (hasMerged && mergeSound) {
            Mix_PlayChannel(-1, mergeSound, 0);
        }

        // Tạo ô mới sau khi hoàn thành animation di chuyển
        SDL_Delay(200); // Đợi một chút trước khi thêm ô mới
        addPiece();

        // Kiểm tra chiến thắng (đã đạt 2048)
        bool justWon = false;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (board[i][j] >= WIN_POINT && !hasWon) {
                    hasWon = true;
                    justWon = true;
                    break;
                }
            }
            if (justWon) break;
        }

        // Phát âm thanh chiến thắng
        if (justWon && winSound) {
            Mix_PlayChannel(-1, winSound, 0);
        }

        // Kiểm tra trò chơi kết thúc
        if (!movesAvailable() && !gameOver) {
            gameOver = true;
            // Phát âm thanh thua cuộc
            if (loseSound) {
                Mix_PlayChannel(-1, loseSound, 0);
            }
        }
    }
}
