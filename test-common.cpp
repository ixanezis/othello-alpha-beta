#include "common.h"

#include <gtest/gtest.h>

TEST(Common, TestAround) {
    {
        const MaskArray ans{{
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},

            {0,0,0,0, 0,1,1,1},
            {0,0,0,0, 0,1,0,1},
            {0,0,0,0, 0,1,1,1},
            {0,0,0,0, 0,0,0,0},
        }};

        ASSERT_EQ(toArray(around[index(5, 6)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,1,1},
            {0,0,0,0, 0,0,1,0},
            {0,0,0,0, 0,0,1,1},

            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
        }};

        ASSERT_EQ(toArray(around[index(2, 7)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},

            {0,0,0,0, 0,0,0,0},
            {0,0,0,0, 0,0,0,0},
            {1,1,0,0, 0,0,0,0},
            {0,1,0,0, 0,0,0,0},
        }};

        ASSERT_EQ(toArray(around[index(7, 0)]), ans);
    }
}

TEST(Common, TestCornerMask) {
    ASSERT_EQ(toArray(borderMask), isborder);
    ASSERT_EQ(toArray(cornerMask), iscorner);
}

TEST(Common, TestLineMask1) {
    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,1,1,1,1,1,1},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask1[1][1][getdir(0, 1)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {1,0,0,0,0,0,0,0},
            {0,1,0,0,0,0,0,0},
            {0,0,1,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask1[4][3][getdir(-1, -1)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,1},
            {0,0,0,0,0,0,1,0},
            {0,0,0,0,0,1,0,0},
            {0,0,0,0,1,0,0,0},
            {0,0,0,1,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask1[7][2][getdir(-1, 1)]), ans);
    }
}

TEST(Common, TestLineMask2) {
    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask2[1][1][index(1, 6)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask2[4][3][index(4, 3)]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,1,0},
            {0,0,0,0,0,1,0,0},
            {0,0,0,0,1,0,0,0},
            {0,0,0,1,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(lineMask2[7][2][index(2, 7)]), ans);
    }
}

int toMask(const std::string& s) {
    int mask = 0;
    assert(s.size() == N);
    for (int i = 0; i < N; ++i) {
        if (s[i] == '1') {
            mask |= 1 << i;
        }
    }
    return mask;
}

TEST(Common, TestAroundMask) {
    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0,0},
            {0,0,1,1,1,1,0,0},
            {0,0,1,1,1,1,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(aroundMask[2][toMask("00011000")]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,1,1,1,1,1,1},
            {0,0,1,1,1,1,1,1},
            {0,0,1,1,1,1,1,1},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(aroundMask[2][toMask("00011010")]), ans);
        ASSERT_EQ(toArray(aroundMask[2][toMask("00011110")]), ans);
        ASSERT_EQ(toArray(aroundMask[2][toMask("00011011")]), ans);
        ASSERT_EQ(toArray(aroundMask[2][toMask("00011111")]), ans);
    }

    {
        const MaskArray ans{{
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0},
            {1,1,1,0,0,0,0,0},
            {1,1,1,0,0,0,0,0},
        }};

        ASSERT_EQ(toArray(aroundMask[7][toMask("11000000")]), ans);
        ASSERT_EQ(toArray(aroundMask[7][toMask("01000000")]), ans);
        ASSERT_NE(toArray(aroundMask[7][toMask("10000000")]), ans);
    }
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
    init();
	return RUN_ALL_TESTS();
}
