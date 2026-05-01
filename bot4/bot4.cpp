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
void applyMove(int board[9][9], int layer[9][9], Move m) {
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

// ===== 合法判斷 =====
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






void copy(int dest[9][9], int src[9][9], int size) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            dest[i][j] = src[i][j];
        }
    }
}



//簡易版evaluate函數，評分方式是：我方沙皇棋子數量 - 對方沙皇棋子數量
// 權重設定：沙皇(Tzaar) > 沙後(Tzarra) > 沙兵(Totts)// 同時要考慮疊棋高度
int evaluate(int b[9][9], int l[9][9], string myColor) {
    int score = 0;
    int whiteScore = 0;
    int blackScore = 0;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (b[i][j] <= 0) continue;
            
            int val = 0;
            int type = b[i][j];
            // 分配權重 (這部分你可以根據測試調整)
            if (type == 1 || type == 4) val = 100 + l[i][j] * 10; // Tzaar
            else if (type == 2 || type == 5) val = 50 + l[i][j] * 10; // Tzarra
            else if (type == 3 || type == 6) val = 20 + l[i][j] * 10; // Totts

            if (type <= 3) whiteScore += val;
            else blackScore += val;
        }
    }
    
    // 如果是我的顏色，回傳正分
    if (myColor == "White") return whiteScore - blackScore;
    else return blackScore - whiteScore;
}


//核心 Minimax (含 Alpha-Beta 剪枝)
const int INF = 1e9;
int minimax(int depth, int alpha, int beta, string currentColor, int step, bool isMax, int b[9][9], int l[9][9], string myColor) {
    if (depth == 0) return evaluate(b, l, myColor);

    // 取得當前步驟的合法移動
    bool mustEat = (step == 1);
    vector<Move> moves = getMoves(currentColor, mustEat);
    
    // 如果是第二步，增加一個 Pass 的選項
    if (step == 2) {
        moves.push_back({'P', 0, 0, 0, 0});
    }

    if (moves.empty()) {
        // 如果第一步就沒法吃，代表輸了，回傳極小值
        return isMax ? -INF + 100 : INF - 100;
    }

    if (isMax) {
        int maxEval = -INF;
        for (auto& m : moves) {
            int nextB[9][9], nextL[9][9];
            memcpy(nextB, b, sizeof(nextB)); // 使用 memcpy 把接收到的board複製一份，都在新的上面做就可以了
            memcpy(nextL, l, sizeof(nextL));
            applyMove(nextB, nextL, m);

            int eval;
            if (step == 1) // 還有第二步要走，依然是我的回合
                eval = minimax(depth - 1, alpha, beta, currentColor, 2, true, nextB, nextL, myColor);
            else // 換對手走第一步
                eval = minimax(depth - 1, alpha, beta, (currentColor == "White" ? "Black" : "White"), 1, false, nextB, nextL, myColor);
            
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (auto& m : moves) {
            int nextB[9][9], nextL[9][9];
            memcpy(nextB, b, sizeof(nextB));
            memcpy(nextL, l, sizeof(nextL));
            applyMove(nextB, nextL, m);

            int eval;
            if (step == 1) // 對手的第二步
                eval = minimax(depth - 1, alpha, beta, currentColor, 2, false, nextB, nextL, myColor);
            else // 換我走第一步
                eval = minimax(depth - 1, alpha, beta, (currentColor == "White" ? "Black" : "White"), 1, true, nextB, nextL, myColor);

            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
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

    // 在 main 裡面替換掉原本隨機的部分
    Move bestM1 = {'P', 0, 0, 0, 0}, bestM2 = {'P', 0, 0, 0, 0};
    int maxScore = -INF;

    // 1. 取得所有合法的第一步 (Must Eat)
    vector<Move> eats1 = getMoves(color, true);

    // 如果是白棋第一回合，特殊處理
    if (Round == 1 && color == "White") {
        for (auto& m1 : eats1) {
            int nb[9][9], nl[9][9];
            memcpy(nb, board, sizeof(board));
            memcpy(nl, layer, sizeof(layer));
            applyMove(nb, nl, m1);
            
            // 深度建議設為 3 或 4 (取決於 5 秒限制)
            int score = minimax(3, -INF, INF, color == "White" ? "Black" : "White", 1, false, nb, nl, color);
            if (score > maxScore) {
                maxScore = score;
                bestM1 = m1;
                bestM2 = {'P', 0, 0, 0, 0};
            }
        }
    } else {
        // 一般回合：兩步組合
        for (auto& m1 : eats1) {
            int nb1[9][9], nl1[9][9];
            memcpy(nb1, board, sizeof(board));
            memcpy(nl1, layer, sizeof(layer));
            applyMove(nb1, nl1, m1);

            vector<Move> moves2 = getMoves(color, false);
            moves2.push_back({'P', 0, 0, 0, 0}); // 考慮 Pass

            for (auto& m2 : moves2) {
                int nb2[9][9], nl2[9][9];
                memcpy(nb2, nb1, sizeof(nb1));
                memcpy(nl2, nl1, sizeof(nl1));
                applyMove(nb2, nl2, m2);

                // 換對手了，所以 isMax 傳 false
                int score = minimax(2, -INF, INF, color == "White" ? "Black" : "White", 1, false, nb2, nl2, color);
                
                if (score > maxScore) {
                    maxScore = score;
                    bestM1 = m1;
                    bestM2 = m2;
                }
            }
        }
    }

    // 最後輸出 bestM1, bestM2
    writeOutput(transformMove(bestM1), transformMove(bestM2));
}