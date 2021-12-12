#include "test_macros.h"
#include <iostream>
#include "../bit_ops.h"

TEST(BitOps_Lock) {
    Entry e = 0b1000000001;
    EXPECT_TRUE(is_locked(e));

    e = 0b0000000001;
    EXPECT_FALSE(is_locked(e));
}


TEST(BitOps_CountBits) {
    Entry e = 0b100110001;
    EXPECT_EQ(4, count_bits(e));

    e = 0b000100000;
    EXPECT_EQ(1, count_bits(e));

    e = 0b000000000;
    EXPECT_EQ(0, count_bits(e));
}

TEST(BitOps_HasBits) {
    Entry e = 0b100110001;
    EXPECT_TRUE(has_bit(e,1));
    EXPECT_FALSE(has_bit(e, 2));
    EXPECT_FALSE(has_bit(e, 4));
    EXPECT_TRUE(has_bit(e, 5));
    EXPECT_TRUE(has_bit(e, 6));
    EXPECT_FALSE(has_bit(e, 7));
    EXPECT_FALSE(has_bit(e, 8));
    EXPECT_TRUE(has_bit(e, 9));


    e = 0b000000000;
    EXPECT_FALSE(has_bit(e, 1));
}

TEST(BitOps_LowestBit) {
    Entry e = 0b100110001;
    EXPECT_EQ(1, lowest_bit(e));

    e = 0b000100100;
    EXPECT_EQ(3, lowest_bit(e));

    e = 0b000000000;
    EXPECT_EQ(0, lowest_bit(e));
}

TEST(BitOps_SingleValue) {
    Entry e = 0b100110001;
    EXPECT_FALSE(has_single_value(e));

    e = 0b000100100;
    EXPECT_FALSE(has_single_value(e));

    e = 0b000000000;
    EXPECT_FALSE(has_single_value(e));

    e = 0b1000100000; // lock bit not counted
    EXPECT_TRUE(has_single_value(e));

    e = 0b100000000;
    EXPECT_TRUE(has_single_value(e));
}


TEST(BitOps_RemoveValues) {
    Entry a = 0b100110001;
    Entry b = 0b100000001;
    remove_values(a, b);
    Entry c = 0b000110000;
    EXPECT_EQ(c, a);

    a = 0b0100110001;
    b = 0b1000000001;
    remove_values(a, b);
    c = 0b0100110000;
    EXPECT_EQ(c, a);

    a = 0b1000000001;
    b = 0b0000000001;
    remove_values(a, b);
    c = 0b1000000001; // locked, no change
    EXPECT_EQ(c, a);
}