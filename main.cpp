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

using namespace std;

const int N = 10;
const int INF = 0x7f7f7f7f;

const int DIR = 8;
const int dx[DIR] = {0, -1,-1,-1, 0, 1, 1, 1};
const int dy[DIR] = {-1,-1, 0, 1, 1, 1, 0,-1};

inline int index(int x, int y) {
    return x * N + y;
}

class TimeLimit : public std::exception {};

typedef unsigned __int128 uint128;

int iscorner[N][N] = {
    {1,0,0,0,0, 0,0,0,0,1},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},

    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {0,0,0,0,0, 0,0,0,0,0},
    {1,0,0,0,0, 0,0,0,0,1},
};

struct Point {
    int x, y;

    Point() : x{-1}, y{-1} {}
    Point(int x_, int y_) : x{x_}, y{y_} {}

    bool operator == (const Point& ot) const {
        return x == ot.x && y == ot.y;
    }

    bool operator != (const Point& ot) const {
        return !(*this == ot);
    }

    friend ostream& operator <<(ostream& out, const Point& obj) {
        return out << obj.x << ' ' << obj.y;
    }
};

int DC[N][N];
Point D[N][N][8];

bool inside(const Point& p) {
    return p.x >= 0 && p.y >= 0 && p.x < N && p.y < N;
}

void checkinside(const Point& p) {
    if (!inside(p))
        throw runtime_error("point must be inside the world");
}

Point points[128];

struct Mask {
    uint128 mask;

    Mask() : mask{0} {}

    inline void setbit(int pos) {
        mask |= static_cast<uint128>(1) << pos;
    }

    inline void togglebit(int pos) {
        mask ^= static_cast<uint128>(1) << pos;
    }

    inline void togglebit(int pos, int prv) {
        if (getbit(pos) != prv)
            throw runtime_error("previous bit value is invalid");
        mask ^= static_cast<uint128>(1) << pos;
    }

    inline int getbit(int pos) const {
        return (mask & (static_cast<uint128>(1) << pos)) != 0;
    }

    int rightMost() const {
        unsigned long long lopart = mask & 0xFFFFFFFFFFFFFFFFull;
        int lo = __builtin_ctzll(lopart);
        int hi = __builtin_ctzll(mask >> 64);
        int islopartnull = lopart == 0;
        return lo * (islopartnull ^ 1) + (hi + 64) * islopartnull;
    }

    int leftMost() const {
        unsigned long long hipart = mask >> 64;
        int lo = __builtin_clzll(mask & 0xFFFFFFFFFFFFFFFFull);
        int hi = __builtin_clzll(hipart);
        int ishipartnull = hipart == 0;
        return (63 - lo) * ishipartnull + (127 - hi) * (ishipartnull ^ 1);
    }

    int closest(const int dir) const {
        if (dir < 4) return leftMost();
        return rightMost();
    }

    int removeRightMost() {
        int pos = rightMost();
        togglebit(pos, 1);
        return pos;
    }

    int bitcount() const {
        return __builtin_popcountll(mask & 0xFFFFFFFFFFFFFFFFull) + __builtin_popcountll(mask >> 64);
    }

    Mask operator ~() const {
        Mask ret = *this;
        ret.mask = ~ret.mask;
        return ret;
    }

    Mask& operator &= (const Mask& ot) {
        mask &= ot.mask;
        return *this;
    }

    Mask operator & (const Mask& ot) const {
        Mask ret = *this;
        ret &= ot;
        return ret;
    }

    Mask& operator |= (const Mask& ot) {
        mask |= ot.mask;
        return *this;
    }

    Mask operator | (const Mask& ot) const {
        Mask ret = *this;
        ret |= ot;
        return ret;
    }

    Mask& operator ^= (const Mask& ot) {
        mask ^= ot.mask;
        return *this;
    }

    Mask operator ^ (const Mask& ot) const {
        Mask ret = *this;
        ret ^= ot;
        return ret;
    }

    friend ostream& operator <<(ostream& out, const Mask& obj) {
        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                out << obj.getbit(index(i, u)) << ' ';
            }
            out << '\n';
        }
        return out;
    }
};

Mask around[128];
Mask cornerMask;
Mask lineMask1[N][N][DIR];
Mask lineMask2[N][N][N*N];
Mask lineAround2[N][N][N*N];

Point newp;

struct World {
    Mask emptyFrontier[2];
    Mask discs[2];
    int free;

    pair<uint128, uint128> hashpair() const {
        return {discs[0].mask, discs[1].mask};
    }

