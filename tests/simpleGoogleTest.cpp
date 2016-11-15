#include <iostream>

#include "gtest/gtest.h"
#include "src/core.hpp"

using namespace std;

// TODO: make positive tests more meaningful (check line and values)
TEST(testCore, PositiveTests) {

    vector<pair<int, unsigned int> > answers;

    char* argv[] = { "", "test/example.myprog" };

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleEQConst.myprog";

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleNEQConst.myprog";

    EXPECT_EQ(0, core(2, argv));

    argv[1] = "test/simpleEQ.myprog";

//    answers = parseAndAnalyze(argv[1]);

    EXPECT_EQ(0, core(2, argv));
}

// TODO: check for exception strings
TEST(testCore, NegativeTestsAnalyzerExceptions) {

    char* argv[] = { "", "test/global_var.myprog" };

    EXPECT_EQ(1, core(2, argv));

    argv[1] = "test/field_in_arg.myprog" ;

    EXPECT_EQ(1, core(2, argv));


    argv[1] = "test/myprog" ;

    EXPECT_EQ(1, core(2, argv));
}

// TODO: looks like crash of parser is due to one instance of it
TEST(testCore, NegativeTestsParserFails) {
    char* argv[] = { "", "test/assignment_in_arg.myprog" };

    EXPECT_EQ(1, core(2, argv));
}