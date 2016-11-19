#include "analyzer/node.h"
#include "incSatAnalyzer.hpp"


map<int, unsigned int> NInteger::intMapping;
unsigned int NInteger::differentIntCount = 0;

void NBlock::genCheck(IncrementalSatStaticAnalyzer &context) const {
    for(auto i : statements) {
        if (i->isFunctionDeclaration()) {
            // TODO: add to the parser check that NFunctionDeclaration could not be here
            throw WrongFunctionStatement();
        }
        i->genCheck(context);
    }
}

void NFunctionDeclaration::genCheck(IncrementalSatStaticAnalyzer &context) const {
    // process inputs
    context.addInputs(arguments, id);

    context.addBody(&block, id.name);

    // process body
    block.genCheck(context);
}

void NVariableDeclaration::genCheck(IncrementalSatStaticAnalyzer &context) const {
    assignmentExpr->addClauses(id, context);
}

void NReturnStatement::genCheck(IncrementalSatStaticAnalyzer &context) const {
    context.addTrueOutput(this->variable);
}

void NMethodCall::genCheck(IncrementalSatStaticAnalyzer &context) const {
    NIdentifier key("", "", 0);
    context.mapMethodCall(*this, key);
}

void NMethodCall::addClauses(const NIdentifier &key, IncrementalSatStaticAnalyzer &context) const {
    context.mapMethodCall(*this, key);
}

// key = this
void NInteger::addClauses(const NIdentifier &lhs, IncrementalSatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

// key = this
void NBinaryOperator::addClauses(const NIdentifier &lhs, IncrementalSatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

// lhs = this
void NIdentifier::addClauses(const NIdentifier &lhs, IncrementalSatStaticAnalyzer &context) const {
    context.addClauses(lhs, *this);
}

// map call arguments
void NIdentifier::processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {
    const string callFunctionName = context.getCurrentCall();

    FunctionDeclaration *calledFunction = context.getFunction(callFunctionName);

    string inputName = calledFunction->getInput(inputId);
    string parentFunctionName = context.getParentCall();
    FunctionDeclaration *parentFunction = context.getFunction(parentFunctionName);

    unordered_set<string> usageOfInput = calledFunction->getUsageOfInputs(inputName);

    // handle conner case with struct as call argument
    if (field != "") {
        throw WrongFunctionArgument();
    }

    for(auto i : usageOfInput) {
        FullVariableName lhs(callFunctionName, inputName, i);
        FullVariableName rhs(parentFunctionName, name, i);
        context.addClauses(lhs, rhs);

        // update context
        if ((i != "") && (parentFunctionName == context.getTopOfStack())) {
            parentFunction->addRhsUsage(inputName, i);
        }
    }
}

// map call outputs
void NIdentifier::processCallOutput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {
    const string callFunctionName = context.getCurrentCall();

    FunctionDeclaration *calledFunction = context.getFunction(callFunctionName);

    string inputName = calledFunction->getInput(inputId);
    string parentFunctionName = context.getParentCall();

    unordered_set<string> usageOfInput = calledFunction->getOutputOfInputs(inputName);

    FunctionDeclaration *parentFunction = context.getFunction(parentFunctionName);

    for(auto i : usageOfInput) {
        FullVariableName lhs(parentFunctionName, name, i);
        FullVariableName rhs(callFunctionName, inputName, i);
        context.addClauses(lhs, rhs);

        // update context
        if ((i != "") && (parentFunctionName == context.getTopOfStack())) {
            parentFunction->addLhsUsage(name, i);
        }
    }
}

void NInteger::processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {
    const string callFunctionName = context.getCurrentCall();

    FunctionDeclaration *calledFunction = context.getFunction(callFunctionName);

    string inputName = calledFunction->getInput(inputId);

    NIdentifier lhs(inputName, 0);

    context.addClauses(lhs, *this);
}
