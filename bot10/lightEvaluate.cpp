



// 注意：參數新增了 int depth
int evaluate(int b[9][9], int l[9][9], const string& myColor, int depth) {
    long long whiteScore = 0; // 使用 long long 防止極端情況溢位
    long long blackScore = 0;

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

            // --- 1. 制空權判定 ---
            if(height > maxHeight){
                maxHeight = height;
                if(type <= 3) { whiteMaxCount = 1; blackMaxCount = 0; }
                else { blackMaxCount = 1; whiteMaxCount = 0; }
            } 
            else if(height == maxHeight){
                if(type <= 3) whiteMaxCount++;
                else blackMaxCount++;
            }

            // --- 2. 計算數量 ---
            if (type == 1) w_tzaar++;
            else if (type == 2) w_tzarra++;
            else if (type == 3) w_totts++;
            else if (type == 4) b_tzaar++;
            else if (type == 5) b_tzarra++;
            else if (type == 6) b_totts++;
            
            // --- 3. 基礎價值評估 ---
            // 調升基礎權重，讓 AI 沒事不要亂疊 (因為疊了會減少一個 piece)
            int val = 0;
            if (type == 1 || type == 4) val = 300 + height * 20; // Tzaar 最貴
            else if (type == 2 || type == 5) val = 150 + height * 20;
            else if (type == 3 || type == 6) val = 100 + height * 20;

            if (type <= 3) whiteScore += val;
            else blackScore += val;
        }
    }

    // --- 4. 終極生死判定 (加入 depth 讓 AI「好死不如賴活著」) ---
    // depth 越大代表離現在越近，depth 越小代表在未來。
    // 所以 -1000000 - depth 會讓越早發生的死亡分數越低 (越負)
    int winScore = 1000000 + depth;
    int loseScore = -1000000 - depth;

    if (w_tzaar == 0 || w_tzarra == 0 || w_totts == 0) {
        return (myColor == "White") ? loseScore : winScore;
    }
    if (b_tzaar == 0 || b_tzarra == 0 || b_totts == 0) {
        return (myColor == "White") ? winScore : loseScore;
    }

    // --- 5. 稀缺性懲罰 (物種多樣性保護) ---
    whiteScore -= (getPenalty(w_tzaar) + getPenalty(w_tzarra) + getPenalty(w_totts));
    blackScore -= (getPenalty(b_tzaar) + getPenalty(b_tzarra) + getPenalty(b_totts));

    // --- 6. 制空權獎勵 ---
    // 只有在保證物種安全的情況下，制空權才有意義
    int immortalBonus = 3000; 
    if(whiteMaxCount > 0 && blackMaxCount == 0) whiteScore += immortalBonus;
    else if(blackMaxCount > 0 && whiteMaxCount == 0) blackScore += immortalBonus;

    // --- 7. 最終輸出 ---
    return (myColor == "White") ? (int)(whiteScore - blackScore) : (int)(blackScore - whiteScore);
}