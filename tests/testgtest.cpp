#include <iostream>

#include "gtest/gtest.h"

extern int core(int argc, char **argv);

TEST(sample_test_case, sample_test) {

    char* argv[] = { "", "example.myprog" };

    EXPECT_EQ(0, core(2, argv));
}