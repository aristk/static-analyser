#include "parser/node.h"
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
    assignmentExpr->addClauses(id, context);
}

void NReturnStatement::genCheck(SatStaticAnalyzer &context) const {
    context.addReturn(this->variable);
}

//TODO: somehow merge into one call
void NInteger::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}

void NBinaryOperator::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}

void NIdentifier::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}