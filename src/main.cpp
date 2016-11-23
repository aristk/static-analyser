#include "core.hpp"
#include <iostream>

using namespace std;

int main(int argc, char **argv)
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