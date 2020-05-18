#pragma once
#include "mask.h"

#include <ostream>
#include <cassert>
#include <array>

const int N = 8;
const int INF = 0x7f7f7f7f;

const int DIR = 8;
const int dx[DIR] = {0, -1,-1,-1, 0, 1, 1, 1};
const int dy[DIR] = {-1,-1, 0, 1, 1, 1, 0,-1};

inline int getdir(const int curdx, const int curdy) {
    for (int dir = 0; dir < DIR; ++dir) {
        if (dx[dir] == curdx && dy[dir] == curdy) {
            return dir;
        }
    }
    throw std::runtime_error("dir not found");
}

inline int index(const int x, const int y) {
    return x * N + y;
}

using MaskArray = std::array<std::array<int, N>, N>;

const MaskArray isborder{{
    {0,1,0,0, 0,0,1,0},
    {1,1,0,0, 0,0,1,1},
    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},

    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},
    {1,1,0,0, 0,0,1,1},
    {0,1,0,0, 0,0,1,0},
}};

const MaskArray iscorner{{
    {1,0,0,0, 0,0,0,1},
    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},

    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},
    {0,0,0,0, 0,0,0,0},
    {1,0,0,0, 0,0,0,1},
}};

MaskArray toArray(const Mask& mask) {
    MaskArray array;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            array[i][j] = mask.getbit(index(i, j));
        }
    }
    return array;
}

struct Point {
    int x, y;

    Point() : x{-1}, y{-1} {}
    Point(int x_, int y_) : x{x_}, y{y_} {}

    int index() const {
        return ::index(x, y);
    }

    bool operator == (const Point& ot) const {
        return x == ot.x && y == ot.y;
    }

    bool operator != (const Point& ot) const {
        return !(*this == ot);
    }

    Point operator ^ (const int dir) const {
        Point p = *this;
        p ^= dir;
        return p;
    }

    Point& operator ^= (const int dir) {
        assert(dir >= 0 && dir < DIR);
        x += dx[dir];
        y += dy[dir];
        return *this;
    }

    friend std::ostream& operator <<(std::ostream& out, const Point& obj) {
        return out << obj.x << ' ' << obj.y;
    }

    bool operator < (const Point& p) const {
        if (x != p.x) return x < p.x;
        return y < p.y;
    }
};

int DC[N][N];
Point D[N][N][8];

bool inside(const Point& p) {
    return p.x >= 0 && p.y >= 0 && p.x < N && p.y < N;
}

void checkinside(const Point& p) {
    if (!inside(p))
        throw std::runtime_error("point must be inside the world");
}

const int MMASK = (1 << N) - 1;

std::array<Point, N*N> points;
Mask around[N*N];
Mask cornerMask;
Mask borderMask;
Mask lineMask1[N][N][DIR];
Mask lineMask2[N][N][N*N];
Mask aroundMask[N][1 << N];

void initPoints() {
    for (int i=0; i<int(points.size()); ++i) {
        points[i].x = i / N;
        points[i].y = i % N;
    }
}

void initAround() {
    for (int i = 0; i < int(points.size()); ++i) {
        const Point& cur = points[i];
        for (int dir=0; dir<DIR; ++dir) {
            const Point newp = cur ^ dir;
            if (inside(newp)) {
                around[i].setbit(newp.index());
            }
        }
    }
}

void initCornerMask() {
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            if (iscorner[i][u])
                cornerMask.setbit(index(i, u));
            if (isborder[i][u])
                borderMask.setbit(index(i, u));
        }
    }
}

void initD() {
    for (const Point& cur : points) {
        for (int dir=0; dir<DIR; ++dir) {
            const Point newp = cur ^ dir;

            if (inside(newp)) {
                D[cur.x][cur.y][DC[cur.x][cur.y]] = Point{dx[dir], dy[dir]};
                ++DC[cur.x][cur.y];
            }
        }
    }
}

void initLineMasks() {
    for (const Point& cur : points) {
        for (int dir=0; dir<DIR; ++dir) {
            Point newp = cur;

            for (;;) {
                newp ^= dir;
                
                if (!inside(newp)) break;

                const int ind = newp.index();
                lineMask2[cur.x][cur.y][ind] = lineMask1[cur.x][cur.y][dir];
                lineMask1[cur.x][cur.y][dir].togglebit(ind, 0);
            }
        }
    }
}

void initAroundMask() {
    for (int row=0; row<N; ++row) {
        for (int mask = 0; mask < 1 << N; ++mask) {
            for (int bit=0; bit<N; ++bit) {
                if (mask & (1 << bit)) {
                    aroundMask[row][mask] |= around[index(row, bit)];
                    aroundMask[row][mask].setbit(index(row, bit));
                }
            }
        }
    }
}

void init() {
    initPoints();
    initAround();
    initCornerMask();
    initD();
    initLineMasks();
    initAroundMask();
}
