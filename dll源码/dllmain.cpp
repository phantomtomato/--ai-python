// Copyright (c) 2025 phantomtomato
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>

using namespace std;

/*==========  基本常量和数据结构  ==========*/
const int BOARD_SIZE = 15;
const int DEPTH = 3;          // 搜索深度
const double RATIO = 1.0;        // 进攻系数
const int INF = 1e9;

struct Point {
    int x, y;
    Point(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
};

/*==========  全局棋盘  ==========*/
int board[BOARD_SIZE][BOARD_SIZE];            // 0 空  1 AI  2 人
vector<Point> aiList, humanList, allList;   // 与 Python 同名

/*==========  棋型评分表（与 Python 完全一致）  ==========*/
const vector<pair<int, vector<int>>> shapeScore = {
    {50,    {0,1,1,0,0}},
    {50,    {0,0,1,1,0}},
    {200,   {1,1,0,1,0}},
    {500,   {0,0,1,1,1}},
    {500,   {1,1,1,0,0}},
    {5000,  {0,1,1,1,0}},
    {5000,  {0,1,0,1,1,0}},
    {5000,  {0,1,1,0,1,0}},
    {5000,  {1,1,1,0,1}},
    {5000,  {1,1,0,1,1}},
    {5000,  {1,0,1,1,1}},
    {5000,  {1,1,1,1,0}},
    {5000,  {0,1,1,1,1}},
    {50000, {0,1,1,1,1,0}},
    {INF,   {1,1,1,1,1}}
};

/*==========  工具：判断位置合法  ==========*/
inline bool inRange(int x, int y) { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }

/*==========  工具：判断五连  ==========*/
bool gameWin(const vector<Point>& lst) {
    for (const Point& p : lst) {
        int dx[4] = { 1,0,1,1 }, dy[4] = { 0,1,1,-1 };
        for (int k = 0; k < 4; ++k) {
            int cnt = 1;
            for (int d = -1; d <= 1; d += 2) {
                for (int step = 1; step < 5; ++step) {
                    int nx = p.x + d * step * dx[k], ny = p.y + d * step * dy[k];
                    if (!inRange(nx, ny)) break;
                    bool hit = false;
                    for (const Point& q : lst) if (q.x == nx && q.y == ny) { hit = true; break; }
                    if (!hit) break;
                    cnt++;
                }
            }
            if (cnt >= 5) return true;
        }
    }
    return false;
}

/*==========  工具：生成空点（带邻居裁剪）  ==========*/
vector<Point> blanks() {
    vector<Point> b;
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            if (board[i][j] == 0) b.emplace_back(i, j);

    /* 只留下有邻居的 */
    vector<Point> res;
    for (const Point& p : b) {
        bool ok = false;
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                int nx = p.x + dx, ny = p.y + dy;
                if (inRange(nx, ny) && board[nx][ny] != 0) { ok = true; break; }
            }
        if (ok) res.push_back(p);
    }
    return res;
}

/*==========  评估函数  ==========*/
int calShape(int x, int y, int dx, int dy, int who) {
    int enemy = 3 - who;
    int maxScore = 0;
    for (int offset = -5; offset <= 0; ++offset) {
        int pos[6] = { 0 };
        for (int i = 0; i < 6; ++i) {
            int nx = x + (offset + i) * dx, ny = y + (offset + i) * dy;
            if (!inRange(nx, ny)) { pos[i] = -1; break; }
            int v = board[nx][ny];
            pos[i] = v == 0 ? 0 : (v == who ? 1 : 2);
        }
        for (const auto& pr : shapeScore) {
            const vector<int>& s = pr.second;
            bool match = true;
            for (size_t i = 0; i < s.size(); ++i)
                if (pos[i] != s[i]) { match = false; break; }
            if (match) maxScore = max(maxScore, pr.first);
        }
    }
    return maxScore;
}

int evaluate(bool isAI) {
    int who = isAI ? 1 : 2;
    int myScore = 0, oppScore = 0;
    for (const Point& p : (isAI ? aiList : humanList))
        for (int dx = 0; dx < 2; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy <= 0) continue;
                myScore += calShape(p.x, p.y, dx, dy, who);
            }
    for (const Point& p : (isAI ? humanList : aiList))
        for (int dx = 0; dx < 2; ++dx)
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy <= 0) continue;
                oppScore += calShape(p.x, p.y, dx, dy, 3 - who);
            }
    return myScore - int(oppScore * RATIO * 0.1);
}

/*==========  α-β 搜索  ==========*/
Point bestPoint;
int negamax(int depth, int alpha, int beta, bool isAI) {
    if (depth == 0 || gameWin(aiList) || gameWin(humanList))
        return evaluate(isAI);

    vector<Point> cand = blanks();
    // 简单排序：距离最近落子越近越优先
    if (!allList.empty()) {
        Point last = allList.back();
        sort(cand.begin(), cand.end(), [last](const Point& a, const Point& b) {
            return abs(a.x - last.x) + abs(a.y - last.y) <
                abs(b.x - last.x) + abs(b.y - last.y);
            });
    }

    for (const Point& p : cand) {
        board[p.x][p.y] = isAI ? 1 : 2;
        (isAI ? aiList : humanList).push_back(p);
        allList.push_back(p);

        int val = -negamax(depth - 1, -beta, -alpha, !isAI);

        board[p.x][p.y] = 0;
        (isAI ? aiList : humanList).pop_back();
        allList.pop_back();

        if (val > alpha) {
            alpha = val;
            if (depth == DEPTH) bestPoint = p;
            if (alpha >= beta) return beta;
        }
    }
    return alpha;
}

/*==========  DLL 导出接口  ==========*/
extern "C" {
    typedef void (*Callback)(const char*);

    __declspec(dllexport)
        void get_ai_move(int b[BOARD_SIZE][BOARD_SIZE], int* row, int* col, Callback cb) {
        // 重建内部棋盘
        memset(board, 0, sizeof(board));
        aiList.clear(); humanList.clear(); allList.clear();
        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j) {
                board[i][j] = b[i][j];
                if (b[i][j] == 1) { aiList.emplace_back(i, j); allList.emplace_back(i, j); }
                if (b[i][j] == 2) { humanList.emplace_back(i, j); allList.emplace_back(i, j); }
            }
        negamax(DEPTH, -INF, INF, true);
        *row = bestPoint.x;
        *col = bestPoint.y;
        if (cb) {
            char buf[128];
            snprintf(buf, sizeof(buf), "AI choose (%d,%d)\n", *row, *col);
            cb(buf);
        }
    }
}
