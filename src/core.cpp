#include <iostream>
#include <string>
#include "parser/node.h"
#include "analyzer/analyzer.hpp"

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
        exit(0);
    }
    yyin = file; // now flex reads from file
    yyparse();
    fclose(file);

//    std::cout << programBlock << std::endl;

    // TODO: properly delete programBlock
    StaticAnalyzer *analyzer = NULL;
    try {
         analyzer = new SymbolicStaticAnalyzer(programBlock);
    } catch(exception& e) {
        cerr << "Exception caught:" << endl;
        cerr << e.what() << endl;
        returnValue = 1;
    }
    delete(analyzer);
    return returnValue;
}