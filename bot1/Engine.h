#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
using namespace std;

struct Move {
    char action;
    int sx, sy, ex, ey;
};

extern int board[9][9];
extern int layer[9][9];

// 初始化
void initBoard();

// IO
void writeBoard();
void writeLayer();
vector<Move> readOutput();



// bot
void callBot(string bot, string color, int round);

// 遊戲邏輯
void applyMove(Move m);



bool hasPiece(int type);
bool canEat(string color);
bool checkGameOver(string &winner);


#endif