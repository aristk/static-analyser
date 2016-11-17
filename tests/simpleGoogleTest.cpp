#include <iostream>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "src/core.hpp"
#include "src/analyzer/exceptions.hpp"

using namespace std;

using namespace ::testing;

TEST(testCore, PositiveTests) {

    vector<pair<int, unsigned int> > answers;
    string inputFileName;
    vector<pair<int, int> > pattern;

    inputFileName = "test/originalExample.myprog";
    answers = parseAndAnalyze(inputFileName.c_str());
    pattern = {{0, 15}, {1, 21}, {1, 28}};
    ASSERT_THAT(answers,
                ElementsAreArray(pattern));

    inputFileName = "test/sameNameInOutput.myprog";
    answers = parseAndAnalyze(inputFileName.c_str());
    pattern = {{0, 15}, {0, 20}, {1, 26}, {1, 33}};
    ASSERT_THAT(answers,
                ElementsAreArray(pattern));

    inputFileName = "test/simpleEQConst.myprog";
    answers = parseAndAnalyze(inputFileName.c_str());
    pattern = {{1,4}, {0,8}, {0, 12}, {0, 16}, {0, 20},
               {1, 24}, {0, 28}, {0, 32}, {0, 36}, {0, 40},
               {1, 44}, {0, 48}, {0, 52}, {0, 56}, {0, 60}, {1, 64}};
    ASSERT_THAT(answers,
                ElementsAreArray(pattern));

    inputFileName = "test/simpleNEQConst.myprog";
    answers = parseAndAnalyze(inputFileName.c_str());
    pattern = {{0,4}, {1,8}, {1, 12}, {1, 16}, {1, 20},
              {0, 24}, {1, 28}, {1, 32}, {1, 36}, {1, 40},
              {0, 44}, {1, 48}, {1, 52}, {1, 56}, {1, 60}, {0, 64}};
    ASSERT_THAT(answers,
                ElementsAreArray(pattern));

    inputFileName = "test/simpleEQ.myprog";
    answers = parseAndAnalyze(inputFileName.c_str());
    ASSERT_THAT(answers, ElementsAre(Pair(1, 4), Pair(0, 8)));
}

TEST(testCore, NegativeTestsAnalyzerExceptions) {

    string inputFileName;
    inputFileName = "test/fieldInArg.myprog";

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), InputIsAStruct);

    inputFileName = "test/globalVar.myprog" ;

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), notNFunctionDeclaration);

    inputFileName = "test/myprog" ;

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), couldNotOpenFile);

    inputFileName = "test/diffArgs.myprog" ;

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), differentNumberOfArgsInFunctionCall);

    inputFileName = "test/recursiveCall.myprog" ;

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), recursiveCall);
}

// TODO: looks like crash of parser is due to one instance of it
TEST(testCore, NegativeTestsParserFails) {
    string inputFileName = "test/assignmentInArg.myprog";

    ASSERT_THROW(parseAndAnalyze(inputFileName.c_str()), parserError);
}