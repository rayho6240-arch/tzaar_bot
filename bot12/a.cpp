int evaluate(int b[9][9], int l[9][9], string myColor, int depth) {
    // ... (前面原本的變數宣告都不動)

    // 1. 【新增這兩個變數】用來計算雙方有幾顆棋子「具備吃子的能力」
    int whiteAttackCount = 0;
    int blackAttackCount = 0;

    // 1. 掃描整個盤面 
    for (int i = 0; i < 9; i++) {
        // ... (中間的計算數量跟基礎分數都不動)

            // --- 射線掃描：偵測威脅與攻擊面 ---
            int threat = 0;
            int attack = 0;
            scanRaycast(i, j, type, height, b, l, threat, attack); 

            if (type <= 3) {
                whiteScore -= threat;
                whiteScore += attack; 
                // 2. 【新增】：如果這顆白棋有攻擊目標，行動力 +1
                if (attack > 0) whiteAttackCount++; 
            } 
            else {
                blackScore -= threat;
                blackScore += attack;
                // 2. 【新增】：如果這顆黑棋有攻擊目標，行動力 +1
                if (attack > 0) blackAttackCount++; 
            }
        }
    }

    // ... (中間的致命傷判定與稀缺性懲罰都不動)

    // 3. 【新增】：結算行動力分數 (這就是救命關鍵！)
    // 每多一顆能吃對手的棋子，就額外加 50 分，引導 AI 保持陣型活躍
    whiteScore += (whiteAttackCount * 50);
    blackScore += (blackAttackCount * 50);

    // 5. 總結算分數
    whiteScore -= whitePenalty;
    blackScore -= blackPenalty;

    if (myColor == "White") return whiteScore - blackScore;
    else return blackScore - whiteScore;
}