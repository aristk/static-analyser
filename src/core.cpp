#include <iostream>
#include <string>
#include "analyzer/node.h"
#include "analyzer/satAnalyzer.hpp"
#include "core.hpp"

using namespace std;

extern NBlock* programBlock;
extern int yyparse();
extern FILE *yyin;

vector<pair<int, unsigned int> > parseAndAnalyze(const char *fileName) {

    FILE* file = fopen(fileName,"r");
    if(file == NULL) {
        throw couldNotOpenFile(fileName);
    }

    vector<pair<int, unsigned int> > answer(0);
    yyin = file; // now flex reads from file
    yyparse();
    fclose(file);

    if (programBlock == NULL) {
        throw isParserCrashed();
    }
    unique_ptr<SatStaticAnalyzer> analyzer(new SatStaticAnalyzer());
    analyzer->generateCheck(*programBlock);
    return analyzer->getAnswers();
};

int core(int argc, char **argv)
{
    int returnValue = 0;
    if(argc != 2) {
        printf("usage: ./staticAnalyzer filename\n");
        return 1;
    }

    try {
        parseAndAnalyze(argv[1]);
    } catch (exception& e) {
        cerr << "Exception caught:" << endl;
        cerr << e.what() << endl;
        returnValue = 1;
    }

    return returnValue;
}