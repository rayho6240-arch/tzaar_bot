// =================================================================
    // 2. 新增：盤點雙方剩餘的棋種數量
    int myTzaar = 0, myTzarra = 0, myTot = 0;
    int enemyTzaar = 0, enemyTzarra = 0, enemyTot = 0;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int piece = b[i][j];
            if (piece == 0) continue;

            if (myColor == "White") {
                if (piece == 1) myTzaar++;
                else if (piece == 2) myTzarra++;
                else if (piece == 3) myTot++;
                else if (piece == 4) enemyTzaar++;
                else if (piece == 5) enemyTzarra++;
                else if (piece == 6) enemyTot++;
            } else { // myColor == "Black"
                if (piece == 4) myTzaar++;
                else if (piece == 5) myTzarra++;
                else if (piece == 6) myTot++;
                else if (piece == 1) enemyTzaar++;
                else if (piece == 2) enemyTzarra++;
                else if (piece == 3) enemyTot++;
            }
        }
    }

    // 3. 生存本能 (Safety Stock)：極力避免自己的棋種瀕危
    // 如果只剩 2 個，給予警告扣分
    if (myTzaar == 2) score -= 500;
    if (myTzarra == 2) score -= 500;
    if (myTot == 2) score -= 500;

    // 如果只剩 1 個，紅色警戒！給予巨量扣分，AI 會為了避開這個分數拼命逃跑
    if (myTzaar == 1) score -= 5000;
    if (myTzarra == 1) score -= 5000;
    if (myTot == 1) score -= 5000;

    // 4. 殺手本能 (Killer Instinct)：極力把對手逼向絕種
    // 如果對手某種棋子只剩 2 個，我們加分 (鼓勵 AI 往這方向下)
    if (enemyTzaar == 2) score += 500;
    if (enemyTzarra == 2) score += 500;
    if (enemyTot == 2) score += 500;

    // 如果對手某種棋子只剩 1 個，給予巨量加分，AI 會像瘋狗一樣去追殺那個棋子！
    if (enemyTzaar == 1) score += 5000;
    if (enemyTzarra == 1) score += 5000;
    if (enemyTot == 1) score += 5000;
    // =================================================================