#include <bits/stdc++.h>
using namespace std;

struct Move {
    
    char action;
    int sx, sy, ex, ey;
};

int board[9][9];
int layer[9][9];

int dx[6] = {1, -1, 0, 0, -1, 1};
int dy[6] = {0, 0, 1, -1, -1, 1};

// ===== 讀檔 =====
void readBoard(string path) {
    ifstream fin(path);
    string line;

    for(int i=0;i<9;i++){
        getline(fin, line);
        stringstream ss(line);
        for(int j=0;j<9;j++){
            string val;
            getline(ss, val, ',');
            board[i][j] = stoi(val);
        }
    }
}

void readLayer(string path) {
    ifstream fin(path);
    string line;

    for(int i=0;i<9;i++){
        getline(fin, line);
        stringstream ss(line);
        for(int j=0;j<9;j++){
            string val;
            getline(ss, val, ',');
            layer[i][j] = stoi(val);
        }
    }
}

int readLastRound(const string& path) {
    ifstream fin(path);
    string line, lastLine;

    if (!fin.is_open()) return 1;

    while (getline(fin, line)) {
        if (!line.empty()) {
            lastLine = line;  // 不斷覆蓋 → 最後會留下最後一行
        }
    }

    fin.close();

    // 如果整個檔案是空的
    if (lastLine.empty()) return 1;

    stringstream ss(lastLine);
    int Round;

    ss >> Round;

    return (Round+1);
}



// ===== 判斷直線 =====
bool isStraight(int sx,int sy,int ex,int ey){
    int dx = ex - sx;
    int dy = ey - sy;
    return (dx==0 || dy==0 || dx-dy==0);
}

// ===== 判斷長距離衝刺是否會撞到 =====
bool clearPath(int sx,int sy,int ex,int ey){
    int dx = ex - sx;
    int dy = ey - sy;

    int steps = max(abs(dx), abs(dy)); //一定有一個動
    if(steps==0) return false;

    int stepx = dx / steps;
    int stepy = dy / steps;

    int x = sx + stepx;
    int y = sy + stepy;

    while(x != ex || y != ey){
        if(board[x][y] != 0) return false;
        x += stepx;
        y += stepy;
    }
    return true;
}

// ===== 合法判斷 =====
bool validMove(Move m, string color, bool mustEat){
    //正方形上下左右邊界
    if(m.sx<0||m.sx>=9||m.sy<0||m.sy>=9) return false;
    if(m.ex<0||m.ex>=9||m.ey<0||m.ey>=9) return false;
    if(m.ex-m.ey>4||2*m.ex-m.ey>-4||m.sx-m.sy>4||2*m.sx-m.sy>-4) return false; //斜線邊界

    //沒進洞，沒選到空
    if(board[m.sx][m.sy] <= 0) return false;
    if(board[m.ex][m.ey] == -1) return false;

    // 沒選錯顏色
    if(color=="White" && board[m.sx][m.sy] > 3) return false;
    if(color=="Black" && board[m.sx][m.sy] < 4) return false;

    //沒走直線
    if(!isStraight(m.sx,m.sy,m.ex,m.ey)) return false;
    if(!clearPath(m.sx,m.sy,m.ex,m.ey)) return false;

    // Eat
    if(mustEat){
        if (m.action != 'E') return false;
        if(board[m.ex][m.ey]==0) return false;
        if(color=="White" && board[m.ex][m.ey] <=3) return false;
        if(color=="Black" && board[m.ex][m.ey] >=4) return false;
        if(layer[m.sx][m.sy] < layer[m.ex][m.ey]) return false;
        else return true;
    }

    else{
        //Eat
        if(m.action=='E'){
            if(board[m.ex][m.ey]==0) return false;
            if(color=="White" && board[m.ex][m.ey] <=3) return false;
            if(color=="Black" && board[m.ex][m.ey] >=4) return false;
            if(layer[m.sx][m.sy] < layer[m.ex][m.ey]) return false;
            else return true;
        }
         // Stack
        if(m.action=='S'){
            if(board[m.ex][m.ey]==0) return false;
            if(color=="White" && board[m.ex][m.ey] >3) return false;
            if(color=="Black" && board[m.ex][m.ey] <4) return false;
            return true;
       }
       
       return true;
    }

}



