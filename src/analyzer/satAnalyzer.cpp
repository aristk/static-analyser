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

unsigned int SatStaticAnalyzer::addNewVariable(const NIdentifier &nIdentifier) {

    unsigned int nVars = solver->nVars();

    FullVariableNameOccurrence key = getFullVariableNameOccurrence(nIdentifier);

    // in case of dummy variables, do not map them
    if (get<1>(key) != "") {
        if (variables.count(key) > 0) {
            // if variable that going to be assigned was used previously, then we need to create new 
            // variable (previous incarnations of variable could be used, so we should not erase them)
            get<3>(key)++;
            FullVariableName fullVariableName = NIdentifierToFullName(nIdentifier);
            unsigned int occurrence = getOccurrences(fullVariableName);
            setOccurrences(fullVariableName, occurrence+1);
        }
        variables[key] = nVars;
    }
    solver->new_vars(numOfBitsPerInt);
    return nVars;
}

FullVariableNameOccurrence SatStaticAnalyzer::getFullVariableNameOccurrence(const NIdentifier &nIdentifier) {
    FullVariableName fullVariableName = NIdentifierToFullName(nIdentifier);

    unsigned int occurrence = getOccurrences(fullVariableName);

    FullVariableNameOccurrence key =
            make_tuple(get<0>(fullVariableName), get<1>(fullVariableName), get<2>(fullVariableName), occurrence);
    return key;
}

unsigned int
SatStaticAnalyzer::getIdentifierVariables(const NIdentifier &nIdentifier) {
    FullVariableNameOccurrence key = getFullVariableNameOccurrence(nIdentifier);
    // if variables was not defined, define them
    if (variables.count(key) == 0) {
        addNewVariable(nIdentifier);
    }
    return variables.at(key);
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NIdentifier &rhs) {

    const int variableCount = 2;
    vector<unsigned int> nVars(variableCount);
    nVars[0] = addNewVariable(lhs);
    nVars[1] = getIdentifierVariables(rhs);
    vector<unsigned int> clause(variableCount);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }
        solver->add_xor_clause(clause, false);
    }
}

void SatStaticAnalyzer::addClauses(const NIdentifier &key, const NInteger &nInteger) {
    int value = nInteger.value;

    unsigned int nVars = addNewVariable(key);
    vector<Lit> clause(1);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = Lit(nVars + i, (bool) (true - (value & 0b1)));
        solver->add_clause(clause);
        value >>= 1;
    }
}

// TODO: required automatic tests
void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator) {
    const int variableCount = 3;
    vector<unsigned int> nVars(variableCount+1);

    // create dummy variables for computations
    nVars[0] = addNewVariable(NIdentifier("","",0));
    nVars[1] = getIdentifierVariables(nBinaryOperator.lhs);
    nVars[2] = getIdentifierVariables(nBinaryOperator.rhs);

    // add new variables to handle output of NBinaryOperator
    unsigned int newVarLast = solver->nVars();
    nVars[3] = addNewVariable(lhs);

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

    updateAnswers("binOp", lhs);
}

void SatStaticAnalyzer::updateAnswers(const string &opName, const NIdentifier &lhs) {
    // TODO: good option here is to introduce a class with undef and int values of return
    int returnValue;
    if (isConstant(returnValue, lhs)) {
        cout << lhs.printName() << " from " << opName << " is " << returnValue <<
             " at line " << lhs.lineNumber << endl;
        answers.push_back(make_pair(returnValue, lhs.lineNumber));
    }
}

bool
SatStaticAnalyzer::isConstant(int &returnValue, const NIdentifier &key) {

    // do not check for inlined functions
    if (!callStack.empty())
        return false;

    const unsigned int lowerBitVariable = getIdentifierVariables(key);

    solver->solve();

    int countEqualBits = 0;
    returnValue = 0;

    // small optimization: do not call solver for literals with values
    vector<bool> skip(numOfBitsPerInt, 0);
    vector<Lit> unitLiterals = solver->get_zero_assigned_lits();

    for(auto i: unitLiterals) {
        if ((i.var() >= lowerBitVariable) && (i.var() < lowerBitVariable + numOfBitsPerInt)) {
            int bitPosition = i.var() - lowerBitVariable;
            returnValue += (1 << bitPosition) * (1-i.sign());
            countEqualBits++;
            skip[bitPosition] = 1;
        }
    }

    // try not yet assigned values
    vector<lbool> model = solver->get_model();
    for(int i=0; i < numOfBitsPerInt; i++) {
        if(!skip[i]) {
            vector<Lit> assumption(1);
            bool sign = model[lowerBitVariable + i] == l_True;
            assumption[0] = Lit(lowerBitVariable + i, sign);
            lbool ret = solver->solve(&assumption);
            if (ret == l_False) {
                returnValue += (1 << i) * (sign);
                countEqualBits++;
            }
        }
    }

    return countEqualBits == numOfBitsPerInt;

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

    currentFunction->addTrueOutput(variableName);
}

bool SatStaticAnalyzer::isCurrentInput(const NIdentifier &nIdentifier) {
    SatFunctionDeclaration *currentFunction = getFunction(currentFunctionName);

    return currentFunction->isInput(nIdentifier.name);
}

void SatStaticAnalyzer::mapMethodCall(const NMethodCall &methodCall, const NIdentifier &output) {

    string calledFunctionName = methodCall.id.name;
    callStack.push(calledFunctionName);
    SatFunctionDeclaration *calledFunction = getFunction(calledFunctionName);

    vector<string> originalInputs = calledFunction->getInputs();

    // map inputs
    // TODO: should not be used
    for(int i = 0; i < methodCall.arguments.size(); i++) {
        correspondences.emplace(originalInputs[i], methodCall.arguments[i]);
        // TODO: check that number of inputs is the same
    }

    // since we cannot remove variables and clauses, we need to store transitions separately (as new lhs variables)

    // TODO: important open question: is how to handle multiple function calls without inlining?

    // distinguish normal function and called function with integer at beginning
    // inline function body
    calledFunction->getBody()->genCheck(*this);

    // TODO: could be buggy here: it is more clear way to implement something like "call stack" with correspondences
    correspondences.clear();

    // cout << "checking function " <<  calledFunctionName << "()" << endl;

    callStack.pop();

    // map and check true output
    if (output.name != "") {
        // map function output and new variable
        addClauses(output, calledFunction->getTrueOutput());

        updateAnswers("func \"" + calledFunctionName + "\"", output);
    }
}

NExpression * SatStaticAnalyzer::mapToInput(const NIdentifier &nIdentifier) {
    if (!correspondences.empty() && (correspondences.count(nIdentifier.name) > 0)) {
        return correspondences[nIdentifier.name];
    }
    return nullptr;
}

