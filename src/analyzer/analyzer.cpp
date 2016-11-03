#include "analyzer.hpp"
#include "exceptions.hpp"

using namespace std;

NaiveStaticAnalyzer::NaiveStaticAnalyzer(NBlock *programBlock) {

    // TODO: implement better verbosity usage
    cout << "We are in NaiveStaticAnalyzer constructor" << endl;
    for(auto i : programBlock->statements) {
        // check that all root items are NFunctionDeclaration
        // TODO: usage of dynamic_cast is not good idea
        NFunctionDeclaration *NFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (NFunction == 0) {
            throw notNFunctionDeclaration();
        }

        // TODO: look like better way is to all follows in construct of FunctionInLanguage
        this->functions.push_back(new FunctionInLanguage(NFunction->id.name));
        FunctionInLanguage *currentFunction = this->functions.back();

        // TODO: add inputs, analyze what is an output

        // add input variable
        for(auto j : NFunction->arguments) {
            // note that all inputs are of NVariableDeclaration type by parsing
            // TODO: add assertion with this check
            currentFunction->addInput(j);
        }
        currentFunction->processBody(NFunction->block);
    }
}

void FunctionInLanguage::addInput(NVariableDeclaration* NVariable) {
    if (NVariable->id.field != "" || NVariable->assignmentExpr != NULL) {
        throw WrongFunctionArgument();
    }
    string name = NVariable->id.name;
    this->inputs.push_back(name);
    this->variables.emplace(name, new InputVariable(name));
}

void FunctionInLanguage::processBody(NBlock &block) {
    for(auto i : block.statements) {
        // could be NVariableDeclaration or NExpressionStatement with NMethodCall
        this->processAssignment(i);
    }
}

FunctionInLanguage::~FunctionInLanguage() {
    // TODO: use shared_ptr for deleting Assignments
}

void FunctionInLanguage::processAssignment(NStatement *currentStatement) {
    NVariableDeclaration *nAssignment = dynamic_cast<NVariableDeclaration *>(currentStatement);
    if (nAssignment != 0) {
        string name = nAssignment->id.name;
        string nameWithField = getVariableName(&(nAssignment->id));
        // dedicated case if input variable is a structure and having assignment to its field
        if (variables.count(name) > 0) {
            // variable already exist in assignments
            Assignment *currentVariable = variables[name];
            string field = nAssignment->id.field;
            // variable could be input structure, so could be used as output
            if (field != "") {
                // process input
                if (currentVariable->getId() == 0) {
                    // TODO: make outputs a map
                    this->outputs.push_back(nameWithField);
                }
            }
        }
        variables[nameWithField] = evaluateAssignment(nAssignment->assignmentExpr);
    } else {
        // we have NMethodCall in currentStatement
    }
}

Assignment *FunctionInLanguage::evaluateAssignment(NExpression *currentExpression) {
    NIdentifier *currentIdentifier = dynamic_cast<NIdentifier *>(currentExpression);

    if (currentIdentifier != 0) {
        // simple case: we have identifier in rhs
        return variables[getVariableName(currentIdentifier)];
    }
    return nullptr;
    // TODO: check other options
}

string FunctionInLanguage::getVariableName(NIdentifier *currentIdentifier) {
    if (currentIdentifier->field != "") {
        return currentIdentifier->name + "." + currentIdentifier->field;
    } else {
        return currentIdentifier->name;
    }

}
