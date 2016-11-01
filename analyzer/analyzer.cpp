#include "analyzer.hpp"
#include <iostream>
#include <typeinfo>

using namespace std;

NaiveStaticAnalyzer::NaiveStaticAnalyzer(NBlock *programBlock) {
    cout << "We are in NaiveStaticAnalyzer constructor" << endl;
    for(auto i : programBlock->statements)
    {
        // check that all root items are NFunctionDeclaration
        NFunctionDeclaration *NFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (NFunction==0) {
            cerr << "root item is not NFunctionDeclaration.\n";
            return;
        }
        this->functions.push_back(new FunctionInLanguage(NFunction->id.name));

    }
}