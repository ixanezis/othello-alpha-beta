#pragma once

#include <cstdint>
#include <ostream>
#include <stdexcept>

struct Mask {
    static constexpr int N = 8;
    uint64_t mask;

    Mask() : mask{0} {}

    inline void setbit(int pos) {
        mask |= static_cast<uint64_t>(1) << pos;
    }

    inline void togglebit(int pos) {
        mask ^= static_cast<uint64_t>(1) << pos;
    }

    inline void togglebit(int pos, int prv) {
        if (getbit(pos) != prv)
            throw std::runtime_error("previous bit value is invalid");
        mask ^= static_cast<uint64_t>(1) << pos;
    }

    inline int getbit(int pos) const {
        return (mask & (static_cast<uint64_t>(1) << pos)) != 0;
    }

    int rightMost() const {
        return __builtin_ctzll(mask);
    }

    int leftMost() const {
        return 63 - __builtin_clzll(mask);
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
        return __builtin_popcountll(mask);
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
