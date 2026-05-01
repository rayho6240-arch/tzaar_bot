#include <bits/stdc++.h>
using namespace std;

struct Move {
    char action;
    int sx, sy, ex, ey;
};

int board[9][9];
int layer[9][9];

int dx[6] = {1, -1, 0, 0, 1, -1};
int dy[6] = {0, 0, 1, -1, 1, -1};

// ===== 基本工具 =====
bool isOnBoard(int r, int c) {
    if (r < 0 || r >= 9 || c < 0 || c >= 9) return false;
    if (board[r][c] == -1) return false;
    return true;
}

bool isStraight(int sx,int sy,int ex,int ey){
    int dx = ex - sx;
    int dy = ey - sy;
    return (dx==0 || dy==0 || dx+dy==0);
}

// ===== 套用走法 =====
void applyMove(Move m) {
    if (m.action == 'P') return;

    if (m.action == 'S') {
        layer[m.ex][m.ey] += layer[m.sx][m.sy];
    } else { // Eat
        layer[m.ex][m.ey] = layer[m.sx][m.sy];
    }

    board[m.ex][m.ey] = board[m.sx][m.sy];
    board[m.sx][m.sy] = 0;
    layer[m.sx][m.sy] = 0;
}

// ===== 合法性判斷 =====
bool isValid(Move m, string color, bool mustEat) {

    if (!isOnBoard(m.sx, m.sy) || !isOnBoard(m.ex, m.ey)) return false;
    if (!isStraight(m.sx, m.sy, m.ex, m.ey)) return false;

    int src = board[m.sx][m.sy];
    int dst = board[m.ex][m.ey];

    // 顏色
    if (color == "White" && (src < 1 || src > 3)) return false;
    if (color == "Black" && (src < 4 || src > 6)) return false;

    // ===== Eat =====
    if (m.action == 'E') {
        if (dst == 0) return false;

        if (color == "White" && dst <= 3) return false;
        if (color == "Black" && dst >= 4) return false;

        if (layer[m.sx][m.sy] < layer[m.ex][m.ey]) return false;

        return true;
    }

    // ===== Stack =====
    if (m.action == 'S') {
        if (mustEat) return false;

        if (dst == 0) return false;

        if (color == "White" && dst > 3) return false;
        if (color == "Black" && dst < 4) return false;

        return true;
    }

    return false;
}

// ===== 找所有合法步 =====
vector<Move> getMoves(string color, bool mustEat) {
    vector<Move> res;

    int myMin = (color == "White") ? 1 : 4;
    int myMax = (color == "White") ? 3 : 6;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {

            if (board[i][j] < myMin || board[i][j] > myMax) continue;

            for (int d = 0; d < 6; d++) {

                for (int step = 1; step < 9; step++) {

                    int ni = i + dx[d] * step;
                    int nj = j + dy[d] * step;

                    if (!isOnBoard(ni, nj)) break;

                    Move m;

                    // ===== 遇到棋子 =====
                    if (board[ni][nj] != 0) {

                        // Eat
                        m = {'E', i, j, ni, nj};
                        if (isValid(m, color, mustEat)) res.push_back(m);

                        // Stack
                        if (!mustEat) {
                            m = {'S', i, j, ni, nj};
                            if (isValid(m, color, false)) res.push_back(m);
                        }

                        break; // 不能穿過
                    }

                    // ===== 空格 =====
                    if (!mustEat) {
                        m = {'S', i, j, ni, nj};
                        if (isValid(m, color, false)) res.push_back(m);
                    }
                }
            }
        }
    }
    return res;
}

// ===== 座標轉換 =====
Move transformMove(Move m) {
    if (m.action == 'P') return m;

    int nsx = m.sy - 4;
    int nsy = -m.sx + 4;
    int nex = m.ey - 4;
    int ney = -m.ex + 4;

    return {m.action, nsx, nsy, nex, ney};
}

// ===== IO =====
void readBoard(string path) {
    ifstream fin(path);
    string line;
    for(int i=0;i<9;i++){
        getline(fin,line);
        stringstream ss(line);
        for(int j=0;j<9;j++){
            string val;
            getline(ss,val,',');
            board[i][j]=stoi(val);
        }
    }
}

void readLayer(string path) {
    ifstream fin(path);
    string line;
    for(int i=0;i<9;i++){
        getline(fin,line);
        stringstream ss(line);
        for(int j=0;j<9;j++){
            string val;
            getline(ss,val,',');
            layer[i][j]=stoi(val);
        }
    }
}

int readLastRound(const string& path) {
    ifstream fin(path);
    string line,last;

    while(getline(fin,line)){
        if(!line.empty()) last=line;
    }

    if(last.empty()) return 1;

    stringstream ss(last);
    int r;
    ss>>r;
    return r+1;
}

void writeOutput(Move m1, Move m2) {
    ofstream fout("out.txt");
    fout<<m1.action<<","<<m1.sx<<","<<m1.sy<<","<<m1.ex<<","<<m1.ey<<"\n";
    fout<<m2.action<<","<<m2.sx<<","<<m2.sy<<","<<m2.ex<<","<<m2.ey<<"\n";
}

void appendHistory(int round, string color, int step, Move m) {
    ofstream fout("stepHistory.txt", ios::app);

    fout<<round<<","
        <<(color=="White"?"W":"B")<<","
        <<step<<","
        <<m.action<<","
        <<m.sx<<","<<m.sy<<","
        <<m.ex<<","<<m.ey<<"\n";
}

// ===== 主程式 =====
int main(int argc, char* argv[]) {

    if (argc < 5) return 1;

    string color = argv[1];
    readBoard(argv[3]);
    readLayer(argv[4]);

    srand(time(0));

    Move m1 = {'P',0,0,0,0};
    Move m2 = {'P',0,0,0,0};

    int Round = readLastRound("stepHistory.txt");

    // ===== 第一手 =====
    vector<Move> eats1 = getMoves(color, true);

    if (!eats1.empty()) {
        m1 = eats1[rand()%eats1.size()];
    }

    // ===== 白棋第一回合 =====
    if (Round==1 && color=="White") {
        m2 = {'P',0,0,0,0};
    }
    else {
        applyMove(m1);

        vector<Move> moves2 = getMoves(color, false);

        if (!moves2.empty()) {
            m2 = moves2[rand()%moves2.size()];
        }
    }

    appendHistory(Round, color, 1, m1);
    appendHistory(Round, color, 2, m2);

    writeOutput(transformMove(m1), transformMove(m2));

    return 0;
}