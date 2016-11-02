#include "analyzer.hpp"
#include "exceptions.hpp"

using namespace std;

NaiveStaticAnalyzer::NaiveStaticAnalyzer(NBlock *programBlock) {
    cout << "We are in NaiveStaticAnalyzer constructor" << endl;
    for(auto i : programBlock->statements) {
        // check that all root items are NFunctionDeclaration
        NFunctionDeclaration *NFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (NFunction == 0) {
            throw notNFunctionDeclaration();
        }
        this->functions.push_back(new FunctionInLanguage(NFunction->id.name));
        // TODO: add inputs, analyze what is an output

        for(auto j : NFunction->arguments) {
            NVariableDeclaration *NVariable = dynamic_cast<NVariableDeclaration*>(j);
            if (NVariable == 0) {
                // should not be called, since not allowed by parser
                throw NotNVariableDeclaration();
            }

            if (NVariable->id.field != "" || NVariable->assignmentExpr != NULL) {
                throw WrongFunctionArgument();
            }

        }
    }
}