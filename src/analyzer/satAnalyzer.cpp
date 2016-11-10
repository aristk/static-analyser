#include "analyzer/node.h"
#include "satAnalyzer.hpp"

void SatStaticAnalyzer::generateCheck(const NBlock &root) {
    for(auto i : root.statements) {
        if (!(i->isFunctionDeclaration())) {
            // TODO: add to the parser check that only NFunctionDeclaration could be here
            throw notNFunctionDeclaration();
        }
        i->genCheck(*this);
    }
}

unsigned int SatStaticAnalyzer::addNewVariable(const FullVariableName &key) {
    unsigned int nVars = solver->nVars();

    // in case of dummy variables, do not map them
    if (get<1>(key) != "") {
        variables[key] = nVars;
    }
    solver->new_vars(numOfBitsPerInt);
    return nVars;
}

unsigned int
SatStaticAnalyzer::getIdentifierVariables(const NIdentifier &nIdentifier) {
    FullVariableName key = make_tuple(currentFunctionName, nIdentifier.name, nIdentifier.field);

    // if variables was not defined, define them
    if (variables.count(key) == 0) {
        addNewVariable(key);
    }
    return variables[key];
}

void SatStaticAnalyzer::addClauses(const FullVariableName &lhs, const NIdentifier &nIdentifier) {
    const int variableCount = 2;
    vector<unsigned int> nVars(variableCount);
    nVars[0] = addNewVariable(lhs);
    nVars[1] = getIdentifierVariables(nIdentifier);
    vector<unsigned int> clause(variableCount);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }
        solver->add_xor_clause(clause, false);
    }
}

void SatStaticAnalyzer::addClauses(const FullVariableName &key, const NInteger &nInteger) {
    unsigned int nVars = addNewVariable(key);
    vector<Lit> clause(1);

    int value = nInteger.value;

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = Lit(nVars + i, (bool) (true - (value & 0b1)));
        solver->add_clause(clause);
        value >>= 1;
    }
}

// TODO: required automatic tests
void SatStaticAnalyzer::addClauses(const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator) {
    const int variableCount = 3;
    vector<unsigned int> nVars(variableCount+1);

    // create dummy variables for computations
    nVars[0] = addNewVariable("");
    nVars[1] = getIdentifierVariables(nBinaryOperator.lhs);
    nVars[2] = getIdentifierVariables(nBinaryOperator.rhs);

    // add new variables to handle output of NBinaryOperator
    unsigned int newVarLast = solver->nVars();
    FullVariableName key = make_tuple(getCurrentFunctionName(), nIdentifier.name, nIdentifier.field);
    nVars[3] = addNewVariable(key);

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
    bool value = nBinaryOperator.op;
    for(int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }

        // newVarI = lhsI xor rhsI
        solver->add_xor_clause(clause, false);

        // newVarLast | -newVarI
        binClause[0] = Lit(clause[0], true);
        binClause[1] = Lit(newVarLast, value);
        solver->add_clause(binClause);

        // (-newVar0 | ... | -newVarN | newVarLast)
        bigClause[i] = Lit(clause[0], false);
    }
    bigClause[numOfBitsPerInt] = Lit(newVarLast, !value);
    solver->add_clause(bigClause);

    // force upper bits of representation to zeros
    vector<Lit> UnitClause(1);
    for (int i = 1; i < numOfBitsPerInt; i++) {
        UnitClause[0] = Lit(newVarLast + i, true);
        solver->add_clause(UnitClause);
    }

    // TODO: good option here is to introduce a class with undef and int values of return
    int returnValue;
    if (isConstant(returnValue, nIdentifier, newVarLast)) {
        cout << nIdentifier.printName() << " has constant value " << returnValue <<
             " at line " << nBinaryOperator.lineno << endl;
    }
}

bool
SatStaticAnalyzer::isConstant(int &returnValue, const NIdentifier &nIdentifier, unsigned int newVarLast) const {
    bool result = false;

    solver->solve();

    // TODO: assume that solver found all unit clauses (should be checked)
    vector<Lit> unitLiterals = solver->get_zero_assigned_lits();

    for(auto i: unitLiterals) {
        if ((i.var() >= newVarLast) && (i.var() < newVarLast + numOfBitsPerInt)) {
            int bitPosition = i.var() - newVarLast;
            returnValue += (1 << bitPosition) * (1-i.sign());
            // TODO: in general case we should check that all bits are constant
            result = true;
        }
    }

    return result;
}

void SatStaticAnalyzer::addInputs(const VariableList &inputs, const NIdentifier &functionName) {
    unique_ptr<SatFunctionDeclaration> Function(new SatFunctionDeclaration);

    vector<string> inputsNames(inputs.size());
    for(auto i : inputs) {
        // TODO: checks that structures are not allowed should be part of the parser
        if (i->field != "")  {
            throw InputIsAStruct();
        }
        Function->addInput(i->printName());
    }

    string name = functionName.printName();

    functions.emplace(name, move(Function));
    currentFunctionName = name;
}

// TODO: join into one function following two
void SatStaticAnalyzer::addOutput(const NIdentifier &variableName) {
    SatFunctionDeclaration *currentFunction = getFunction(currentFunctionName);

    currentFunction->addOutput(currentFunctionName, variableName.name, variableName.field);
}

void SatStaticAnalyzer::addTrueOutput(const NIdentifier &variableName){
    SatFunctionDeclaration *currentFunction = getFunction(currentFunctionName);

    currentFunction->addTrueOutput(currentFunctionName, variableName.name, variableName.field);
}

bool SatStaticAnalyzer::isCurrentInput(const NIdentifier &nIdentifier) {
    SatFunctionDeclaration *currentFunction = getFunction(currentFunctionName);

    if (currentFunction->isInput(nIdentifier.name)) {
        return true;
    }
    return false;
}

void SatStaticAnalyzer::mapMethodCall(const NMethodCall &methodCall, const FullVariableName &output) {

    string calledFunctionName = methodCall.id.name;
    SatFunctionDeclaration *calledFunction = getFunction(calledFunctionName);

    vector<string> originalInputs = calledFunction->getInputs();
    map<string, string> correspondences;

    // map inputs
    for(int i = 0; i < methodCall.arguments.size(); i++) {
        pair<string, string> correspondence =
                methodCall.arguments[i]->mapVariables(calledFunctionName, originalInputs[i], *this);
        correspondences.emplace(correspondence);
    }

    // map outputs
    for (auto i: calledFunction->getOutputs()) {
        const string currentIdentifierName = correspondences[get<1>(i)];
        if (currentIdentifierName == "") {
            throw InputIsAStruct();
        }
        NIdentifier nIdentifier(currentIdentifierName, get<2>(i));
        addClauses(i, nIdentifier);
    }

    // map true output
    if (get<0>(output) != "") {
        NIdentifier nIdentifier(get<1>(output), get<2>(output));
        addClauses(calledFunction->getTrueOutput(), nIdentifier);
    }
}

