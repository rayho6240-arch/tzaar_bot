#include <bits/stdc++.h>
using namespace std;

struct Move {
    char action;
    int sx, sy, ex, ey;
};

// 用來記錄被覆蓋掉的格子狀態
struct UndoRecord {
    int targetPiece;
    int targetLayer;
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

bool isStraight(int sx,int sy,int ex,int ey){
    int dx = ex - sx;
    int dy = ey - sy;
    return (dx==0 || dy==0 || dx+dy==0);
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

            if (board[i][j] < myMin || board[i][j] > myMax) continue;  //省資源

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



//===========================AI核心部分=========================================================================================================



// ===== 套用走法 =====多一個紀錄record的功能======
UndoRecord applyUndoMove(int board[9][9], int layer[9][9], Move m) {
    UndoRecord record={0,0};
    if (m.action == 'P') return record;

    record.targetPiece = board[m.ex][m.ey];
    record.targetLayer = layer[m.ex][m.ey]; //紀錄被抹滅的敵方棋子

    if (m.action == 'S') {
        layer[m.ex][m.ey] += layer[m.sx][m.sy];
    } else { // Eat
        layer[m.ex][m.ey] = layer[m.sx][m.sy];
    }

    board[m.ex][m.ey] = board[m.sx][m.sy];
    board[m.sx][m.sy] = 0;
    layer[m.sx][m.sy] = 0;

    return record;
}

//回朔功能，取代copy
void undoMove(int b[9][9], int l[9][9], Move m, UndoRecord record) {
    if (m.action == 'P') return;

    // 1. 先把移動的棋子「拔」回起點
    b[m.sx][m.sy] = b[m.ex][m.ey];
    
    if (m.action == 'S') {
        // 如果剛才是疊子 (Stack)：起點原本的高度 = 現在塔的總高度 - 終點原本的高度
        l[m.sx][m.sy] = l[m.ex][m.ey] - record.targetLayer;
    } else if (m.action == 'E') {
        // 如果剛才是吃子 (Eat)：起點原本的高度就是現在終點的高度
        l[m.sx][m.sy] = l[m.ex][m.ey];
    }

    // 2. 恢復終點原本的苦主 (被吃掉的，或是被壓在下面的棋子)
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
    return 0; // 數量大於 3 的時候不扣分
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
                    // 狀況A：敵人比我高或一樣高 (我被威脅了！)
                    if (HigherOrEqual) {
                        threatPenalty += 40; 
                    }
                    // 狀況B：敵人比我矮 (我可以吃它！)
                    else {
                        attackBonus += 20; 
                    }
                }
                break; 
            }
        }
    }
}

// 評估 minimax 最後的場面分數，正分對我有利，負分對對手有利
int evaluate(int b[9][9], int l[9][9], string myColor) {

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
                if(type <= 3) {
                    whiteMaxCount = 1; 
                    blackMaxCount = 0; 
                }
                else {
                    blackMaxCount = 1; 
                    whiteMaxCount = 0; 
                }
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

            // 【修正點】：在迴圈內直接把這顆棋子的 threat 和 attack 算進總分裡
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

    // 2. 致命傷判定區 (滅種直接判勝負)
    if (myColor == "White") {
        if(w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) return -999999;
        if(b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) return 999999;
    } 
    else { 
        if(b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) return -999999;
        if(w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) return 999999;            
    }

    // 3. 稀缺性懲罰區
    whitePenalty += getPenalty(w_tzaar) + getPenalty(w_tzarra) + getPenalty(w_totts);
    blackPenalty += getPenalty(b_tzaar) + getPenalty(b_tzarra) + getPenalty(b_totts);

    // 4. 結算制空權分數
    int immortalBonus = 5000; 
    if(whiteMaxCount > 0 && blackMaxCount == 0) whiteScore += immortalBonus;
    else if(blackMaxCount > 0 && whiteMaxCount == 0) blackScore += immortalBonus;

    // 5. 總結算分數
    // 【修正點】：確實扣掉稀缺性懲罰
    whiteScore -= whitePenalty;
    blackScore -= blackPenalty;

    if (myColor == "White") return whiteScore - blackScore;
    else return blackScore - whiteScore;
}


//==============================================================
// MiniMax 模塊
//===============================================================
//總共有兩層for迴圈，第一層是第一步，第二層是第二步//包含alpha/beta剪枝

