#include <algorithm>
#include <vector>



// 用來綁定著法與其猜測分數的結構
struct MoveScore {
    Move move;
    int guessScore;
};

// 著法排序函數 (注意：這裡的參數必須對應你專案裡的棋盤變數型態)
void orderMoves(vector<Move>& moves, int b[9][9], int l[9][9], string currentColor, int step) {
    vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const auto& m : moves) {
        int score = 0;

        // 1. 特殊情況：Pass (跳過)
        if (m.type == 'P') {
            // 提示：通常 Pass 不是個好主意，除非逼不得已。
            // 建議給一個很低的分數，讓它排在最後面測試。
            scoredMoves.push_back({m, -1000}); 
            continue;
        }

        // 取得起點與終點座標
        int fx = m.fromX, fy = m.fromY;
        int tx = m.toX, ty = m.toY;

        // =======================================================
        // TODO: 在這裡實作你的「猜測評分邏輯 (Heuristics)」
        // 絕對不要在這裡呼叫 evaluate()！只能用加減法和讀取陣列！
        // =======================================================
        
        if (step == 1) {
            // 【第一步：必定是吃子 (Capture)】
            // 提示 A：實作 MVV-LVA (最有價值受害者 - 最無價值攻擊者)
            // 你需要讀取 b[tx][ty] 判斷被吃的是什麼棋子。
            // Tzaar 最珍貴 -> 加極高分 (例如 +10000)
            // Tzarra 次之  -> 加高分 (例如 +5000)
            // Totts 最普通 -> 加普通分 (例如 +1000)
            
            // 提示 B：受害者的高度 (l[tx][ty])
            // 吃掉對方疊很高的塔，威脅就解除了 -> 高度越高，加分越多 (例如 l[tx][ty] * 100)
            
            // 提示 C：攻擊者的代價 (l[fx][fy])
            // 用矮塔吃高塔是賺的，用高塔吃矮塔有風險 -> 可以稍微扣除攻擊者高度的分數

        } else if (step == 2) {
            // 【第二步：可能是吃子，也可能是強化 (Stack/Reinforce)】
            // 提示 D：你需要先判斷這步是吃子還是疊自己的子
            // 如果終點 (tx, ty) 是對手的子 -> 套用上面 step 1 的吃子加分邏輯。
            // 如果終點 (tx, ty) 是自己的子 (強化) ->
            //   - 把自己的 Tzaar 疊高保護起來 -> 加分！
            //   - 疊合後高度變很高 (l[fx][fy] + l[tx][ty]) -> 加分！
        }

        scoredMoves.push_back({m, score});
    }

    // 2. 根據 guessScore 由大到小降冪排序
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
        return a.guessScore > b.guessScore;
    });

    // 3. 覆寫原本的 moves 陣列，這樣回傳後就是排好序的了
    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scoredMoves[i].move;
    }
}