    bool makeMove(const Point& p, const int myindex) {
        const int enemyindex = 3 - myindex;

        Mask empty = ~(discs[0] | discs[1]);

        Mask totalMask;
        Mask totalLineAround;
        for (int dir=0; dir<DIR; ++dir) {
            const Mask myInDir = discs[myindex - 1] & lineMask1[p.x][p.y][dir];

            if (myInDir.mask != 0) {
                int closest = myInDir.closest(dir);

                if ((empty & lineMask2[p.x][p.y][closest]).mask != 0) // this direction intersects an empty region
                    continue;

                totalMask |= lineMask2[p.x][p.y][closest];
                totalLineAround |= lineAround2[p.x][p.y][closest];
            }
        }

        if (totalMask.mask == 0) return false; // cannot make a move

        --free;

        const int ind = index(p.x, p.y);

        discs[enemyindex - 1] ^= totalMask;
        discs[myindex - 1] ^= totalMask;
        discs[myindex - 1].togglebit(ind, 0);

        emptyFrontier[myindex - 1] |= totalLineAround;
        empty.togglebit(ind, 1);
        emptyFrontier[0] &= empty;
        emptyFrontier[1] &= empty;

        return true;
    }

    friend istream& operator >>(istream& in, World& obj) {
        obj.free = N*N;
        obj.emptyFrontier[0] = obj.emptyFrontier[1] = Mask{};
        obj.discs[0] = obj.discs[1] = Mask{};

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                int cur;
                in >> cur;
                if (cur == 3)
                    cur = 0;

                if (cur != 0) {
                    --obj.free;
                    obj.discs[cur-1].setbit(index(i, u));
                }
            }
        }

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                const int ind = index(i, u);
                for (int player=0; player<2; ++player) {
                    if (obj.discs[player].getbit(ind)) {
                        obj.emptyFrontier[player] |= around[ind];
                    }
                }
            }
        }

        Mask empty = ~(obj.discs[0] | obj.discs[1]);
        obj.emptyFrontier[0] &= empty;
        obj.emptyFrontier[1] &= empty;

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

int calcScore(const World& world, const int myindex, int deep, bool enemygo = true) {
    int score = 0;

    const int enemyindex = 3 - myindex;

    if (enemygo) {
        if (world.discs[enemyindex - 1].mask == 0) {
            //if (world.discs[myindex - 1].mask == 0)
                //throw runtime_error("world is invalid 3");
            return INF + deep; // the sooner I win, the better
        }
        if (world.discs[myindex - 1].mask == 0) {
            //if (world.discs[enemyindex - 1].mask == 0)
                //throw runtime_error("world is invalid 4");
            return -INF - deep; // the later I loose, the better
        }

        if (world.free == 0) {
            int mydiscs = world.discs[myindex - 1].bitcount();
            int enemydiscs = world.discs[enemyindex - 1].bitcount();
            if (mydiscs > enemydiscs) return INF + deep;
            if (mydiscs < enemydiscs) return -INF - deep;
            return 0;
        }
    }

    //if (enemygo)
        //cerr << "Calculating score for world: (index " << myindex << ")\n" << world << endl;

    //score += world.discs[myindex - 1].bitcount();

    score -= world.emptyFrontier[myindex - 1].bitcount();

    // score for capturing the corner:
    Mask myCorner = cornerMask & world.discs[myindex - 1];
    score += myCorner.bitcount() * 100;

    if (enemygo)
        score -= calcScore(world, 3 - myindex, deep, false);

    /*
    if (score == 16 && deep == 1) {
        cerr << "score is 16:\n" << world << endl;
        cerr << "emptyFrontiers:\n" << endl;
        cerr << world.emptyFrontier[0] << endl << world.emptyFrontier[1] << endl;
    }
    */
    return score;
}

