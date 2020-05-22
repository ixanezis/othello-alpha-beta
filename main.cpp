#include "mask.h"
#include "common.h"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <cassert>

using namespace std;

class TimeLimit : public std::exception {};

Point newp;

struct World {
    Mask empty;
    Mask discs[2];
    Mask frontier[2];
    int free;

    pair<uint64_t, uint64_t> hashpair() const {
        return {discs[0].mask, discs[1].mask};
    }

    bool makeMove(const Point& p, const int myindex) {
        const int enemyindex = 3 - myindex;

        Mask totalMask;
        for (int dir=0; dir<DIR; ++dir) {
            const Mask myInDir = discs[myindex - 1] & lineMask1[p.x][p.y][dir];

            if (myInDir.mask != 0) {
                int closest = myInDir.closest(dir);

                if ((empty & lineMask2[p.x][p.y][closest]).mask != 0) // this direction intersects an empty region
                    continue;

                totalMask |= lineMask2[p.x][p.y][closest];
            }
        }

        if (totalMask.mask == 0) return false; // cannot make a move

        --free;

        const int ind = index(p.x, p.y);

        discs[enemyindex - 1] ^= totalMask;
        discs[myindex - 1] ^= totalMask;
        discs[myindex - 1].togglebit(ind, 0);

        empty.togglebit(ind, 1);
        updateFrontier();

        return true;
    }

    void updateFrontier() {
        frontier[0] = frontier[1] = Mask{};

        for (int i=0; i<N; ++i) {
            frontier[0] |= aroundMask[i][(discs[0].mask >> (N * i)) & MMASK];
            frontier[1] |= aroundMask[i][(discs[1].mask >> (N * i)) & MMASK];
        }
    }

    friend istream& operator >>(istream& in, World& obj) {
        obj.free = N*N;
        obj.frontier[0] = obj.frontier[1] = Mask{};
        obj.discs[0] = obj.discs[1] = Mask{};

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                int cur;
                in >> cur;
                if (!in) {
                    return in;
                }
                if (cur == 3)
                    cur = 0;

                if (cur != 0) {
                    --obj.free;
                    obj.discs[cur-1].setbit(index(i, u));
                }
            }
        }

        obj.empty = ~(obj.discs[0] | obj.discs[1]);
        obj.updateFrontier();

        return in;
    }

    friend ostream& operator <<(ostream& out, const World& obj) {
        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                int value = 0;
                const int ind = index(i, u);
                if (obj.discs[0].getbit(ind) && obj.discs[1].getbit(ind))
                    throw runtime_error("both discs in same place");

                if (obj.discs[0].getbit(ind))
                    value = 1;
                if (obj.discs[1].getbit(ind))
                    value = 2;
                out << value << ' ';
            }
            out << '\n';
        }
        return out;
    }
};

int calcScore(const World& world, const int myindex, int deep, bool enemygo, bool nobodycanmove) {
    int score = 0;

    const int enemyindex = 3 - myindex;

    int mydiscs = world.discs[myindex - 1].bitcount();
    int enemydiscs = world.discs[enemyindex - 1].bitcount();

    if (nobodycanmove) {
        if (mydiscs > enemydiscs) return INF + mydiscs;
        if (mydiscs < enemydiscs) return -INF - enemydiscs;
        return 0;
    }

    Mask emptyFrontier = world.frontier[myindex - 1] & world.empty;
    score -= emptyFrontier.bitcount();

    // score for capturing the corner:
    Mask corner = cornerMask & world.discs[myindex - 1];
    score += corner.bitcount() * 100;

    Mask contactEnemy = world.frontier[myindex - 1] & world.discs[enemyindex - 1];
    score += contactEnemy.bitcount();

    if (world.free >= 30)
        score -= mydiscs / 4;

    if (enemygo)
        score -= calcScore(world, 3 - myindex, deep, false, false);

    return score;
}

const std::chrono::milliseconds TIME_LIMIT{940};
bool check = false;
double elapsed() {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

int g_sum = 0;

std::chrono::system_clock::time_point start;
int firstdeep;

pair<Point, int> findBestMove(const World& world, const int myindex, const int deep, int alpha, int beta) {
    Point bestMove;
    int bestScore = -INF - 100;

    if (check && std::chrono::system_clock::now() - start > TIME_LIMIT) throw TimeLimit{};

    const int enemyindex = 3 - myindex;

    vector<pair<int, Point>> moves;

    Mask mask = world.frontier[enemyindex - 1] & world.empty;
    for (; mask.mask != 0; ) {
        //cerr << "mask:\n" << mask << endl;
        const Point& cur = points[mask.removeRightMost()];
        World w = world;
        w.makeMove(cur, myindex);
        moves.emplace_back(deep == firstdeep ? -calcScore(w, myindex, deep, true, false) : 0, cur);
    }

    sort(moves.begin(), moves.end());

    for (const pair<int, Point>& m : moves) {
        const Point& cur = m.second;

        World w = world;
        if (w.makeMove(cur, myindex)) {
            //for (int i=0; i<4-deep; ++i)
                //cerr << ' ';
            //cerr << myindex << " made a move " << cur << endl;
            int score = 0;

            if (deep > 1) {
                pair<Point, int> bestMove = findBestMove(w, 3 - myindex, deep - 1, -beta, -alpha);

                if (bestMove.first.x == -1) {
                    // enemy could not make a move, however, the game is not finished yet
                    // I make another move
                    bestMove = findBestMove(w, myindex, deep - 1, alpha, beta);

                    if (bestMove.first.x == -1) {
                        // I cannot make a move too
                        // The game is over
                        score = calcScore(w, myindex, deep, true, true);
                    } else {
                        score = bestMove.second;
                    }
                } else {
                    score = -bestMove.second;
                }
            } else {
                score = calcScore(w, myindex, deep, true, false);
                //score = -m.first;
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = cur;
            }

            alpha = max(alpha, score);
            if (alpha >= beta)
                break;
        }
    }

    return {bestMove, bestScore};
}

int main(int argc, char** argv) {
    init();

    for (int turn = 0; ; ++turn) {
        start = std::chrono::system_clock::now();
        World world;
        if (!(cin >> world)) {
          break;
        }

        int myindex; cin >> myindex;

        Point mov;
        int score = -INF; cerr << "World:\n" << world << endl;
        cerr << "free cells: " << world.free << endl;
        int maxdepth = 0;
        check = false;

        for (int depth=6; ; ++depth) {
        //for (int depth=3; depth<=3; ++depth) {
            try {
                firstdeep = depth;
                pair<Point, int> bestMove = findBestMove(world, myindex, depth, -INF - 111, INF + 111);
                mov = bestMove.first;
                score = bestMove.second;
                cerr << "depth: " << depth << ", move: " << bestMove.first << ", score: " << bestMove.second << endl;

                check = true;
                maxdepth = depth;
            } catch (const TimeLimit& t) {
                break;
            }
        }

        cout << mov << endl;
        cerr << "Score " << score << " at depth " << maxdepth << endl;
        cerr << "Elapsed: " << elapsed() << endl;
    }
}
