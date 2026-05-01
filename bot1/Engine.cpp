#include "Engine.h"
#include <fstream>
#include <iostream>
#include <cstdlib>

int board[9][9];
int layer[9][9];

// ===== 初始化 =====
void initBoard() {
    int init[9][9] = {
        {3,3,3,3,6,-1,-1,-1,-1},
        {6,2,2,2,5,6,-1,-1,-1},
        {6,5,1,1,4,5,6,-1,-1},
        {6,5,4,3,6,4,5,6,-1},
        {6,5,4,6,-1,3,1,2,3},
        {-1,3,2,1,3,6,1,2,3},
        {-1,-1,3,2,1,4,4,2,3},
        {-1,-1,-1,3,2,5,5,5,3},
        {-1,-1,-1,-1,3,6,6,6,6}
    };

    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){
            board[i][j] = init[i][j];
            if(init[i][j] == -1) layer[i][j] = -1;
            else if(init[i][j] == 0) layer[i][j] = 0;
            else layer[i][j] = 1;
        }
    }
}

// ===== IO =====
void writeBoard() {
    ofstream fout("board.txt");
    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){
            fout << board[i][j];
            if(j<8) fout << ",";
        }
        fout << "\n";
    }
}

void writeLayer() {
    ofstream fout("chessLayer.txt");
    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){
            fout << layer[i][j];
            if(j<8) fout << ",";
        }
        fout << "\n";
    }
}

vector<Move> readOutput() {
    vector<Move> moves;
    ifstream fin("out.txt");

    for(int i=0;i<2;i++){
        Move m;
        char comma;
        fin >> m.action >> comma
            >> m.sx >> comma >> m.sy >> comma
            >> m.ex >> comma >> m.ey;
        moves.push_back(m);
    }
    return moves;
}

// ===== 呼叫 bot =====
void callBot(string bot, string color, int round) {
    string cmd = bot + " " + color + " " + to_string(round)
               + " board.txt chessLayer.txt stepHistory.txt";
    system(cmd.c_str());
}


// ===== 套用 move =====
void applyMove(Move m) {
    if(m.action == 'P') return;

    if(m.action == 'E') {
        board[m.ex][m.ey] = board[m.sx][m.sy];
        layer[m.ex][m.ey] = layer[m.sx][m.sy] + layer[m.ex][m.ey];
        board[m.sx][m.sy] = 0;
        layer[m.sx][m.sy] = 0;
    }

    if(m.action == 'S') {
        layer[m.ex][m.ey] += layer[m.sx][m.sy];
        board[m.ex][m.ey] = board[m.sx][m.sy];
        board[m.sx][m.sy] = 0;
        layer[m.sx][m.sy] = 0;
    }
}






//判斷輸贏
bool hasPiece(int type) {
    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){
            if(board[i][j] == type)
                return true;
        }
    }
    return false;
}

bool canEat(string color) {

    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){

            if(board[i][j] <= 0) continue;

            // 自己棋
            if(color=="White" && board[i][j] > 3) continue;
            if(color=="Black" && board[i][j] < 4) continue;

            // 嘗試周圍6方向
            int dx[6]={1,-1,0,0,1,-1};
            int dy[6]={0,0,1,-1,-1,1};

            for(int d=0;d<6;d++){
                for(int step=1;step<9;step++){

                    int ni = i + dx[d]*step;
                    int nj = j + dy[d]*step;

                    if(ni<0||ni>=9||nj<0||nj>=9) break;
                    if(board[ni][nj]==-1) break;

                    // 遇到棋
                    if(board[ni][nj] != 0){

                        // 對手棋
                        if(color=="White" && board[ni][nj] > 3 &&
                           layer[i][j] >= layer[ni][nj])
                            return true;

                        if(color=="Black" && board[ni][nj] < 4 &&
                           layer[i][j] >= layer[ni][nj])
                            return true;

                        break;
                    }
                }
            }
        }
    }
    return false;
}


bool checkGameOver(string &winner) {

    // White 三種棋
    if(!hasPiece(1) || !hasPiece(2) || !hasPiece(3)){
        winner = "Black";
        return true;
    }

    // Black 三種棋
    if(!hasPiece(4) || !hasPiece(5) || !hasPiece(6)){
        winner = "White";
        return true;
    }

    // 無法吃子
    if(!canEat("White")){
        winner = "Black";
        return true;
    }

    if(!canEat("Black")){
        winner = "White";
        return true;
    }

    return false;
}