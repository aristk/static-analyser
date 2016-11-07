#include "parser/node.h"
#include "satAnalyzer.hpp"

void SatStaticAnalyzer::generateCheck(const NBlock &root) {
    root.genCheck(*this);
}


int SatStaticAnalyzer::addNewVariable(const NIdentifier &nIdentifier) {
    int nVars = solver->nVars();
    variables[make_pair(nIdentifier.name, nIdentifier.field)] = nVars;
    solver->new_vars(numOfBitsPerInt);
    return nVars;
}

void SatStaticAnalyzer::addClauses(const NIdentifier &nIdentifier, const NInteger &nInteger) {
    int nVars = addNewVariable(nIdentifier);
    vector<Lit> clause(1);

    int value = nInteger.value;

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = Lit(nVars + i, 1-(value & 0b1));
        solver->add_clause(clause);
        value >>= 1;
    }
}

// TODO: required automatic tests
void SatStaticAnalyzer::addClauses(const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator) {
    const int variableCount = 3;
    vector<unsigned int> nVars(variableCount);
    nVars[0] = addNewVariable(nIdentifier);
    // add new variable to handle output of NBinaryOperator
    int newVarLast = solver->nVars();
    solver->new_var();
    nVars[1] = getIdentifierVariables(nBinaryOperator.lhs);
    nVars[2] = getIdentifierVariables(nBinaryOperator.rhs);

    vector<unsigned int> clause(variableCount);
    vector<Lit> binClause(2);
    vector<Lit> bigClause(numOfBitsPerInt+1);

    // it is a circuit with xor gates on the top:
    // newVarI = lhsI xor rhsI
    // and &-gate at the bottom:
    // newVarLast = newVar0 & ... & newVarN
    // &-gate is modeled as
    // (newVar0 | -newVarLast) & ... & (newVarN | -newVarLast) &
    // (-newVar0 | ... | -newVarN | newVarLast)
    for(int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }

        // newVarI = lhsI xor rhsI
        solver->add_xor_clause(clause, false);

        // newVarLast | -newVarI
        binClause[0] = Lit(clause[0], true);
        binClause[1] = Lit(newVarLast, false);
        solver->add_clause(binClause);

        // (-newVar0 | ... | -newVarN | newVarLast)
        bigClause[i] = Lit(clause[0], false);
    }
    bigClause[numOfBitsPerInt] = Lit(newVarLast, true);
    solver->add_clause(bigClause);

    vector<Lit> assumptions(1);

    assumptions[0] = Lit(newVarLast, false);

    lbool ret = solver->solve(&assumptions);

    if (ret == l_False) {
        string name = nIdentifier.name;
        if (nIdentifier.field != "") {
            name = nIdentifier.name + "." + nIdentifier.field;
        }
        cout << name << " has constant value " << nBinaryOperator.op << endl;
    }

    assumptions[0] = Lit(newVarLast, true);

    ret = solver->solve(&assumptions);

    if (ret == l_False) {
        string name = nIdentifier.name;
        if (nIdentifier.field != "") {
            name = nIdentifier.name + "." + nIdentifier.field;
        }
        cout << name << " has constant value " << (0b1 ^ nBinaryOperator.op) << endl;
    }
}

int
SatStaticAnalyzer::getIdentifierVariables(const NIdentifier &nIdentifier) const {
    pair<string, string> key = make_pair(nIdentifier.name, nIdentifier.field);
    // TODO: best way is to hide with assert or NDEBUG
    if (variables.count(key) == 0 ) {
        cerr << nIdentifier.name << "." << nIdentifier.field << endl;
        throw SatVariableIsNotDefinened();
    }

    return variables.at(key);
}

