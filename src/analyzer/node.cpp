#include "analyzer/node.h"
#include "satAnalyzer.hpp"

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
    // if variable is a struct and is an input add it to outputs
    if((this->id.field != "") && (context.isCurrentInput(id))) {
        context.addOutput(id);
    }

    FullVariableName key = make_tuple(context.getCurrentFunctionName(), id.name, id.field);
    assignmentExpr->addClauses(key, context);
}

void NReturnStatement::genCheck(SatStaticAnalyzer &context) const {
    context.addTrueOutput(this->variable);
}

void NMethodCall::genCheck(SatStaticAnalyzer &context) const {
    FullVariableName key = make_tuple("", "", "");
    context.mapMethodCall(*this, key);
}

void NMethodCall::addClauses(FullVariableName &key, SatStaticAnalyzer &context) {
    context.mapMethodCall(*this, key);
}

//TODO: somehow merge into one call
// key = this
void NInteger::addClauses(FullVariableName &key, SatStaticAnalyzer &context) {
    context.addClauses(key, *this);
}

// key = this
void NBinaryOperator::addClauses(FullVariableName &key, SatStaticAnalyzer &context) {
    context.addClauses(key, *this);
}

// key = this
void NIdentifier::addClauses(FullVariableName &key, SatStaticAnalyzer &context) {
    context.addClauses(key, *this);
}

unique_ptr<LanguageType> NIdentifier::mapVariables(const string &functionName, const string &inputName,
                                                   SatStaticAnalyzer &context) {
    return unique_ptr<LanguageType>(new VariableType(name, field));
}

unique_ptr<LanguageType> NInteger::mapVariables(const string &functionName, const string &inputName,
                                                SatStaticAnalyzer &context) {
    return unique_ptr<LanguageType>(new IntegerType(value));
}