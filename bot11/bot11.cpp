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

    // --- 【效能優化與防呆前置】：預先計算盤面上各種棋子的數量 ---
    int pieceCounts[7] = {0}; // 陣列大小開 7，用來存 type 1~6 的數量
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            int type = board[r][c];
            if (type >= 1 && type <= 6) {
                pieceCounts[type]++;
            }
        }
    }
    // -----------------------------------------------------------

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
                            int targetType = board[ni][nj];
                            
                            // 【終極防呆鎖】：判斷目標是不是自己人，且是不是全場最後一個
                            bool isMyPiece = (targetType >= myMin && targetType <= myMax);
                            bool isSuicide = (isMyPiece && pieceCounts[targetType] <= 1);
                            
                            // 只有在「不是自殺」的情況下，才把這個 Stack 步加入合法清單
                            if (!isSuicide) {
                                m = {'S', i, j, ni, nj};
                                if (isValid(m, color, false)) res.push_back(m);
                            }
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
    ifstream fin(path);   //(預設ios::in)
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
    ofstream fout("out.txt");  //預設就自帶 ios::out 屬性。 所以不需要特別寫。
    fout<<m1.action<<","<<m1.sx<<","<<m1.sy<<","<<m1.ex<<","<<m1.ey<<"\n";
    fout<<m2.action<<","<<m2.sx<<","<<m2.sy<<","<<m2.ex<<","<<m2.ey<<"\n";
}

void appendHistory(int round, const string& color, int step, Move m) {
    ofstream fout("stepHistory.txt", ios::app);  //[ios::app] 是 "append"（附加） 的縮寫。使用它的核心原因在於：保護舊資料，並將新資料寫在檔案的最末端。
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
    if (count == 1) return 6000; // 【修正】：大幅提高瀕危懲罰，一定要比制空權加分高！
    return 0; 
}

// 射線掃描：同時計算威脅扣分與攻擊加分
void scanRaycast(int startX, int startY, int myType, int myHeight, int b[9][9], int l[9][9], int &threatPenalty, int &attackBonus) {
    threatPenalty = 0;
    attackBonus = 0;
    bool isWhite = (myType <= 3);

    for (int dir = 0; dir < 6; dir++) {
        int curX = startX + dx[dir];
        int curY = startY + dy[dir];

        while (curX >= 0 && curX < 9 && curY >= 0 && curY < 9 && b[curX][curY] >= 0) {
            if (b[curX][curY] == 0) {
                curX += dx[dir]; 
                curY += dy[dir];
                continue;               
            } 
            else {
                int targetType = b[curX][curY];
                int targetHeight = l[curX][curY];
                bool isEnemy = (isWhite && targetType >= 4) || (!isWhite && targetType <= 3);
                bool HigherOrEqual = targetHeight >= myHeight;

                if (isEnemy) {
                    if (targetHeight > myHeight) {
                        threatPenalty += 50; // 絕對劣勢：他能吃我，我吃不動他（致命威脅）
                    } 
                    else if (targetHeight == myHeight) {
                        threatPenalty += 20; // 互相威脅
                        attackBonus += 20;   // 互相攻擊（局勢平衡）
                    } 
                    else {
                        attackBonus += 40;   // 絕對優勢：我能吃他，他吃不動我
                    }
                }
                break; 
            }
        }
    }
}

int evaluate(int b[9][9], int l[9][9], string myColor, int depth) {

    int whiteScore = 0;
    int blackScore = 0;
    int whitePenalty = 0;
    int blackPenalty = 0;

    int w_tzaar = 0, w_tzarra = 0, w_totts = 0;
    int b_tzaar = 0, b_tzarra = 0, b_totts = 0;

    int maxHeight = 0;      
    int whiteMaxCount = 0;  
    int blackMaxCount = 0;  

    // 1. 掃描整個盤面 
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (b[i][j] <= 0) continue; 

            int type = b[i][j];
            int height = l[i][j]; 

            // --- 制空權追蹤 ---
            if(height > maxHeight){
                maxHeight = height;
                if(type <= 3) { whiteMaxCount = 1; blackMaxCount = 0; }
                else { blackMaxCount = 1; whiteMaxCount = 0; }
            } 
            else if(height == maxHeight){
                if(type <= 3) whiteMaxCount++;
                else blackMaxCount++;
            }

            // --- 計算數量與基礎分數 ---
            if (type == 1) w_tzaar++;
            else if (type == 2) w_tzarra++;
            else if (type == 3) w_totts++;
            else if (type == 4) b_tzaar++;
            else if (type == 5) b_tzarra++;
            else if (type == 6) b_totts++;
            
            int val = 0;
            if (type == 1 || type == 4) val = 100 + height * 10;
            else if (type == 2 || type == 5) val = 50 + height * 10;
            else if (type == 3 || type == 6) val = 20 + height * 10;

            if (type <= 3) whiteScore += val;
            else blackScore += val;

            // --- 射線掃描：偵測威脅與攻擊面 ---
            int threat = 0;
            int attack = 0;
            scanRaycast(i, j, type, height, b, l, threat, attack); 

            if (type <= 3) {
                whiteScore -= threat;
                whiteScore += attack; 
            } 
            else {
                blackScore -= threat;
                blackScore += attack;
            }
        }
    }

    // 2. 致命傷判定區 (加入 depth，好死不如賴活著！)
    int winScore = 1000000 + depth;
    int loseScore = -1000000 - depth;

    if (myColor == "White") {
        if(w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) return loseScore;
        if(b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) return winScore;
    } 
    else { 
        if(b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) return loseScore;
        if(w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) return winScore;            
    }

    // 3. 稀缺性懲罰區
    whitePenalty += getPenalty(w_tzaar) + getPenalty(w_tzarra) + getPenalty(w_totts);
    blackPenalty += getPenalty(b_tzaar) + getPenalty(b_tzarra) + getPenalty(b_totts);

    // 4. 結算制空權分數
    int immortalBonus = 80; 
    if(whiteMaxCount > 0 && blackMaxCount == 0) whiteScore += immortalBonus;
    else if(blackMaxCount > 0 && whiteMaxCount == 0) blackScore += immortalBonus;

    // 5. 總結算分數
    whiteScore -= whitePenalty;
    blackScore -= blackPenalty;

    if (myColor == "White") return whiteScore - blackScore;
    else return blackScore - whiteScore;
}