int minimax(int depth, int alpha, int beta, bool isMaximizing, string currentColor, int step, int b[9][9], int l[9][9], string myColor) {
    // 終點判斷
    if (depth == 0) return evaluate(b, l, myColor);

    bool mustEat = (step == 1);
    vector<Move> moves = getMoves(currentColor, mustEat);
    if (step == 2) moves.push_back({'P', 0, 0, 0, 0});

    if (moves.empty()) {
        return isMaximizing ? -999999 : 999999; // 沒棋走就輸了
    }

    if (isMaximizing) { //note 目前實驗階段，算力不足，都是我方m1,m2用for loop 出來，直接模擬敵方(false)情況
        int maxEval = -1e9;
        for (auto& m : moves) {
            // 1. 走棋，並「存下」這一步的歷史紀錄
            UndoRecord record = applyUndoMove(b, l, m);

            int eval;
            if (step == 1) // 大if是代表現在是我方，step1代表我方第一動，還有第二動，所以模擬完第一動後依然是我方回合。
                eval = minimax(depth, alpha, beta, true, currentColor, 2, b, l, myColor); //我的第二動
            else // 兩動做完，換對手
                eval = minimax(depth - 1, alpha, beta, false, (currentColor=="White"?"Black":"White"), 1, b, l, myColor); // 注意step 是1因為換對手了
            
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval); //ALPHA是一開始傳入的，後面會因為模擬被更新

            // 2. 復原盤面 (必須放在 break 之前！)
            undoMove(b, l, m, record); 

            // 3. 判斷是否剪枝
            if (beta <= alpha) break; 
        }
        return maxEval;
    } 
    else {
        int minEval = 1e9;
        for (auto& m : moves) {
            // 1. 走棋，並「存下」這一步的歷史紀錄
            UndoRecord record = applyUndoMove(b, l, m);

            int eval;
            if (step == 1) // 對手還有第二動
                eval = minimax(depth, alpha, beta, false, currentColor, 2, b, l, myColor);
            else // 對手做完，換回我
                eval = minimax(depth - 1, alpha, beta, true, (currentColor=="White"?"Black":"White"), 1, b, l, myColor);

            minEval = min(minEval, eval);
            beta = min(beta, eval);

            // 2. 復原盤面 (必須放在 break 之前！)
            undoMove(b, l, m, record); 

            // 3. 判斷是否剪枝
            if (beta <= alpha) break; 
        }
        return minEval;
    }
}

//==============================================================
// findBestMove模塊，最初最終選擇
//===============================================================
//其中的depth先設自己2,敵方1
void findBestMove(string myColor, int currentRound) {
    int maxScore = -1e9;
    Move bestM1 = {'P', 0, 0, 0, 0};
    Move bestM2 = {'P', 0, 0, 0, 0};

    // 1. 取得所有合法的第一步 (必須是吃子)
    vector<Move> firstMoves = getMoves(myColor, true);
    
    // 一個合法步驟都沒有
    if (firstMoves.empty()) {
        writeOutput(transformMove(bestM1), transformMove(bestM2));
        return;
    }

    for (auto& m1 : firstMoves) {
        // 模擬執行第一步，並存下 record1
        UndoRecord record1 = applyUndoMove(board, layer, m1);  //再次提醒，undorecord是data type // 套用了applyMove，board跟layer都被改了// 而record1裡面存的就是這一步被吃掉的棋子資料


        //===================特殊處理：白棋第一回合只有一動======================================
        if (currentRound == 1 && myColor == "White") {
            int score = minimax(1, -1e9, 1e9, false, "Black", 1, board, layer, myColor); //注意false , step 代表讓敵方下棋
            if (score > maxScore) {
                maxScore = score;
                bestM1 = m1;                 //m1生出來就有改到board了，所以就算沒有生出m2也不影響跑minimax
                bestM2 = {'P', 0, 0, 0, 0};  //這邊一樣要賦值，因為.out要看到
            }
            // 必須復原 record1，否則 continue 會讓棋子黏在盤面上
            undoMove(board, layer, m1, record1); 
            continue;
        }
        //====================================================================================



    // 2. 取得所有合法的第二步 (因為全域 board 已經更新了，所以會取到正確的第二步)
        vector<Move> secondMoves = getMoves(myColor, false);
        secondMoves.push_back({'P', 0, 0, 0, 0}); // 加入 Pass 選項

        for (auto& m2 : secondMoves) {
            // 模擬執行第二步，並存下 record2
            UndoRecord record2 = applyUndoMove(board, layer, m2);
//************************************************************************************************實際minimax是在這裡呼叫，自己的m1,m2都迴圈跑出來了 
            int score = minimax(1, -1e9, 1e9, false, (myColor=="White"?"Black":"White"), 1, board, layer, myColor); //false, step==1 已經是換對手了(模擬)
            if (score > maxScore) {
                maxScore = score;
                bestM1 = m1;
                bestM2 = m2;
            }
            // 測試完第二步，復原 record2
            undoMove(board, layer, m2, record2); 
        }
        // 測試完第一步所有的後續可能，復原 record1
        undoMove(board, layer, m1, record1); 
    }

    // 儲存歷史紀錄並輸出
    appendHistory(currentRound, myColor, 1, bestM1);
    appendHistory(currentRound, myColor, 2, bestM2);
    writeOutput(transformMove(bestM1), transformMove(bestM2));
}



 

int main(int argc, char* argv[]) {

    string color = argv[1];
    readBoard(argv[3]);
    readLayer(argv[4]);

    // 讀取目前是第幾回合
    int Round = readLastRound("stepHistory.txt");

    // 交給函數執行動作
    findBestMove(color, Round);

    return 0;

}