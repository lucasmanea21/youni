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
    int minDistEnemyHead = 2'000'000'000;
    int distMyHead;
    int x, y;
    Cell* myHeadLeeFather;
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
    }

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
    for (int i = 0; i <= n + 1; i++) {
        mat[i][0].doneLee = mat[i][n + 1].doneLee = mat[0][i].doneLee = mat[n + 1][i].doneLee = true;
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
        cCell->minDistEnemyHead = min(cDist, cCell->minDistEnemyHead);

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

void calculateDistancesFromMyHead (int startDir, bool allDir) {
    queue< pair< pair<int, Cell*>, Cell* > > q;

    if (allDir) {
        for (int i = 0; i < 4; i++) {
            q.push({{1, NULL}, &mat[headX + dx[i]][headY + dy[i]]});
        }
    }
    else {
        for (int i = startDir; i < startDir; i++) {
            q.push({{1, NULL}, &mat[headX + dx[i]][headY + dy[i]]});
        }
    }

    int cDist;
    Cell* cCell;
    Cell* father;
    while (!q.empty()) {
        cDist = q.front().first.first;
        father = q.front().first.second;
        cCell = q.front().second;
        q.pop();

        if (cCell->doneLee) continue;
        cCell->doneLee = true;
        if (cCell->val != 0) continue;
        cCell->myHeadLeeFather = father;

        cCell->distMyHead = cDist;

        for (int i = 0; i < 4; i++) {
            q.push({{cDist + 1, cCell}, &mat[cCell->x + dx[i]][cCell->y + dy[i]]});
        }
    }
}

int freeSpace (int x, int y) {
    if (mat[x][y].val != 0) return 0;
    int res = 0;
    for (int i = 0; i < 4; i++) {
        if (mat[x + dx[i]][y + dy[i]].val == 0) {
            res++;
        }
    }
    return res;
}

int minDistToWall (int x, int y) {
    int a, b;
    a = min(x - 1, y - 1);
    b = min(n - x, n - y);
    return min(a, b);
}

bool snakeTouchedWall (int snakeId) {
    for (int i = 1; i <= n; i++) {
        if (mat[i][1].val == snakeId) return true;
        if (mat[i][n].val == snakeId) return true;
        if (mat[1][i].val == snakeId) return true;
        if (mat[n][i].val == snakeId) return true;
    }
    return false;
}

void considerBoxing() {
    if (snakeTouchedWall(mySnake)) return;

    int dL, dR, dT, dB, dirL, dirR, dirT, dirB;
    dL = dR = dT = dB = 2'000'000'000;
    Cell *bL, *bR, *bB, *bT;
    for (int i = 0; i < 4; i++) {
        setupLee();
        calculateDistancesFromMyHead(i, false);
        for (int j = 1; j <= n; j++) {
        }
    }

    // left/right wall
    bool goToLeftWall, goToRightWall, goToTopWall, goToBottomWall;
    goToLeftWall = goToRightWall = goToTopWall = goToBottomWall = false;

    int minEnemyX, maxEnemyX;
    minEnemyX = 2'000'000'000;
    maxEnemyX = -1;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (mat[i][j].val == enemySnake) {
                minEnemyX = min(minEnemyX, i);
                maxEnemyX = max(maxEnemyX, i);
            }
        }
    }
}

char findDirection() {
    // each cardinal direction has a cost (w[i]) equal to
    // the sum of the manhatten distances to all head candidates
    const char dir[] = "VSEN";
    long long int w[4] = {0};
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

    long long int minTotalDist = 2'000'000'000'000'000'000;
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