//===========================================================
//  orderMoves 模塊 
//==============================================================
void orderMoves(vector<Move>& moves, int b[9][9], int l[9][9], const string& currentColor, int step) {
    if (moves.empty()) return;

    // 1. 預先計算盤面上所有棋子的數量 (O(N) 效能優化)
    int counts[7] = {0}; 
    for(int i = 0; i < 9; i++) {
        for(int j = 0; j < 9; j++) {
            if(b[i][j] > 0) counts[b[i][j]]++;
        }
    }

    vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());

    // 2. 走法評分
    for (const auto& m : moves) {
        if (m.action == 'P') { 
            // Pass (放棄第二步) 通常是最無奈的選擇，優先度放最低
            scoredMoves.push_back({m, -10000});
            continue;
        }

        int score = 0;
        int fx = m.sx, fy = m.sy;
        int tx = m.ex, ty = m.ey;

        int topType = b[fx][fy];    // 準備移動的棋子
        int bottomType = b[tx][ty]; // 目標位置的棋子

        // 判斷是「堆疊 (Stack)」還是「吃子 (Eat)」
        // (1,2,3 為白方，4,5,6 為黑方。若兩者同屬一方則為堆疊)
        bool isStack = ((topType <= 3 && bottomType <= 3) || (topType >= 4 && bottomType >= 4));

        if (!isStack) { 
            // 吃子：進攻邏輯
            int enemyRemains = counts[bottomType];

            if (enemyRemains == 1) {
                score += 20000; // 滅族之擊！最優先
            } 
            else if (enemyRemains == 2) {
                score += 8000;  // 瀕危打擊
            } 
            else {
                // 一般吃子：結合高度邏輯，吃越高，分越高
                score += 1000 + l[tx][ty] * 100; 
            }
        } 
        else { 
            // 堆疊：防守與加固
            if (counts[bottomType] <= 2) {
                score -= 20000; // 致命自殺步：把自己的稀缺棋子壓死，排到最後面！
            } 
            else {
                if (counts[topType] <= 2) {
                    score += 5000; // 保護稀缺棋種，優先移走它去疊高
                }
                // 一般堆疊：結合原本的高度邏輯
                score += 500 + l[tx][ty] * 50; 
            }
        }

        // 保稍微懲罰移動既有高塔的行為
        if (step == 1) {
             score -= l[fx][fy] * 10; 
        }

        // 寫入 vector，note::::( MoveScore 構造方式是 {Move, guessScore})
        scoredMoves.push_back({m, score});
    }

    //根據 guessScore 降序排列 (分數高的排前面)
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.guessScore > b.guessScore; 
    });

    // 5. 寫回原本的 moves 陣列:::大老闆手中的地圖
    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scoredMoves[i].move;
    }
}



//此函數使minimax可以有其他跳出條件，而非死了還繼續跑
//note我原本用int data type +1來 處理。 Gemini說bool較省算力。
int fastCheckGameOver(int b[9][9], const string& myColor) {
    bool wTzaar = false, wTzarra = false, wTotts = false;
    bool bTzaar = false, bTzarra = false, bTotts = false;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int piece = b[i][j];
            
            //  0 是空位或邊界，直接跳過以加速
            if (piece == 0) continue; 

            // 判斷白棋
            if (piece == 1) wTzaar = true;
            else if (piece == 2) wTzarra = true;
            else if (piece == 3) wTotts = true;
            // 判斷黑棋 
            else if (piece == 4) bTzaar = true;
            else if (piece == 5) bTzarra = true;
            else if (piece == 6) bTotts = true;

            // 只要雙方三種棋子都至少找到一顆，遊戲就確定還沒結束，直接中斷掃描！//來自gemini的邏輯優化
            if (wTzaar && wTzarra && wTotts && bTzaar && bTzarra && bTotts) {
                return 0; 
            }
        }
    }

    // 掃描完畢（或提前觸發滅族），結算存活狀態
    bool whiteAlive = wTzaar && wTzarra && wTotts;
    bool blackAlive = bTzaar && bTzarra && bTotts;

    // 回傳權重
    if (!whiteAlive && blackAlive) return (myColor == "White") ? -1 : 1;
    if (!blackAlive && whiteAlive) return (myColor == "White") ? 1 : -1;
    
    return 0; 
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

    if (depth == 0) return evaluate(b, l, myColor, depth);

    int checkStatus = fastCheckGameOver(b, myColor); // 輕量級的函數，只掃描三種棋子數量
    if (checkStatus == 1) return 1000000 + depth;    // 我方贏了
    if (checkStatus == -1) return -1000000 - depth;  // 我方輸了（被滅族）

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
                //第一回合特殊步驟
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





//============================main函數===================================================================================================

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




