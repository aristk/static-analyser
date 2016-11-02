#include <iostream>

#include "gtest/gtest.h"

extern int core(int argc, char **argv);

TEST(testCore, PositiveTests) {

    char* argv[] = { "", "test/example.myprog" };

    EXPECT_EQ(0, core(2, argv));
}

TEST(testCore, NegativeTests) {

    char* argv[] = { "", "test/global_var.myprog" };

    EXPECT_EQ(1, core(2, argv));

    argv[1] = "test/assignment_in_arg.myprog" ;

    EXPECT_EQ(1, core(2, argv));

    argv[1] = "test/field_in_arg.myprog" ;

    EXPECT_EQ(1, core(2, argv));
}