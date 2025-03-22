#ifndef BOARD_H
#define BOARD_H

#include <utility>

extern int board[4][4];
extern int previousBoard[4][4];
extern int score;
extern int bestScore;
extern bool hasWon;
extern bool gameOver;
extern bool showMenu;

std::pair<int, int> generateUnoccupiedPosition();
void addPiece();
void applyMove(int direction);
bool canDoMove(int line, int column, int nextLine, int nextColumn);
bool movesAvailable();

#endif // BOARD_H 