






// ===== MiniMax 模塊 =====
// 新增了 Timer& tmr 和 double time_limit 兩個參數
int minimax(int depth, int alpha, int beta, bool isMaximizing, const string& currentColor, int step, int b[9][9], int l[9][9], const string& myColor, Timer& tmr, double time_limit) {
    
    // 【防超時機制】每次進入節點先看時間，超時就用 throw 瞬間脫出所有遞迴！
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


// ===== findBestMove 模塊 (根節點搜尋與計時) =====
void findBestMove(const string& myColor, int currentRound) {
    Timer tmr1;
    tmr1.start(); 
    double time_limit = 4.5; 

    // 用來儲存「上一個安全算完的深度」所找到的最好步
    Move finalBestM1 = {'P', 0, 0, 0, 0};
    Move finalBestM2 = {'P', 0, 0, 0, 0};

    vector<Move> firstMoves = getMoves(myColor, true);
    orderMoves(firstMoves, board, layer, myColor, 1); 

    if (firstMoves.empty()) {
        writeOutput(transformMove(finalBestM1), transformMove(finalBestM2));
        return;
    }
    
    // 先給定一組預設合法步，以防深度 1 都還沒算完就超時
    finalBestM1 = firstMoves[0]; 

    // ===== 進入 Iterative Deepening 迭代加深 =====
    try {
        // 設定最大深度上限 (例如 10)，通常時間到了就會被強制中斷
        for (int targetDepth = 1; targetDepth <= 10; targetDepth++) {
            
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
                        throw runtime_error("TIMEOUT"); 
                    }

                    UndoRecord record2 = applyUndoMove(board, layer, m2);
                    
                    // 注意這裡傳入 targetDepth 以及計時器
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
            
            // ===== 如果程式順利走到這裡，代表「這個 targetDepth」已經安全算完！ =====
            // 我們把結果更新到 finalBestM1/M2，這樣就算下一層算到一半超時，我們也有最保險的答案。
            finalBestM1 = currentDepthBestM1;
            finalBestM2 = currentDepthBestM2;
            
            // (選用) 偵錯用：可以印出目前跑到的深度
            // cerr << "Depth " << targetDepth << " completed safely. Score: " << maxScore << endl;
        }
    } 
    catch (const runtime_error& e) {
        // ===== 超時攔截區 =====
        // 當 throw runtime_error 觸發時，程式會「瞬間」跳來這裡。
        // 我們什麼都不用做，直接沿用上一次安全算完的 finalBestM1 和 finalBestM2 即可！
        // cerr << "Timeout reached! Fallback to last completed depth." << endl;
    }

    // 寫出歷史紀錄與輸出
    appendHistory(currentRound, myColor, 1, finalBestM1);
    appendHistory(currentRound, myColor, 2, finalBestM2);
    writeOutput(transformMove(finalBestM1), transformMove(finalBestM2));
}