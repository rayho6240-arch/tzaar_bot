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