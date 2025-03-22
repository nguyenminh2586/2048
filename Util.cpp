#include "Util.h"
#include "Board.h"
#include "Constants.h"
#include "Audio.h"
#include "Animation.h"
#include <fstream>
#include <cstring>

void newGame() {
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = false;
    hasWon = false;
    showMenu = false;
    animating = false;
    animations.clear();
    addPiece();

    // Tắt nhạc nền khi bắt đầu game
    controlBackgroundMusic(false);
}

// Lưu trạng thái trò chơi
void saveGame() {
    std::ofstream saveFile(SAVE_FILE, std::ios::binary);
    if (saveFile.is_open()) {
        saveFile.write((char*)board, sizeof(board));
        saveFile.write((char*)&score, sizeof(score));
        saveFile.write((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
    }
}

// Tải trạng thái trò chơi đã lưu
bool loadGame() {
    std::ifstream saveFile(SAVE_FILE, std::ios::binary);
    if (saveFile.is_open()) {
        saveFile.read((char*)board, sizeof(board));
        saveFile.read((char*)&score, sizeof(score));
        saveFile.read((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
        gameOver = false;
        hasWon = false;
        showMenu = false;
        animating = false;
        animations.clear();

        // Tắt nhạc nền khi tiếp tục game
        controlBackgroundMusic(false);

        return true;
    }
    return false;
}
