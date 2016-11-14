#include <iostream>

#include "gtest/gtest.h"

extern int core(int argc, char **argv);

// TODO: make positive tests more meaningful (check line and values)
TEST(testCore, PositiveTests) {

    char* argv[] = { "", "test/example.myprog" };

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleEQConst.myprog";

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleNEQConst.myprog";

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleEQ.myprog";

    EXPECT_EQ(0, core(2, argv));
}

TEST(testCore, NegativeTestsAnalyzerExceptions) {

    char* argv[] = { "", "test/global_var.myprog" };

    EXPECT_EQ(1, core(2, argv));

    argv[1] = "test/field_in_arg.myprog" ;

    EXPECT_EQ(1, core(2, argv));

}

/* looks like fail of parser crashing is not concurrent
TEST(testCore, NegativeTestsParserFails) {
    char* argv[] = { "", "test/assignment_in_arg.myprog" };

    EXPECT_EQ(1, core(2, argv));
}
 */