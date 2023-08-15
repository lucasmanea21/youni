#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cstring>

using namespace std;

ifstream fin("snake.in");
ofstream fout("snake.out");

struct Cell {
    int val;
    int totalDist;
    bool isWhite;
    bool doneLee;
    int x, y;
};

vector< vector<Cell> > mat;
queue<Cell*> headPositions;
int headX, headY;
int n;
const int dx[4] = { 0,  1,  0, -1};
const int dy[4] = {-1,  0,  1,  0};
int mySnake;
int enemySnake;
bool test = false;

void readInput() {
    string s;
    fin >> s;
    n = s.size();

    mat.clear();
    mat.resize(n + 2);
    for (int i = 0; i <= n + 1; i++) {
        mat[i].resize(n + 2);
    }
    // matrix frame
    for (int i = 0; i <= n + 1; i++) {
        mat[i][0].val = mat[i][n + 1].val = mat[0][i].val = mat[n + 1][i].val = 3;
        mat[i][0].doneLee = mat[i][n + 1].doneLee = mat[0][i].doneLee = mat[n + 1][i].doneLee = true;
    }

    string s;
    for (int i = 1; i <= n; i++) {
        if (i != 1) {
            fin >> s;
        }
        for (int j = 1; j <= n; j++) {
            mat[i][j].val = s[j - 1] - '0';
            mat[i][j].isWhite = (i + j) % 2;
            mat[i][j].x = i;
            mat[i][j].y = j;
        }
    }

    fin >> headX >> headY;
    mySnake = mat[headX][headY].val;
    enemySnake = 3 - mySnake;
}

int countNeighbors (int x, int y) {
    int res = 0;
    for (int i = 0; i < 4; i++) {
        if (mat[x + dx[i]][y + dy[i]].val == enemySnake) {
            res++;
        }
    }
    return res;
}

void findEnemyHeads() {
    // neighbor count frequency
    int nei[5] = {0};
    int enemyTileCount = 0;
    // chessboard optimization for odd steps
    int whiteCount = 0;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (test) cout << mat[i][j].val << " ";
            if (mat[i][j].val == enemySnake) {
                enemyTileCount++;
                nei[countNeighbors(i, j)]++;
                if (mat[i][j].isWhite) {
                    whiteCount++;
                }
            }
        }
        if (test) cout << "\n";
    }

    if (test) {
        for (int i = 0; i < 5; i++) {
            cout << nei[i] << " ";
        }
        cout << "\n";
    }

    bool identifiedStartColor;
    bool whiteStart;
    if (2 * whiteCount != enemyTileCount) {
        identifiedStartColor = true;
        whiteStart = 2 * whiteCount > enemyTileCount;
    }

    int neiOneOrLess = 0;
    for (int i = 0; i <= 1; i++) {
        neiOneOrLess += nei[i];
    }

    while (!headPositions.empty()) {
        headPositions.pop();
    }

    // tiles that have only one neighbor must be either the head or the tip of the tail
    if (neiOneOrLess >= 2) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (mat[i][j].val == enemySnake && countNeighbors(i, j) < 2) {
                    headPositions.push(&mat[i][j]);
                }
            }
        }
    }
    else {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                if (mat[i][j].val == 2 && ((mat[i][j].isWhite == whiteStart) || (!identifiedStartColor))) {
                    headPositions.push(&mat[i][j]);
                }
            }
        }
    }
}

void setupLee () {
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            mat[i][j].doneLee = false;
        }
    }
}

void lee (int startX, int startY) {
    queue< pair<int, Cell*> > leeQ;

    for (int i = 0; i < 4; i++) {
        leeQ.push({1, &mat[startX + dx[i]][startY + dy[i]]});
    }

    int cDist;
    Cell* cCell;
    while (!leeQ.empty()) {
        cDist = leeQ.front().first;
        cCell = leeQ.front().second;
        leeQ.pop();

        if (cCell->doneLee) continue;
        cCell->doneLee = true;
        if (cCell->val != 0) continue;

        cCell->totalDist += cDist;

        for (int i = 0; i < 4; i++) {
            leeQ.push({cDist + 1, &mat[cCell->x + dx[i]][cCell->y + dy[i]]});
        }
    }

    if (test) {
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++) {
                cout << mat[i][j].totalDist << " ";
            }
            cout << "\n";
        }
    }
}

int freeSpace (int x, int y) {
    int res = 0;
    for (int i = 0; i < 4; i++) {
        if (mat[x + dx[i]][y + dy[i]].val == 0) {
            res++;
        }
    }
    return res;
}

char findDirection() {
    // each cardinal direction has a cost (w[i]) equal to
    // the sum of the manhatten distances to all head candidates
    const char dir[] = "VSEN";
    int w[4] = {0};
    for (int i = 0; i < 4; i++) {
        if (mat[headX + dx[i]][headY + dy[i]].val == 0) {
            w[i] = mat[headX + dx[i]][headY + dy[i]].totalDist;
        }
        else {
            w[i] = 1'000'000'000;
        }
        if (freeSpace(headX + dx[i], headY + dy[i]) == 0) {
            w[i] += 100'000'000;
        }
    }

    if (test) {
        for (int i = 0; i < 4; i++) {
            cout << w[i] << " " << dir[i] << "\n";
        }
    }

    int minTotalDist = 2'000'000'000;
    int res;
    for (int i = 0; i < 4; i++) {
        if (w[i] < minTotalDist) {
            minTotalDist = w[i];
            res = i;
        }
    }
    return dir[res];
}

int main()
{
    readInput();
    findEnemyHeads();
    Cell* cHead;
    while (!headPositions.empty()) {
        cHead = headPositions.front();
        headPositions.pop();
        setupLee();
        lee(cHead->x, cHead->y);
    }
    fout << findDirection() << "\n";

    return 0;
}
