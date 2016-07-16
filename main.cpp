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
    typedef unsigned __int128 uint128;
    uint128 mask;

    Mask() : mask{0} {}

    inline void setbit(int pos) {
        mask |= static_cast<uint128>(1) << pos;
    }

    inline void togglebit(int pos) {
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
        if (!getbit(pos))
            throw runtime_error("oh no no no");
        togglebit(pos);
        return pos;
    }
};


Point newp;

struct World {
    uint8_t s[N][N];
    Mask emptyFrontier;
    int ende;
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

                    if (s[newp.x][newp.y] == myindex)
                        break;

                    s[newp.x][newp.y] = myindex;
                }
            }
        }

        if (!canMove) return false;

        --free;

        s[p.x][p.y] = myindex;
        for (int dir=0; dir<DIR; ++dir) {
            newp.x = p.x + dx[dir];
            newp.y = p.y + dy[dir];

            if (inside(newp)) {
                emptyFrontier.setbit(index(newp.x, newp.y));
            }
        }

        return true;
    }

    void printFrontier(ostream& out) {
        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                out << emptyFrontier.getbit(index(i, u)) << ' ';
            }
            out << '\n';
        }
    }

    friend istream& operator >>(istream& in, World& obj) {
        obj.ende = 0;
        obj.free = N*N;
        obj.emptyFrontier = Mask{};

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                int cur;
                in >> cur;
                if (cur == 3)
                    cur = 0;
                obj.s[i][u] = cur;

                if (cur != 0)
                    --obj.free;
            }
        }

        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                if (obj.get(i, u) == 0) {
                    for (int dir=0; dir<DIR; ++dir) {
                        newp.x = i + dx[dir];
                        newp.y = u + dy[dir];
                        if (inside(newp) && obj.s[newp.x][newp.y] != 0) {
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

    int enemyindex = 3 - myindex;

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
//vector<int> order = {2,1,7,0,6,3,5,4};
//vector<int> order = {1,0,3,7,2,4,6,5};

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
    if (check && elapsed() > TIME_LIMIT) throw 42;

    //++g_sum;

    Point bestMove;
    int bestScore = -INF;

    Mask mask = world.emptyFrontier;
    for (; mask.mask; ) {
        const Point& cur = points[mask.removeRightMost()];
        //cerr << "trying position: " << cur << endl;
        if (world.s[cur.x][cur.y] != 0) continue;

        World w = world;
        if (w.makeMove(cur, myindex)) {
            int score = 0;
            score = calcScore(w, myindex);

            if (w.free != 0) {
                if (score == INF) {
                    score = INF + deep; // the sooner I win, the better
                } else if (deep == 1) {
                    //score = calcScore(world, myindex);
                    //cerr << " " << "Simple score = " << score << endl; 
                } else if (deep > 1 && score < INF) {
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
                } else {
                    throw runtime_error("Oh no no no");
                }
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = cur;
            }

            //alpha = max(alpha, score);
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

int main(int argc, char** argv) {
    initpoints();

    World world;
    cin >> world;

    int myindex; cin >> myindex;

    Point mov;

    cerr << "World:\n" << world << endl;

    for (int depth=2; depth<=5; ++depth) {
    //for (int depth=1; depth<=1; ++depth) {
        //cache.clear();
        try {
            pair<Point, int> bestMove = findBestMove(world, myindex, depth, -INF, INF);
            mov = bestMove.first;
            cerr << "depth: " << depth << ", move: " << bestMove.first << ", score: " << bestMove.second << endl;

            World w = world;
            w.makeMove(mov, myindex);
            //cerr << "after this move world:\n" << w << endl;

            check = true;
        } catch (...) {
            break;
        }
    }

    cout << mov << endl;
    cerr << "Elapsed: " << elapsed() << endl;
    //cerr << "g_sum = " << g_sum << endl;
#ifdef DEBUG
    ofstream fout("elapsed.txt", fstream::app);
    fout << elapsed() << endl;
#endif
}
