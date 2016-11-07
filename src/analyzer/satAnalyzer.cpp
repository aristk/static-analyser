#include "parser/node.h"
#include "satAnalyzer.hpp"

void SatStaticAnalyzer::generateCheck(NBlock &root) {
    root.genCheck(*this);
}

void SatStaticAnalyzer::addClauses(NIdentifier &nIdentifier, NInteger &nInteger) {
    int nVars = solver->nVars();
    variables[make_pair(nIdentifier.name, nIdentifier.field)] = nVars;
    solver->new_vars(numOfBitsPerInt);
    vector<unsigned int> clause(1);

    int value = nInteger.value;

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = nVars + i;
        solver->add_xor_clause(clause, value & 1);
        value >>= 1;
    }
}

