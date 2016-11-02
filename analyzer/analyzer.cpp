#include "analyzer.hpp"

using namespace std;

NaiveStaticAnalyzer::NaiveStaticAnalyzer(NBlock *programBlock) {
    cout << "We are in NaiveStaticAnalyzer constructor" << endl;
    for(auto i : programBlock->statements)
    {
        // check that all root items are NFunctionDeclaration
        NFunctionDeclaration *NFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (NFunction==0) {
            throw notNFunctionDeclaration();
        }
        this->functions.push_back(new FunctionInLanguage(NFunction->id.name));
        // TODO: add inputs, analyze what is an output
    }
}