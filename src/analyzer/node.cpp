#include "parser/node.h"
#include "satAnalyzer.hpp"

// TODO: rename to satNode.cpp (since it is sat related)

void NBlock::genCheck(SatStaticAnalyzer& context) const {
    for(auto i : statements) {
        // TODO: add to the parser check that only NFunctionDeclaration could be here
        i->genCheck(context);
    }
}

void NFunctionDeclaration::genCheck(SatStaticAnalyzer &context) const {
    // TODO: process inputs
    // TODO: checks that Variable assignments and structures are not allowed should be part of the parser

    // process body
    block.genCheck(context);
}

void NVariableDeclaration::genCheck(SatStaticAnalyzer &context) const {
    assignmentExpr->addClauses(id, context);
}

void NInteger::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}

void NBinaryOperator::addClauses(NIdentifier &nIdentifier, SatStaticAnalyzer &context) {
    context.addClauses(nIdentifier, *this);
}