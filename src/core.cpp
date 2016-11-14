#include <iostream>
#include <string>
#include "analyzer/node.h"
#include "analyzer/satAnalyzer.hpp"

using namespace std;

extern NBlock* programBlock;
extern int yyparse();
extern FILE *yyin;

int core(int argc, char **argv)
{
    int returnValue = 0;
    if(argc != 2) {
        printf("usage: ./staticAnalyzer filename\n");
        exit(0);
    }
    FILE* file = fopen(argv[1],"r");
    if(file == NULL) {
        printf("couldn’t open %s\n", argv[1]);
        return 1;
    }
    yyin = file; // now flex reads from file
    yyparse();
    fclose(file);

    if (programBlock == NULL) {
        cout << isParserCrashed().what() << endl;
        return 1;
    }

//    z3::context c;
    unique_ptr<SatStaticAnalyzer> analyzer(new SatStaticAnalyzer());
    try {
        analyzer->generateCheck(*programBlock);
    } catch (exception& e) {
        cerr << "Exception caught:" << endl;
        cerr << e.what() << endl;
        returnValue = 1;
    }

    return returnValue;
}