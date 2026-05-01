#include <bits/stdc++.h>
#include <ctime> // 用來計算時間的函式庫
#include <algorithm>
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

    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.guessScore > b.guessScore; // 分數高的排前面 //從原本 Bubble Sort 的 O(N^2) 降到了 O(N \log N)
    });


    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scoredMoves[i].move;
    }
}




//===========================================================
//  MiniMax 模塊
//==============================================================
// 新增了 Timer& tmr 和 double time_limit 兩個參數**************
int minimax(int depth, int alpha, int beta, bool isMaximizing, const string& currentColor, int step, int b[9][9], int l[9][9], const string& myColor, Timer& tmr, double time_limit) {
    
    // 【防超時機制】每次進入節點先看時間，超時就用 throw 瞬間脫出所有遞迴*****************
    if (tmr.getElapsedTime() > time_limit) {
        throw runtime_error("TIMEOUT"); 
    }

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
                eval = minimax(depth, alpha, beta, true, currentColor, 2, b, l, myColor, tmr, time_limit); 
            else 
                eval = minimax(depth - 1, alpha, beta, false, (currentColor=="White"?"Black":"White"), 1, b, l, myColor, tmr, time_limit); 
            
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
                eval = minimax(depth, alpha, beta, false, currentColor, 2, b, l, myColor, tmr, time_limit);
            else 
                eval = minimax(depth - 1, alpha, beta, true, (currentColor=="White"?"Black":"White"), 1, b, l, myColor, tmr, time_limit);

            minEval = min(minEval, eval);
            beta = min(beta, eval);

            undoMove(b, l, m, record); 
            if (beta <= alpha) break; 
        }
        return minEval;
    }
}


//===========================================================
//  findBestMove 模塊
//==============================================================
void findBestMove(const string& myColor, int currentRound) {

    //計時器建立時間戳
    Timer tmr1;
    tmr1.start(); 
    double time_limit = 4.0; //deadline

    // 用來儲存「上一個安全算完的深度」所找到的最好步
    Move finalBestM1 = {'P', 0, 0, 0, 0};
    Move finalBestM2 = {'P', 0, 0, 0, 0};

    vector<Move> firstMoves = getMoves(myColor, true);
    orderMoves(firstMoves, board, layer, myColor, 1); 

    if (firstMoves.empty()) {
        writeOutput(transformMove(finalBestM1), transformMove(finalBestM2));
        return;
    }
    
    // 先給定一組預設合法步，以防深度 1 時都還沒算完就超時
    finalBestM1 = firstMoves[0]; 




    // ===== 進入 Iterative Deepening 迭代加深 ====================================
    //**學到了"try-->安全保護"。 "throw, catch區 的用法-->跨函數的GOTO"。
    //=============================================================================
    try {
        // 設定最大深度上限 (例如 10)，通常時間到了就會被強制中斷
        for (int targetDepth = 1; targetDepth <= 10; targetDepth++) {
            

            // ===== 歷史繼承 (Root Move Ordering) =======
            // 如果這不是第一層 (已經有上一層的 best move 了)
            if (targetDepth > 1) {
                for (size_t i = 0; i < firstMoves.size(); i++) {
                    // 找到上一層決定的最好步
                    if (firstMoves[i].action == finalBestM1.action && 
                        firstMoves[i].sx == finalBestM1.sx && firstMoves[i].sy == finalBestM1.sy &&
                        firstMoves[i].ex == finalBestM1.ex && firstMoves[i].ey == finalBestM1.ey) {
                        
                        // 把這個最好步跟第一個元素交換，讓它成為下一次 Alpha-Beta 第一個探索的分支
                        swap(firstMoves[0], firstMoves[i]); //(algorithm裡的swap，直接交換兩個元素，不需要額外空間)
                        break;
                    }
                }
            }
            // ==========================================


            int alpha = -1e9;
            int beta = 1e9;
            int maxScore = -1e9;
            
            // 記錄「當前深度」找到的最好步
            Move currentDepthBestM1 = firstMoves[0];
            Move currentDepthBestM2 = {'P', 0, 0, 0, 0};

            for (auto& m1 : firstMoves) {
                // 最外層迴圈的安全檢查 (雙重保險)
                if (tmr1.getElapsedTime() > time_limit) throw runtime_error("TIMEOUT");

                UndoRecord record1 = applyUndoMove(board, layer, m1);  

                if (currentRound == 1 && myColor == "White") {
                    // 注意這裡傳入 targetDepth 以及計時器
                    int score = minimax(targetDepth, alpha, beta, false, "Black", 1, board, layer, myColor, tmr1, time_limit); 
                    if (score > maxScore) {
                        maxScore = score;
                        currentDepthBestM1 = m1;                
                        currentDepthBestM2 = {'P', 0, 0, 0, 0};  
                    }
                    alpha = max(alpha, maxScore);
                    undoMove(board, layer, m1, record1); 
                    continue;
                }

                vector<Move> secondMoves = getMoves(myColor, false);
                secondMoves.push_back({'P', 0, 0, 0, 0}); 
                orderMoves(secondMoves, board, layer, myColor, 2); 

                for (auto& m2 : secondMoves) {
                    if (tmr1.getElapsedTime() > time_limit) {
                        undoMove(board, layer, m1, record1); // 拋出例外前確保這層被復原
                        throw runtime_error("TIMEOUT");   //特殊招式，將超時視為系統error，直接丟出例外，跳出所有遞迴回到 findBestMove 的 catch 區塊
                    }

                    UndoRecord record2 = applyUndoMove(board, layer, m2);
                    
                    // 注意這裡傳入 targetDepth 以及計時器(minimax裡面每進入一層都會檢查時間，因為層數可能很大了)
                    int score = minimax(targetDepth, alpha, beta, false, (myColor=="White"?"Black":"White"), 1, board, layer, myColor, tmr1, time_limit); 
                    
                    if (score > maxScore) {
                        maxScore = score;
                        currentDepthBestM1 = m1;
                        currentDepthBestM2 = m2;
                    }
                    alpha = max(alpha, maxScore);

                    undoMove(board, layer, m2, record2); 
                }
                undoMove(board, layer, m1, record1); 
            }
            
            // ===== 每次for記錄一次 =====
            //下一次如果超時就還有數據可以用
            finalBestM1 = currentDepthBestM1;
            finalBestM2 = currentDepthBestM2;
            
        }
    } 
    catch (const runtime_error& ) {
        // 當 throw runtime_error 觸發時，程式會「瞬間」跳來這裡。
    }

    // 寫出歷史紀錄與輸出
    appendHistory(currentRound, myColor, 1, finalBestM1);
    appendHistory(currentRound, myColor, 2, finalBestM2);
    writeOutput(transformMove(finalBestM1), transformMove(finalBestM2));
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




