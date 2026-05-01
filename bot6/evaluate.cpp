

int dx[6] = {1, -1, 0, 0, 1, -1};
int dy[6] = {0, 0, 1, -1, 1, -1};



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