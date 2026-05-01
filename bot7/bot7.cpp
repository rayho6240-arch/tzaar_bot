#include <bits/stdc++.h>
#include <ctime> // 用來計算時間的函式庫
using namespace std;


class Timer {
    public:
        Timer() {
            start_ts = 0;
        }
        void start() {
            start_ts = clock(); // 改用 clock()
        }
        void setStart(clock_t ts) {
            start_ts = ts;
        }
        clock_t getStart() {
            return start_ts;
        }
        double getElapsedTime() {
            // 將經過的時脈週期除以 CLOCKS_PER_SEC 轉換為「秒」
            return (double)(clock() - start_ts) / CLOCKS_PER_SEC; 
        }
    private:
        clock_t start_ts; // 型別改為 clock_t
};

struct Move {
    char action;
    int sx, sy, ex, ey;
};

// 用來記錄被覆蓋掉的格子狀態
struct UndoRecord {
    int targetPiece;
    int targetLayer;
};

// 輔助結構，用來綁定著法與其猜測分數
struct MoveScore {
    Move move;
    int guessScore;
};

int board[9][9];
int layer[9][9];

int dx[6] = {1, -1, 0, 0, 1, -1};
int dy[6] = {0, 0, 1, -1, 1, -1};

// ===== 基本工具 =======================================================================================================================
bool isOnBoard(int r, int c) {
    if (r < 0 || r >= 9 || c < 0 || c >= 9) return false;
    if (board[r][c] == -1) return false;
    return true;
}