// ===== 找所有合法 move =====
vector<Move> getMoves(string color, bool mustEat){
    vector<Move> res;
    for(int i=0;i<9;i++){
        for(int j=0;j<9;j++){

            if(board[i][j] <=0) continue;

            if(color=="White" && board[i][j]>3) continue;
            if(color=="Black" && board[i][j]<4) continue;

            for(int d=0;d<6;d++){
                for(int step=1;step<9;step++){
                    int ni = i + dx[d]*step;
                    int nj = j + dy[d]*step;

                    Move m; //建立區域物件
                    m.sx=i; m.sy=j; m.ex=ni; m.ey=nj;

                    if(mustEat){
                        m.action='E';
                        if(validMove(m,color,true))
                            res.push_back(m);
                    }else{
                        m.action='E';
                        if(validMove(m,color,false))
                            res.push_back(m);

                        m.action='S';
                        if(validMove(m,color,false))
                            res.push_back(m);
                    }

                    //*********************************************缺pass case 
                }
            }
        }
    }

    return res;
}


//=====座標變換=====
vector<int> transform(vector<vector<int>>& A, vector<int>& B, vector<int> p){
    vector<int> res(2, 0);
    for(int i=0;i<2;i++){
        for(int j=0;j<2;j++){
            res[i] += A[i][j] * p[j];
        }
        res[i] += B[i];  // 平移
    }
    return res;
}

Move transformMove(Move m, vector<vector<int>>& A, vector<int>& B){
    vector<int> s = {m.sx, m.sy};
    vector<int> e = {m.ex, m.ey};

    vector<int> ns = transform(A, B, s);
    vector<int> ne = transform(A, B, e);

    m.sx = ns[0];
    m.sy = ns[1];
    m.ex = ne[0];
    m.ey = ne[1];

    return m;
}
//=====================分隔線=====================



// ===== 輸出 =====
void writeOutput(Move m1, Move m2){
    ofstream fout("out.txt");
    fout << m1.action << "," << m1.sx << "," << m1.sy << "," << m1.ex << "," << m1.ey << "\n";
    fout << m2.action << "," << m2.sx << "," << m2.sy << "," << m2.ex << "," << m2.ey << "\n";
}

void appendHistory(int round, string color, int step, Move m) {
    ofstream fout("stepHistory.txt", ios::app); // 重點：append模式

    fout << round << ","
         << (color=="White" ? "W" : "B") << ","
         << step << ","
         << m.action << ","
         << m.sx << "," << m.sy << ","
         << m.ex << "," << m.ey << "\n";

    fout.close();
}



// ===== main =====
int main(int argc, char* argv[]) {

   // 平台參數解析
    string color = argv[1];
    string boardPath = argv[3];
    string layerPath = argv[4];
    
    //座標轉換矩陣
    vector<vector<int>> A = {
            {1,  0},
            {0, -1}
    };
    vector<int> B={-4,4};

    srand(time(0));
    int Round=0;
    
    readBoard(boardPath);
    readLayer(layerPath);
    Round=readLastRound("stepHistory.txt");

    if ( Round==1 && color=="White"){
        // Step1: 一定 Eat
        auto moves1 = getMoves(color, true);
        Move m1 = moves1[rand()%(moves1.size())];

        // Step2: 隨機
        Move m2;
            m2 = {'P',0,0,0,0};
        writeOutput(m1, m2);
        appendHistory(Round, color, 1, m1);
        appendHistory(Round, color, 2, m2);
    }


    else{
        // Step1: 一定 Eat
        auto moves1 = getMoves(color, true);
        Move m1 = moves1[rand()%moves1.size()];

        // Step2: 隨機
        auto moves2 = getMoves(color, false);

        Move m2;
        if(moves2.empty()){
            m2 = {'P',0,0,0,0};
        }else{
            m2 = moves2[rand()%moves2.size()];
        }

        m1 = transformMove(m1, A, B);
        m2 = transformMove(m2, A, B);

        writeOutput(m1, m2);
        appendHistory(Round, color, 1, m1);
        appendHistory(Round, color, 2, m2);
    }

}

