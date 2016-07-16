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
const int dx[DIR] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[DIR] = {-1, 0, 1, -1, 1, -1, 0, 1};

inline int index(int x, int y) {
    return x * N + y;
}

class TimeLimit : public std::exception {};

typedef unsigned __int128 uint128;

int isborder[N][N] = {
    {1,1,1,1,1, 1,1,1,1,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},

    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,0,0,0,0, 0,0,0,0,1},
    {1,1,1,1,1, 1,1,1,1,1},
};

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

bool inside(const Point& p) {
    return p.x >= 0 && p.y >= 0 && p.x < N && p.y < N;
}

Point points[128];

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

    int removeRightMost() {
        int pos = rightMost();
        togglebit(pos, 1);
        return pos;
    }

    int bitcount() const {
        return __builtin_popcountll(mask & 0xFFFFFFFFFFFFFFFFull) + __builtin_popcountll(mask >> 64);
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

Point newp;

struct World {
    uint8_t s[N][N];
    Mask emptyFrontier;
    Mask discs[2];
    int free;

    uint8_t get(int i, int u) const {
        return s[i][u];
    }

    bool makeMove(const Point& p, int myindex) {
        int enemyindex = 3 - myindex;

        bool canMove = false;
        for (int dir=0; dir<DIR; ++dir) {
            newp = p;
            bool hasEnemyInThisDir = false;
            bool hasMeInThisDir = false;
            for (int go=0; ; ++go) {
                newp.x += dx[dir];
                newp.y += dy[dir];

                if (!inside(newp)) break;

                if (s[newp.x][newp.y] == 0)
                    break;

                if (s[newp.x][newp.y] == enemyindex) {
                    hasEnemyInThisDir = true;
                }

                if (s[newp.x][newp.y] == myindex) {
                    canMove |= hasEnemyInThisDir;
                    hasMeInThisDir = true;
                    break;
                }
            }

            newp = p;
            if (canMove && hasMeInThisDir) {
                for (int go=0; ;++go) {
                    newp.x += dx[dir];
                    newp.y += dy[dir];

                    if (get(newp.x, newp.y) == myindex)
                        break;

                    s[newp.x][newp.y] = myindex;
                    discs[myindex-1].togglebit(index(newp.x, newp.y), 0);
                    discs[enemyindex-1].togglebit(index(newp.x, newp.y), 1);
                }
            }
        }

        if (!canMove) return false;

        --free;

        s[p.x][p.y] = myindex;
        emptyFrontier.togglebit(index(p.x, p.y), 1);

        for (int dir=0; dir<DIR; ++dir) {
            newp.x = p.x + dx[dir];
            newp.y = p.y + dy[dir];

            if (inside(newp) && get(newp.x, newp.y) == 0) {
                emptyFrontier.setbit(index(newp.x, newp.y));
            }
        }

        const int ind = index(p.x, p.y);
        discs[myindex-1].setbit(ind);

        return true;
    }

    friend istream& operator >>(istream& in, World& obj) {
        obj.free = N*N;
        obj.emptyFrontier = Mask{};
        obj.discs[0] = obj.discs[1] = Mask{};

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                int cur;
                in >> cur;
                if (cur == 3)
                    cur = 0;
                obj.s[i][u] = cur;

                if (cur != 0) {
                    --obj.free;
                    obj.discs[cur-1].setbit(index(i, u));
                }
            }
        }

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                if (obj.get(i, u) == 0) {
                    for (int dir=0; dir<DIR; ++dir) {
                        newp.x = i + dx[dir];
                        newp.y = u + dy[dir];
                        if (inside(newp) && obj.get(newp.x, newp.y) != 0) {
                            obj.emptyFrontier.setbit(index(i, u));
                            break;
                        }
                    }
                }
            }
        }

        return in;
    }

    friend ostream& operator <<(ostream& out, const World& obj) {
        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                out << static_cast<int>(obj.s[i][u]) << ' ';
            }
            out << '\n';
        }
        return out;
    }
};

int calcScore(const World& world, const int myindex, bool enemygo = true) {
    int score = 0;

    const int enemyindex = 3 - myindex;

    // check
    /*
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            if (world.get(i, u) == 0) {
                if (world.discs[0].getbit(index(i, u)) || world.discs[1].getbit(index(i, u)))
                    throw runtime_error("world is invalid");
            } else if (world.get(i, u) == 1) {
                if (!world.discs[0].getbit(index(i, u)))
                    throw runtime_error("world is invalid 1");
            } else if (world.get(i, u) == 2) {
                if (!world.discs[1].getbit(index(i, u)))
                    throw runtime_error("world is invalid 2");
            }
        }
    }
    */
    // end check

    if (enemygo) {
        if (world.discs[enemyindex - 1].mask == 0) {
            if (world.discs[myindex - 1].mask == 0)
                throw runtime_error("world is invalid 3");
            return INF;
        }
        if (world.discs[myindex - 1].mask == 0) {
            if (world.discs[enemyindex - 1].mask == 0)
                throw runtime_error("world is invalid 4");
            return -INF;
        }

        if (world.free == 0) {
            int mydiscs = world.discs[myindex - 1].bitcount();
            int enemydiscs = world.discs[enemyindex - 1].bitcount();
            if (mydiscs > enemydiscs) return INF;
            if (mydiscs < enemydiscs) return -INF;
            return 0;
        }
    }

    //if (enemygo)
        //cerr << "Calculating score for world: (index " << myindex << ")\n" << world << endl;

    Mask maskAroundFrontier;
    for (Mask emptyFrontier = world.discs[myindex-1]; emptyFrontier.mask;) {
        int index = emptyFrontier.removeRightMost();
        //cerr << "point " << points[index] << endl;
        maskAroundFrontier |= around[index];
    }

    //if (enemygo)
        //cerr << "maskAroundFrontier:\n" << maskAroundFrontier << endl;
    maskAroundFrontier &= world.emptyFrontier;
    //if (enemygo) {
        //cerr << "maskAroundFrontier after &:\n" << maskAroundFrontier << endl;
        //cerr << "its bitcount: " << maskAroundFrontier.bitcount() << endl;
    //}
    
    //score += world.discs[myindex].bitcount();
    score -= maskAroundFrontier.bitcount();

    // score for capturing the corner:
    Mask myCorner = cornerMask & world.discs[myindex - 1];
    score += myCorner.bitcount() * 100;

    /*
    int corner = 0;
    int border = 0;
    int total = 0;
    bool hasenemy = false;

    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            if (world.get(i, u) == myindex) {
                ++total;
                border += isborder[i][u];
                corner += iscorner[i][u];
            }
            hasenemy |= world.get(i, u) == enemyindex;
        }
    }

    if (!hasenemy) // evaporate him
        return INF;

    score += corner * 100;
    score += border * 10;
    score += total;
    */

    if (enemygo)
        score -= calcScore(world, 3 - myindex, false);

    return score;
}

