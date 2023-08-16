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
Cell* bestCell;
bool boxing;
bool huggingWall;
bool goingUp, goingDown, goingLeft, goingRight;
bool lining;
bool hasTarget;
bool hasTargetX, hasTargetY;
int targetX, targetY;
bool test = false;
int minEnemyX = 2'000'000'000, maxEnemyX = -1, minEnemyY = 2'000'000'000, maxEnemyY = -1;

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
            mat[i][j].distMyHead = 2'000'000'000;
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

bool snakeTouchedBottomWall (int snakeId) {
    for (int i = 1; i <= n; i++) {
        if (mat[n][i].val == snakeId) return true;
    }
    return false;
}

bool snakeTouchedTopWall (int snakeId) {
    for (int i = 1; i <= n; i++) {
        if (mat[1][i].val == snakeId) return true;
    }
    return false;
}

bool snakeTouchedLeftWall (int snakeId) {
    for (int i = 1; i <= n; i++) {
        if (mat[i][1].val == snakeId) return true;
    }
    return false;
}

bool snakeTouchedRightWall (int snakeId) {
    for (int i = 1; i <= n; i++) {
        if (mat[i][n].val == snakeId) return true;
    }
    return false;
}

bool snakeTouchedWall (int snakeId) {
    if (snakeTouchedBottomWall(snakeId)) return true;
    if (snakeTouchedTopWall(snakeId)) return true;
    if (snakeTouchedLeftWall(snakeId)) return true;
    if (snakeTouchedRightWall(snakeId)) return true;
    return false;
}

void updateWallCellCandidate(Cell* newCell, int &dist, Cell* &bestCell) {
    int newDist = newCell->distMyHead;
    if (newDist < dist) {
        dist = newDist;
        bestCell = newCell;
    }
}

int minHorizontalWallDistance (int x, int y) {
    return min(x - 1, n - x);
}

int minVerticalWallDistance (int x, int y) {
    return min(y - 1, n - y);
}

int minWallDistance (int x, int y) {
    return min(minVerticalWallDistance(x, y), minHorizontalWallDistance(x, y));
}

void considerBoxing() {
    if (snakeTouchedWall(mySnake)) return;
    setupLee();
    calculateDistancesFromMyHead(0, true);
    Cell* bCHorizontal;
    Cell* bCVertical;

    int minDist = 2'000'000'000;
    for (int i = 1; i <= n; i++) {
        updateWallCellCandidate(&mat[i][1], minDist, bCVertical);
        updateWallCellCandidate(&mat[i][n], minDist, bCVertical);
        updateWallCellCandidate(&mat[1][i], minDist, bCHorizontal);
        updateWallCellCandidate(&mat[n][i], minDist, bCHorizontal);
    }

    // TBD
    if (minWallDistance(bCHorizontal->x, bCHorizontal->y) < minWallDistance(bCVertical->x, bCVertical->y)) {
        bestCell = bCHorizontal;
    }
    else {
        bestCell = bCVertical;
    }

    boxing = true;
    for (int i = 0; i < 4; i++) {
        int x, y;

        if (i / 2) {
            x = minEnemyX;
        }
        else {
            x = maxEnemyX;
        }

        if (i % 2) {
            y = minEnemyY;
        }
        else {
            y = maxEnemyY;
        }

        if (min(minHorizontalWallDistance(headX, headY), minVerticalWallDistance(headX, headY)) * 2 <= min(minHorizontalWallDistance(x, y), minVerticalWallDistance(x, y))) {
            boxing = false;
        }
    }
}

bool isHuggingHorizontalWall() {
    if (headX == 1 || headX == n) return true;
    return false;
}

bool isHuggingVerticalWall() {
    if (headY == 1 || headY == n) return true;
    return false;
}

void calculateEnemyRectangleCorners() {
    minEnemyX = minEnemyY = 2'000'000'000;
    maxEnemyX = maxEnemyY = -1;
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= n; j++) {
            if (mat[i][j].val == enemySnake) {
                minEnemyX = min(minEnemyX, i);
                maxEnemyX = max(maxEnemyX, i);
                minEnemyY = min(minEnemyY, j);
                maxEnemyY = max(maxEnemyY, j);
            }
        }
    }
}

