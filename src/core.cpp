#include <iostream>
#include "analyzer/node.h"
#include "analyzer/SatStaticAnalyzer.hpp"
#include "core.hpp"

using namespace std;

extern NBlock* programBlock;
extern int yyparse();
extern void yyset_lineno (int  line_number );
extern FILE *yyin;

vector<pair<int, unsigned int> > parseAndAnalyze(const char *fileName) {

    FILE* file = fopen(fileName,"r");
    if(file == nullptr) {
        throw couldNotOpenFile(fileName);
    }

    cout << "parsing file: " << fileName << endl;

    vector<pair<int, unsigned int> > answer(0);
    yyin = file; // now flex reads from file
    yyset_lineno (1);
    yyparse();
    fclose(file);

    unique_ptr<SatStaticAnalyzer> analyzer(new SatStaticAnalyzer());
//    unique_ptr<IncrementalSatStaticAnalyzer> analyzer(new IncrementalSatStaticAnalyzer());
    analyzer->addClauses(*programBlock);
    return analyzer->getAnswers();
};