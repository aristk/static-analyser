#include "analyzer/node.h"
#include "satAnalyzer.hpp"

void NBlock::genCheck(SatStaticAnalyzer& context) const {
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

    // process body
    block.genCheck(context);
}

void NVariableDeclaration::genCheck(SatStaticAnalyzer &context) const {
    // if variable is a struct and is an input add it to outputs
    if((this->id.field != "") && (context.isCurrentInput(id))) {
        context.addReturn(id);
    }

    assignmentExpr->addClauses(id, context);
}

void NReturnStatement::genCheck(SatStaticAnalyzer &context) const {
    context.addReturn(this->variable);
}

void NMethodCall::genCheck(SatStaticAnalyzer &context) const {
    FullVariableName key = make_tuple("", "", "");
    context.mapMethodCall(*this, key);
}

void NMethodCall::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    FullVariableName key = make_tuple(context.getCurrentFunctionName(), nIdentifier.name, nIdentifier.field);
    context.mapMethodCall(*this, key);
}

//TODO: somehow merge into one call
void NInteger::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    FullVariableName key = make_tuple(context.getCurrentFunctionName(), nIdentifier.name, nIdentifier.field);
    context.addClauses(key, *this);
}

void NBinaryOperator::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}

void NIdentifier::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    FullVariableName key = make_tuple(context.getCurrentFunctionName(), nIdentifier.name, nIdentifier.field);
    context.addClauses(key, *this);
}

void NIdentifier::mapVariables(const string &functionName, const string &inputName, SatStaticAnalyzer &context) {
    FullVariableName key = make_tuple(functionName, inputName, "");
    context.addClauses(key, *this);
}

void NInteger::mapVariables(const string &functionName, const string &inputName, SatStaticAnalyzer &context) {
    FullVariableName key = make_tuple(functionName, inputName, "");
    context.addClauses(key, *this);
}