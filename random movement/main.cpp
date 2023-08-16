#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <ctime>

using namespace std;

ifstream fin("snake.in");
ofstream fout("snake.out");

struct Cell {
    int val;
    int x, y;
};

vector< vector<Cell> > mat;
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
            mat[i][j].x = i;
            mat[i][j].y = j;
        }
    }

    fin >> headX >> headY;
    mySnake = mat[headX][headY].val;
    enemySnake = 3 - mySnake;
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

char findDirection() {
    // each direction has a randomized weight, but free spaces take priority
    const char dir[] = "VSEN";
    int w[4] = {0};

    int* p = new int;
    srand(time(nullptr) + (long long)p);
    for (int i = 0; i < 4; i++) {
        w[i] = rand() % 50'000'000;
    }

    for (int i = 0; i < 4; i++) {
        if (mat[headX + dx[i]][headY + dy[i]].val != 0) {
            w[i] += 1'000'000'000;
        }
        if (freeSpace(headX + dx[i], headY + dy[i]) == 0) {
            w[i] += 800'000'000;
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
    fout << findDirection() << "\n";

    return 0;
}
