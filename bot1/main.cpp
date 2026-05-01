#include <iostream>
#include "Engine.h"
#include<string>
#include<fstream>
using namespace std;

string bot1 = "./bot1";
string bot2 = "./bot2";

int main() {
    initBoard();
    ofstream("stepHistory.txt"); // 清空

    for(int round = 1; round <= 20; round++) {

    cout << "Round " << round << endl;

        string winner;

        // ===== White =====
        writeBoard();
        writeLayer();
        callBot(bot1, "White", round);

        vector<Move> moves = readOutput();

        applyMove(moves[0]);

        if(checkGameOver(winner)) {
            cout << winner << " wins!\n";
            break;
        }

        applyMove(moves[1]);

        if(checkGameOver(winner)) {
            cout << winner << " wins!\n";
            break;
        }

        // ===== Black =====
        writeBoard();
        writeLayer();
        callBot(bot2, "Black", round);

        moves = readOutput();
        applyMove(moves[0]);


        if(checkGameOver(winner)) {
            cout << winner << " wins!\n";
            break;
        }

        applyMove(moves[1]);

        if(checkGameOver(winner)) {
            cout << winner << " wins!\n";
            break;
        }
    }
}