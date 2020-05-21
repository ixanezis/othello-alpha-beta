#include "mask10.h"

#include <gtest/gtest.h>

TEST(Mask, TestSetBit) {
    Mask m;
    ASSERT_EQ(m.mask, 0);

    m.setbit(0);
	ASSERT_EQ(m.mask, 1);
    m.setbit(1);
	ASSERT_EQ(m.mask, 3);
    m.setbit(1);
	ASSERT_EQ(m.mask, 3);
    m.setbit(4);
	ASSERT_EQ(m.mask, 19);
    m.setbit(55);
	ASSERT_EQ(m.mask, 36028797018963987);

    m.togglebit(5);
	ASSERT_EQ(m.mask, 36028797018964019);
    m.togglebit(1);
	ASSERT_EQ(m.mask, 36028797018964017);
    m.togglebit(11, 0);
	ASSERT_EQ(m.mask, 36028797018966065);
    m.togglebit(11, 1);
	ASSERT_EQ(m.mask, 36028797018964017);

    ASSERT_THROW(m.togglebit(11, 1), std::runtime_error);

    ASSERT_EQ(m.getbit(58), 0);
    ASSERT_EQ(m.getbit(4), 1);

    Mask max;
    for (int i = 0; i < 64; ++i) {
        max.setbit(i);
    }
    ASSERT_EQ(max.mask, std::numeric_limits<uint64_t>::max());
    max.togglebit(33);
    ASSERT_NE(max.mask, std::numeric_limits<uint64_t>::max());
}

TEST(Mask, LeftRightMost) {
    Mask m;
    m.setbit(17);
    ASSERT_EQ(m.rightMost(), 17);
    ASSERT_EQ(m.leftMost(), 17);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
