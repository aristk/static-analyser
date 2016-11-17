#include <assert.h>
#include "analyzer/node.h"
#include "satAnalyzer.hpp"

void SatStaticAnalyzer::addClauses(const NBlock &root) {
    for(auto i : root.statements) {
        if (!(i->isFunctionDeclaration())) {
            // TODO: add to the parser check that only NFunctionDeclaration could be here
            throw notNFunctionDeclaration();
        }
        i->genCheck(*this);
    }
}

unsigned int SatStaticAnalyzer::addNewSatVariable(const NIdentifier &nIdentifier) {

    unsigned int nVars = solver->nVars();

    FullVariableNameOccurrence key = getFullVariableNameOccurrence(nIdentifier);

    // in case of dummy variables, do not map them
    if (key.first.first != "") {
        if (variables.count(key) > 0) {
            // if variable that going to be assigned was used previously, then we need to create new 
            // variable (previous incarnations of variable could be used, so we should not erase them)
            key.second++;
            FullVariableName fullVariableName = getFullVariableName(nIdentifier);
            unsigned int occurrence = getOccurrences(fullVariableName);
            setOccurrences(fullVariableName, occurrence+1);
        }

        variables[key] = nVars;
    }
    solver->new_vars(numOfBitsPerInt);
    return nVars;
}

unsigned int
SatStaticAnalyzer::getSatVariable(const NIdentifier &nIdentifier) {

    FullVariableNameOccurrence key = getFullVariableNameOccurrence(nIdentifier);

    // if variables was not defined, define them
    if (variables.count(key) == 0) {
        addNewSatVariable(nIdentifier);
    }

    return variables.at(key);
}

const FullVariableName SatStaticAnalyzer::getFullVariableName(const NIdentifier &lhs) {
    string variableName = lhs.name;

    if(!callStack.empty()) {
        string calledFunctionName = this->getCurrentCall();
        FunctionDeclaration *currentFunction = getFunction(calledFunctionName);

        string newVariableName = currentFunction->getCallArgument(variableName);
        if (newVariableName != "") {
            variableName = newVariableName;
        }
    }

    return FullVariableName(variableName, lhs.field);
}

FullVariableNameOccurrence SatStaticAnalyzer::getFullVariableNameOccurrence(const NIdentifier &nIdentifier) {

    FullVariableName fullVariableName = getFullVariableName(nIdentifier);

    unsigned int occurrence = getOccurrences(fullVariableName);

    FullVariableNameOccurrence key =
            make_pair(fullVariableName, occurrence);

    return key;
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NIdentifier &rhs) {

    const int variableCount = 2;
    vector<unsigned int> nVars(variableCount);
    // it is important to first get old variable and then introduce a new variable
    // Usecase x = x
    nVars[1] = getSatVariable(rhs);
    nVars[0] = addNewSatVariable(lhs);

    vector<unsigned int> clause(variableCount);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }
        solver->add_xor_clause(clause, false);
    }

    if (doDebug == 1) {
        cout << getFullVariableNameOccurrence(lhs);
        cout << " = " << getFullVariableNameOccurrence(rhs);
        cout << endl;
    }
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NInteger &nInteger) {

    int value = NInteger::intMapping[nInteger.value];

    unsigned int nVars = addNewSatVariable(lhs);

    vector<Lit> clause(1);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = Lit(nVars + i, (bool) (true - (value & 0b1)));
        solver->add_clause(clause);
        value >>= 1;
    }

    if (doDebug == 1) {
        cout << getFullVariableNameOccurrence(lhs);
        cout << " = " << nInteger.value;
        cout << endl;
    }
}

// TODO: need tests
void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator) {

    // TODO: check that operands of BinaryOperator could not be integers or structs
    const int variableCount = 3;
    vector<unsigned int> nVars(variableCount+1);

    // create dummy variables for computations
    nVars[0] = addNewSatVariable(NIdentifier("", "", 0));
    nVars[1] = getSatVariable(nBinaryOperator.lhs);
    nVars[2] = getSatVariable(nBinaryOperator.rhs);

    // add new variables to handle output of NBinaryOperator
    unsigned int newVarLast = solver->nVars();
    nVars[3] = addNewSatVariable(lhs);

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

    if (doDebug == 1) {
        cout << getFullVariableNameOccurrence(lhs);
        cout << " = " << getFullVariableNameOccurrence(nBinaryOperator.lhs);
        cout << " == " << getFullVariableNameOccurrence(nBinaryOperator.rhs);
        cout << endl;
    }
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

    const unsigned int lowerBitVariable = getSatVariable(key);

    lbool result = solver->solve();

    assert(result == l_True);

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
    unique_ptr<FunctionDeclaration> Function(new FunctionDeclaration);

    for(auto i : inputs) {
        // TODO: checks that structures are not allowed should be part of the parser
        if (i->field != "")  {
            throw InputIsAStruct();
        }
        Function->addInput(i->printName());
    }

    string name = functionName.printName();

    currentFunctionName = name;
    if(doDebug == 1) {
        cout << "func " << name << "(";
        for(int i = 0; i < inputs.size(); i++) {
            cout << Function->getInput(i) << ", ";
        }
        cout << ")" << endl;
    }
    functions.emplace(name, move(Function));
}

void SatStaticAnalyzer::addTrueOutput(const NIdentifier &variableName){
    FunctionDeclaration *currentFunction = getFunction(currentFunctionName);

    currentFunction->addTrueOutput(variableName);
}


void SatStaticAnalyzer::mapMethodCall(const NMethodCall &methodCall, const NIdentifier &output) {

    string calledFunctionName = methodCall.id.name;

    // disallow recursive calls
    list<string>::iterator stackIter = find(callStack.begin(), callStack.end(), calledFunctionName);
    if (stackIter != callStack.end()) {
        throw recursiveCall(calledFunctionName);
    }

    callStack.push_back(calledFunctionName);
    FunctionDeclaration *calledFunction = getFunction(calledFunctionName);

    vector<string> originalInputs = calledFunction->getInputs();

    if (methodCall.arguments.size() != originalInputs.size()) {
        throw differentNumberOfArgsInFunctionCall();
    }

    // map inputs
    // TODO: check that integer input is not used as struct later
    // TODO: if input struct is used in call function, it should be added to inputs
    for(int i = 0; i < methodCall.arguments.size(); i++) {
        methodCall.arguments[i]->processCallInput(i, *this);
    }

    // TODO: important open question: is how to handle multiple function calls without inlining?

    // inline function body
    calledFunction->getBody()->genCheck(*this);

    // return back to parent function
    calledFunction->clearCallInputMap();
    callStack.pop_back();

    // map and check true output
    if (output.name != "") {
        // map function output and new variable
        addClauses(output, calledFunction->getTrueOutput());

        updateAnswers("func \"" + calledFunctionName + "\"", output);
    }
}


