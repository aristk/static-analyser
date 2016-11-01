#include <iostream>
#include <string>
using namespace std;
#include "parser/node.h"
extern NBlock* programBlock;
extern int yyparse();
extern FILE *yyin;

int main(int argc, char **argv)
{
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
    std::cout << programBlock << std::endl;
    return 0;
}