const double TIME_LIMIT = 0.94;
bool check = false;
double elapsed() {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

int g_sum = 0;

struct CacheValue {
    enum class Type {
        EXACT,
        LOWERBOUND,
        UPPERBOUND,
        UNKNOWN
    };
    pair<Point, int> movscore;
    Type type;
    
    CacheValue() : type(Type::UNKNOWN) {}
};

map<pair<uint128, uint128>, CacheValue> cache;

pair<Point, int> findBestMove(const World& world, const int myindex, const int deep, int alpha, int beta) {
    const double alphaoriginal = alpha;
    CacheValue& cached = cache[world.hashpair()];
    if (cached.movscore.first.x != -1) {
        if (cached.type == CacheValue::Type::EXACT) return cached.movscore;
        if (cached.type == CacheValue::Type::LOWERBOUND) alpha = max(alpha, cached.movscore.second);
        if (cached.type == CacheValue::Type::UPPERBOUND) beta = min(beta, cached.movscore.second);
        if (alpha > beta) return cached.movscore;
    } else {
        cached.movscore.second = -INF - 100;
    }

    if (check && elapsed() > TIME_LIMIT) throw TimeLimit{};

    const int enemyindex = 3 - myindex;

    Point& bestMove = cached.movscore.first;
    int& bestScore = cached.movscore.second;

    Mask mask = world.emptyFrontier[enemyindex - 1];
    for (; mask.mask != 0; ) {
        //cerr << "mask:\n" << mask << endl;
        const Point& cur = points[mask.removeRightMost()];
        //if ((world.discs[0] | world.discs[1]).getbit(index(cur.x, cur.y)))
            //throw runtime_error("empty frontier is invalid");

        World w = world;
        if (w.makeMove(cur, myindex)) {
            //for (int i=0; i<4-deep; ++i)
                //cerr << ' ';
            //cerr << myindex << " made a move " << cur << endl;
            int score = 0;

            if (deep > 1 && w.free != 0) {
                pair<Point, int> bestMove = findBestMove(w, 3 - myindex, deep - 1, -beta, -alpha);

                if (bestMove.first.x == -1) {
                    // enemy could not make a move, however, the game is not finished yet
                    // I make another move
                    bestMove = findBestMove(w, myindex, deep - 1, alpha, beta);

                    if (bestMove.first.x == -1) {
                        // I also cannot make a move
                        // The game is over
                        score = calcScore(w, myindex, deep);
                    } else {
                        score = bestMove.second;
                    }
                } else {
                    score = -bestMove.second;
                }
            } else {
                score = calcScore(w, myindex, deep);
            }

            //for (int i=0; i<4-deep; ++i)
                //cerr << ' ';
            //cerr << "score: " << score << endl;

            if (score > bestScore) {
                bestScore = score;
                bestMove = cur;
            }

            alpha = max(alpha, score);
            if (alpha >= beta)
                break;
        }
    }

    //return {bestMove, bestScore};

    //cached.movscore = make_pair(bestMove, bestScore);
    if (bestScore <= alphaoriginal) cached.type = CacheValue::Type::UPPERBOUND;
    else if (bestScore >= beta) cached.type = CacheValue::Type::LOWERBOUND;
    else cached.type = CacheValue::Type::EXACT;

    return cached.movscore;
}

void initpoints() {
    for (int i=0; i<128; ++i) {
        points[i].x = i / N;
        points[i].y = i % N;
    }
}

void initaround() {
    for (int i=0; i<128; ++i) {
        const Point& cur = points[i];
        for (int dir=0; dir<DIR; ++dir) {
            newp.x = cur.x + dx[dir];
            newp.y = cur.y + dy[dir];
            if (inside(newp)) {
                around[i].setbit(index(newp.x, newp.y));
            }
        }
    }
}

void initcornermask() {
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            if (iscorner[i][u])
                cornerMask.setbit(index(i, u));
        }
    }
}

void initD() {
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            Point cur{i, u};
            for (int dir=0; dir<DIR; ++dir) {
                newp.x = cur.x + dx[dir];
                newp.y = cur.y + dy[dir];

                if (inside(newp)) {
                    D[i][u][DC[i][u]] = Point{dx[dir], dy[dir]};
                    ++DC[i][u];
                }
            }
        }
    }
}

void initLineMasks() {
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            Point cur{i, u};
            for (int dir=0; dir<DIR; ++dir) {
                newp = cur;
                Mask curLineAround = around[index(i, u)];

                for (;;) {
                    newp.x += dx[dir];
                    newp.y += dy[dir];
                    
                    if (!inside(newp)) break;

                    const int ind = index(newp.x, newp.y);
                    lineMask2[i][u][ind] = lineMask1[i][u][dir];
                    lineMask1[i][u][dir].togglebit(ind, 0);

                    lineAround2[i][u][ind] = curLineAround;
                    curLineAround |= around[ind];
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    initpoints();
    initaround();
    initcornermask();
    initD();
    initLineMasks();

    World world;
    cin >> world;

    int myindex; cin >> myindex;

    Point mov;
    int score = -INF;

    cerr << "World:\n" << world << endl;
    cerr << "free cells: " << world.free << endl;
    int maxdepth = 0;

    for (int depth=6; depth<=11; ++depth) {
    //for (int depth=9; depth<=9; ++depth) {
        cache.clear();
        try {
            pair<Point, int> bestMove = findBestMove(world, myindex, depth, -INF - 111, INF + 111);
            mov = bestMove.first;
            score = bestMove.second;
            cerr << "depth: " << depth << ", move: " << bestMove.first << ", score: " << bestMove.second << endl;

            //cerr << "g_sum: " << g_sum << endl;
            //cerr << "visitedPositions: " << visitedPositions.size() << endl;
            //World w = world;
            //w.makeMove(mov, myindex);
            //cerr << "after this move world:\n" << w << endl;
            //cerr << "after this move empty frontier:\n" << w.emptyFrontier[0] << endl;
            //cerr << "after this move empty frontier:\n" << w.emptyFrontier[1] << endl;

            check = true;
            maxdepth = depth;
        } catch (const TimeLimit& t) {
            break;
        }
    }

    cout << mov << endl;
    cout << score << ' ' << maxdepth << endl;
    cerr << "Elapsed: " << elapsed() << endl;
    //cerr << "g_sum = " << g_sum << endl;
#ifdef DEBUG
    ofstream fout("elapsed.txt", fstream::app);
    fout << elapsed() << endl;
#endif
}