const double TIME_LIMIT = 0.94;
bool check = false;
double elapsed() {
    return static_cast<double>(clock()) / CLOCKS_PER_SEC;
}

//int g_sum = 0;

pair<Point, int> findBestMove(const World& world, const int myindex, const int deep, int alpha, int beta) {
    /*
    const double alphaoriginal = alpha;
    CacheValue& cached = cache[world.hashpair()];
    if (cached.movscore.first.column != -1) {
        if (cached.type == CacheValue::Type::EXACT) return cached.movscore;
        if (cached.type == CacheValue::Type::LOWERBOUND) alpha = max(alpha, cached.movscore.second);
        if (cached.type == CacheValue::Type::UPPERBOUND) beta = min(beta, cached.movscore.second);
        if (alpha > beta) return cached.movscore;
    }
    */
    if (check && elapsed() > TIME_LIMIT) throw TimeLimit{};

    //++g_sum;

    Point bestMove;
    int bestScore = -INF - 100;

    Mask mask = world.emptyFrontier;
    for (; mask.mask != 0; ) {
        //cerr << "mask:\n" << mask << endl;
        const Point& cur = points[mask.removeRightMost()];
        if (world.get(cur.x, cur.y) != 0)
            throw runtime_error("empty frontier is invalid");

        World w = world;
        if (w.makeMove(cur, myindex)) {
            //for (int i=0; i<4-deep; ++i)
                //cerr << ' ';
            //cerr << myindex << " made a move " << cur << endl;
            int score = 0;
            score = calcScore(w, myindex);
            if (score == INF)
                score = INF + deep; // the sooner I win, the better
            if (score == -INF)
                score = -INF - deep; // the later I loose, the better
            //for (int i=0; i<4-deep; ++i)
                //cerr << ' ';
            //cerr << "score: " << score << endl;

            if (deep > 1 && w.free != 0 && score < INF - 100) {
                pair<Point, int> bestMove = findBestMove(w, 3 - myindex, deep - 1, -beta, -alpha);

                if (bestMove.first.x == -1) {
                    // enemy could not make a move, however, the game is not finished yet
                    // I make another move
                    bestMove = findBestMove(w, myindex, deep - 1, alpha, beta);

                    if (bestMove.first.x == -1) {
                        // I also cannot make a move
                        // The game is over, use the score already calculated
                    } else {
                        score = bestMove.second;
                    }
                } else {
                    score = -bestMove.second;
                }
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = cur;
            }

            alpha = max(alpha, score);
            if (alpha > beta) {
                break;
            }
        }
    }

    //cached.movscore = make_pair(bestMove, bestScore);
    //if (bestScore <= alphaoriginal) cached.type = CacheValue::Type::UPPERBOUND;
    //else if (bestScore >= beta) cached.type = CacheValue::Type::LOWERBOUND;
    //else cached.type = CacheValue::Type::EXACT;
    //return cached.movscore;

    return {bestMove, bestScore};
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
    cornerMask = Mask{};
    for (int i=0; i<N; ++i) {
        for (int u=0; u<N; ++u) {
            if (iscorner[i][u])
                cornerMask.setbit(index(i, u));
        }
    }
}

int main(int argc, char** argv) {
    initpoints();
    initaround();
    initcornermask();

    World world;
    cin >> world;

    int myindex; cin >> myindex;

    Point mov;
    int score = -INF;

    cerr << "World:\n" << world << endl;

    //world.makeMove(Point{1, 3}, myindex);

    //cerr << "after 1 3:\n" << world << endl;

    //return 0;

    for (int depth=4; depth<=8; ++depth) {
    //for (int depth=4; depth<=4; ++depth) {
        //cache.clear();
        try {
            pair<Point, int> bestMove = findBestMove(world, myindex, depth, -INF, INF);
            mov = bestMove.first;
            score = bestMove.second;
            cerr << "depth: " << depth << ", move: " << bestMove.first << ", score: " << bestMove.second << endl;

            World w = world;
            w.makeMove(mov, myindex);
            //cerr << "after this move world:\n" << w << endl;

            check = true;
        } catch (const TimeLimit& t) {
            break;
        }
    }

    cout << mov << endl;
    cout << score << endl;
    cerr << "Elapsed: " << elapsed() << endl;
    //cerr << "g_sum = " << g_sum << endl;
#ifdef DEBUG
    ofstream fout("elapsed.txt", fstream::app);
    fout << elapsed() << endl;
#endif
}
