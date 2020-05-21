#include <cstdint>
#include <ostream>
#include <stdexcept>

using uint128 = unsigned __int128;

struct Mask {
    static constexpr int N = 10;
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
            throw std::runtime_error("previous bit value is invalid");
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

    friend std::ostream& operator <<(std::ostream& out, const Mask& obj) {
        for (int i=0; i<N; ++i) {
            for (int u=0; u<N; ++u) {
                out << obj.getbit(i * N + u) << ' ';
            }
            out << '\n';
        }
        return out;
    }
};
