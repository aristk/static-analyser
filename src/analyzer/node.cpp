#include "analyzer/node.h"
#include "satAnalyzer.hpp"


map<int, unsigned int> NInteger::intMapping;
unsigned int NInteger::differentIntCount = 0;

void NBlock::genCheck(SatStaticAnalyzer &context) const {
    for(auto i : statements) {
        if (i->isFunctionDeclaration()) {
            // TODO: add to the parser check that NFunctionDeclaration could not be here
            throw WrongFunctionStatement();
        }
        i->genCheck(context);
    }
}

void NFunctionDeclaration::genCheck(SatStaticAnalyzer &context) const {
    // process inputs
    context.addInputs(arguments, id);

    context.addBody(&block, id.name);

    // process body
    block.genCheck(context);
}

void NVariableDeclaration::genCheck(SatStaticAnalyzer &context) const {
    assignmentExpr->addClauses(id, context);
}

void NReturnStatement::genCheck(SatStaticAnalyzer &context) const {
    context.addTrueOutput(this->variable);
}

void NMethodCall::genCheck(SatStaticAnalyzer &context) const {
    NIdentifier key("", "", 0);
    context.mapMethodCall(*this, key);
}

void NMethodCall::addClauses(const NIdentifier &key, SatStaticAnalyzer &context) const {
    context.mapMethodCall(*this, key);
}

// key = this
void NInteger::addClauses(const NIdentifier &lhs, SatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

// key = this
void NBinaryOperator::addClauses(const NIdentifier &lhs, SatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

// lhs = this
void NIdentifier::addClauses(const NIdentifier &lhs, SatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

void NIdentifier::processCallInput(unsigned int inputId, SatStaticAnalyzer &context) {
    const string callFunctionName = context.getCurrentCall();

    FunctionDeclaration *calledFunction = context.getFunction(callFunctionName);

    string inputName = calledFunction->getInput(inputId);

    calledFunction->mapCallInput(inputName, name);
}

void NInteger::processCallInput(unsigned int inputId, SatStaticAnalyzer &context) {
    const string callFunctionName = context.getCurrentCall();

    FunctionDeclaration *calledFunction = context.getFunction(callFunctionName);

    string inputName = calledFunction->getInput(inputId);

    NIdentifier lhs(to_string(value) + inputName, 0);

    // we to map all inputs also that have integer values, for that we need a new name
    calledFunction->mapCallInput(inputName, lhs.name);

    context.addClauses(lhs, *this);
}