// ===== 合法性判斷 =====
bool isValid(Move m, const string& color, bool mustEat) {
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
vector<Move> getMoves(const string& color, bool mustEat) {
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
void readBoard(const string& path) {
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

void readLayer(const string& path) {
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

void appendHistory(int round, const string& color, int step, Move m) {
    ofstream fout("stepHistory.txt", ios::app);
    fout<<round<<","
        <<(color=="White"?"W":"B")<<","
        <<step<<","
        <<m.action<<","
        <<m.sx<<","<<m.sy<<","
        <<m.ex<<","<<m.ey<<"\n";
}





//===========================AI核心部分=========================================================================================================


// ===== 套用走法 =====多一個紀錄record的功能======
UndoRecord applyUndoMove(int b[9][9], int l[9][9], Move m) {
    UndoRecord record={0,0};
    if (m.action == 'P') return record;

    record.targetPiece = b[m.ex][m.ey];
    record.targetLayer = l[m.ex][m.ey];

    if (m.action == 'S') {
        l[m.ex][m.ey] += l[m.sx][m.sy];
    } else { 
        l[m.ex][m.ey] = l[m.sx][m.sy];
    }

    b[m.ex][m.ey] = b[m.sx][m.sy];
    b[m.sx][m.sy] = 0;
    l[m.sx][m.sy] = 0;

    return record;
}

//回朔功能，取代copy
void undoMove(int b[9][9], int l[9][9], Move m, UndoRecord record) {
    if (m.action == 'P') return;

    b[m.sx][m.sy] = b[m.ex][m.ey];
    
    if (m.action == 'S') {
        l[m.sx][m.sy] = l[m.ex][m.ey] - record.targetLayer;
    } else if (m.action == 'E') {
        l[m.sx][m.sy] = l[m.ex][m.ey];
    }

    b[m.ex][m.ey] = record.targetPiece;
    l[m.ex][m.ey] = record.targetLayer;
}

//===========================================================
//  evaluate模塊
//===========================================================
int getPenalty(int count) {
    if (count == 3) return 50;
    if (count == 2) return 200;
    if (count == 1) return 2000;
    return 0; 
}

int evaluate(int b[9][9], int l[9][9], const string& myColor) {
    int whiteScore = 0;
    int blackScore = 0;

    int w_tzaar = 0, w_tzarra = 0, w_totts = 0;
    int b_tzaar = 0, b_tzarra = 0, b_totts = 0;

    int maxHeight = 0;      
    int whiteMaxCount = 0;  
    int blackMaxCount = 0;  

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int type = b[i][j];
            if (type <= 0) continue; 

            int height = l[i][j]; 

            // 制空權
            if(height > maxHeight){
                maxHeight = height;
                if(type <= 3) { whiteMaxCount = 1; blackMaxCount = 0; }
                else { blackMaxCount = 1; whiteMaxCount = 0; }
            } 
            else if(height == maxHeight){
                if(type <= 3) whiteMaxCount++;
                else blackMaxCount++;
            }

            // 計算數量
            if (type == 1) w_tzaar++;
            else if (type == 2) w_tzarra++;
            else if (type == 3) w_totts++;
            else if (type == 4) b_tzaar++;
            else if (type == 5) b_tzarra++;
            else if (type == 6) b_totts++;
            
            // 基礎分數
            int val = 0;
            if (type == 1 || type == 4) val = 100 + height * 10;
            else if (type == 2 || type == 5) val = 50 + height * 10;
            else if (type == 3 || type == 6) val = 20 + height * 10;

            if (type <= 3) whiteScore += val;
            else blackScore += val;
        }
    }

    // 致命傷判定區
    if (w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) return (myColor == "White") ? -999999 : 999999;
    if (b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) return (myColor == "White") ? 999999 : -999999;

    // 稀缺性懲罰區
    whiteScore -= (getPenalty(w_tzaar) + getPenalty(w_tzarra) + getPenalty(w_totts));
    blackScore -= (getPenalty(b_tzaar) + getPenalty(b_tzarra) + getPenalty(b_totts));

    // 結算制空權分數
    int immortalBonus = 5000; 
    if(whiteMaxCount > 0 && blackMaxCount == 0) whiteScore += immortalBonus;
    else if(blackMaxCount > 0 && whiteMaxCount == 0) blackScore += immortalBonus;

    return (myColor == "White") ? (whiteScore - blackScore) : (blackScore - whiteScore);
}


//===========================================================
//  orderMoves 模塊 
//==============================================================
void orderMoves(vector<Move>& moves, int b[9][9], int l[9][9], const string& currentColor, int step) {
    if (moves.empty()) return;

    vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const auto& m : moves) {
        int score = 0;
        if (m.action == 'P') {
            scoredMoves.push_back({m, -10000});
            continue;
        }

        int fx = m.sx, fy = m.sy;
        int tx = m.ex, ty = m.ey;

        if (step == 1) {
            score += l[tx][ty] * 100;
            score -= l[fx][fy] * 10; 
        } else if (step == 2) {
            score += l[tx][ty] * 50; 
        }

        scoredMoves.push_back({m, score});
    }

    // ===== Bubble Sort + 提早結束 =====
    int n = scoredMoves.size();
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false; // 紀錄這一輪是否有交換發生

        for (int j = 0; j < n - 1 - i; ++j) {
            if (scoredMoves[j].guessScore < scoredMoves[j+1].guessScore) {
                MoveScore temp = scoredMoves[j];
                scoredMoves[j] = scoredMoves[j+1];
                scoredMoves[j+1] = temp;
                swapped = true; // 有發生交換就標記為 true
            }
        }
        
        // 如果這一輪從頭到尾都沒有交換，代表已經排序完成，提早收工！
        if (!swapped) {
            break;
        }
    }
    // =====================================

    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scoredMoves[i].move;
    }
}



//==============================================================
// MiniMax 模塊(智庫，去平行宇宙模擬未來走法，評估盤面，回朔)
//===============================================================
//總共有兩層for迴圈，第一層是第一步，第二層是第二步//包含alpha/beta剪枝
int minimax(int depth, int alpha, int beta, bool isMaximizing, const string& currentColor, int step, int b[9][9], int l[9][9], const string& myColor) {
    if (depth == 0) return evaluate(b, l, myColor);

    bool mustEat = (step == 1);
    vector<Move> moves = getMoves(currentColor, mustEat);
    if (step == 2) moves.push_back({'P', 0, 0, 0, 0});

    orderMoves(moves, b, l, currentColor, step); 

    if (moves.empty()) {
        return isMaximizing ? -999999 : 999999; 
    }

    if (isMaximizing) { 
        int maxEval = -1e9;
        for (auto& m : moves) {
            UndoRecord record = applyUndoMove(b, l, m);

            int eval;
            if (step == 1) 
                eval = minimax(depth, alpha, beta, true, currentColor, 2, b, l, myColor); 
            else 
                eval = minimax(depth - 1, alpha, beta, false, (currentColor=="White"?"Black":"White"), 1, b, l, myColor); 
            
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval); 

            undoMove(b, l, m, record); 
            if (beta <= alpha) break; 
        }
        return maxEval;
    } 
    else {
        int minEval = 1e9;
        for (auto& m : moves) {
            UndoRecord record = applyUndoMove(b, l, m);

            int eval;
            if (step == 1) 
                eval = minimax(depth, alpha, beta, false, currentColor, 2, b, l, myColor);
            else 
                eval = minimax(depth - 1, alpha, beta, true, (currentColor=="White"?"Black":"White"), 1, b, l, myColor);

            minEval = min(minEval, eval);
            beta = min(beta, eval);

            undoMove(b, l, m, record); 
            if (beta <= alpha) break; 
        }
        return minEval;
    }
}