void considerHuggingWall() {
    if (isHuggingHorizontalWall()) {
        int flippedX = n + 1 - headX;
        int minProx = 2'000'000'000;
        int distDif;

        for (int i = 1; i <= n; i++) {
            if (mat[flippedX][i].distMyHead > 1'000'000) continue;
            distDif = mat[flippedX][i].distMyHead - mat[flippedX][i].minDistEnemyHead;
            if (distDif > 0) continue;
            if (distDif <= minProx) {
                minProx = distDif;
                targetY = i;
            }
        }

        if (minProx < 0) hasTarget = true;

        if (targetY > headY) goingRight = true;
        if (targetY < headY) goingLeft = true;

        if (goingRight || goingLeft) {
            hasTargetY = true;
        }
        else {
            if (headX == 1) {
                goingDown = true;
            }
            else {
                goingUp = true;
            }
            hasTargetX = true;
        }

        if (targetY <= n / 2 && maxEnemyY <= n / 2) {
            huggingWall = true;
        }
        if (targetY > n / 2 && minEnemyY > n / 2) {
            huggingWall = true;
        }
    }

    if (isHuggingVerticalWall()) {
        int flippedY = n + 1 - headY;
        int minProx = 2'000'000'000;
        int distDif;

        for (int i = 1; i <= n; i++) {
            if (test) cout << i << "\n";
            if (mat[i][flippedY].distMyHead > 1'000'000) continue;
            distDif = mat[i][flippedY].distMyHead - mat[i][flippedY].minDistEnemyHead;
            if (distDif > 0) continue;
            if (distDif <= minProx) {
                minProx = distDif;
                targetX = i;
            }
        }

        if (minProx < 0) hasTarget = true;

        if (targetX > headX) goingDown = true;
        if (targetX < headX) goingUp = true;

        if (goingUp || goingDown) {
            hasTargetX = true;
        }
        else {
            if (headY == 1) {
                goingRight = true;
            }
            else {
                goingLeft = true;
            }
            hasTargetY = true;
        }

        if (targetX <= n / 2 && maxEnemyX <= n / 2) {
            huggingWall = true;
        }
        if (targetX > n / 2 && minEnemyY > n / 2) {
            huggingWall = true;
        }
    }
}

char findDirection() {
    const char dir[] = "VSEN";

    if (boxing) {
        int res;
        bool found;
        if (test) cout << bestCell->x << " " << bestCell->y << "\n";
        for (int i = 0; i < 4; i++) {
            if ((bestCell->x == (headX + dx[i])) && (bestCell->y == (headY + dy[i]))) {
                res = i;
                found = true;
            }
        }
        if (found) {
            return dir[res];
        }
    }

    if (huggingWall) {
        int res;

        if (goingLeft) {
            if (mat[headX][headY - 1].val == 0) {
                res = 0;
            }
        }
        if (goingDown) {
            if (mat[headX + 1][headY].val == 0) {
                res = 1;
            }
        }
        if (goingRight) {
            if (mat[headX][headY + 1].val == 0) {
                res = 2;
            }
        }
        if (goingUp) {
            if (mat[headX - 1][headY].val == 0) {
                res = 3;
            }
        }

        return dir[res];
    }

    if (snakeTouchedWall(mySnake) && hasTarget) {
        if (hasTargetX) {
            bestCell = &mat[targetX][headY];
        }
        if (hasTargetY) {
            bestCell = &mat[headX][targetY];
        }

        while (bestCell->myHeadLeeFather) {
            bestCell = bestCell->myHeadLeeFather;
        }

        int res;
        bool found;
        for (int i = 0; i < 4; i++) {
            if ((bestCell->x == (headX + dx[i])) && (bestCell->y == (headY + dy[i]))) {
                res = i;
                found = true;
            }
        }
        if (found) {
            return dir[res];
        }
    }

    // each cardinal direction has a cost (w[i]) equal to
    // the sum of the manhatten distances to all head candidates
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
    calculateEnemyRectangleCorners();
    considerBoxing();
    considerHuggingWall();
    fout << findDirection() << "\n";

    return 0;
}