//==============================================================
// findBestMove模塊，(大老闆，手上有一個地圖，叫minimax去平行宇宙算分看看)
//===============================================================
//其中的depth先設自己2,敵方2/包含alpha/betae更新寶底值更新
void findBestMove(const string& myColor, int currentRound) {
    
    // 啟動計時器 (設定 4.5 秒安全極限)
    Timer tmr1;
    tmr1.start(); //訂立開始時間戳記
    double time_limit = 4.5; 

    int alpha = -1e9;
    int beta = 1e9;
    
    int maxScore = -1e9;
    Move bestM1 = {'P', 0, 0, 0, 0};
    Move bestM2 = {'P', 0, 0, 0, 0};
    // 1. 取得所有合法的第一步 (必須是吃子)
    vector<Move> firstMoves = getMoves(myColor, true);
    orderMoves(firstMoves, board, layer, myColor, 1); 

    // 一個合法步驟都沒有
    if (firstMoves.empty()) {
        writeOutput(transformMove(bestM1), transformMove(bestM2));
        return;
    }
    bestM1 = firstMoves[0]; // 保底，避免超時跳出時第一步回傳不合法的 Pass//後面如果有回傳自然會覆蓋掉





    for (auto& m1 : firstMoves) {
        // 模擬執行第一步，並存下 record1
        UndoRecord record1 = applyUndoMove(board, layer, m1);  

        if (currentRound == 1 && myColor == "White") {

            if (tmr1.getElapsedTime() > time_limit) {
                undoMove(board, layer, m1, record1);
                goto TIME_OUT_BREAK; 
            }

            int score = minimax(2, alpha, beta, false, "Black", 1, board, layer, myColor); 
            if (score > maxScore) {
                maxScore = score;
                bestM1 = m1;                
                bestM2 = {'P', 0, 0, 0, 0};  
            }
            
            alpha = max(alpha, maxScore); // 動態更新底線
            undoMove(board, layer, m1, record1); 
            continue;
        }

        vector<Move> secondMoves = getMoves(myColor, false);
        secondMoves.push_back({'P', 0, 0, 0, 0}); 
        orderMoves(secondMoves, board, layer, myColor, 2); 

        for (auto& m2 : secondMoves) {
            // 防超時機制：檢查是否超過 4.5 秒
            double elapsed = tmr1.getElapsedTime();

            if (elapsed > time_limit) {
                undoMove(board, layer, m1, record1); // 提早跳出前復原盤面
                goto TIME_OUT_BREAK; 
            }

            UndoRecord record2 = applyUndoMove(board, layer, m2);
            
            int score = minimax(2, alpha, beta, false, (myColor=="White"?"Black":"White"), 1, board, layer, myColor); 
            
            if (score > maxScore) {
                maxScore = score;
                bestM1 = m1;
                bestM2 = m2;
            }

            alpha = max(alpha, maxScore); // 動態更新底線

            undoMove(board, layer, m2, record2); 
        }
        undoMove(board, layer, m1, record1); 
    }

TIME_OUT_BREAK:
    appendHistory(currentRound, myColor, 1, bestM1);
    appendHistory(currentRound, myColor, 2, bestM2);
    writeOutput(transformMove(bestM1), transformMove(bestM2));
}





int main(int argc, char* argv[]) {
    // 避免參數數量不對導致崩潰的基本防護
    if (argc < 5) {
        cerr << "Usage: ./a.out <Color> <Step> <BoardFile> <LayerFile>" << endl;
        return 1;
    }

    string color = argv[1];
    readBoard(argv[3]);
    readLayer(argv[4]);
    
    int Round = readLastRound("stepHistory.txt");
    findBestMove(color, Round);
    
    return 0;
}




