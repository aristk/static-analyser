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

unsigned int SatStaticAnalyzer::addNewSatVariable(FullVariableNameOccurrence &key) {

    // number of used variables
    unsigned int nVars = solver->nVars();

    // in case of dummy variables, do not map them
    if (getVariableName(key) != "") {
        if (variables.count(key) > 0) {
            // if variable that going to be assigned was used previously, then we need to create new 
            // variable (previous incarnations of variable could be used, so we should not erase them)
            key.second++;
            FullVariableName fullVariableName = key.first;
            unsigned int occurrence = getOccurrences(fullVariableName);
            setOccurrences(fullVariableName, occurrence+1);
        }

        variables[key] = nVars;
    }
    solver->new_vars(numOfBitsPerInt);
    return nVars;
}

unsigned int
SatStaticAnalyzer::getSatVariable(FullVariableNameOccurrence &key) {

    // if variables was not defined, define them
    if (variables.count(key) == 0) {
        return addNewSatVariable(key);
    }

    return variables.at(key);
}

unsigned int
SatStaticAnalyzer::getRhsSatVariable(FullVariableNameOccurrence &key) {
    if(callStack.empty()) {
        FunctionDeclaration *currentFunction = getFunction(currentFunctionName);
        string name = getVariableName(key);
        if (currentFunction->isInput(name)) {
            string field = getVariableField(key);
            currentFunction->addRhsUsage(name, field);
        }
    }

    return getSatVariable(key);
}

unsigned int
SatStaticAnalyzer::addLhsSatVariable(FullVariableNameOccurrence &key) {
    if(callStack.empty()) {
        FunctionDeclaration *currentFunction = getFunction(currentFunctionName);
        string name = getVariableName(key);
        if (currentFunction->isInput(name)) {
            string field = getVariableField(key);
            currentFunction->addLhsUsage(name, field);
        }
    }

    return addNewSatVariable(key);
}

const FullVariableName SatStaticAnalyzer::getFullVariableName(const NIdentifier &lhs) {
    string variableName = lhs.name;

    string callName = currentFunctionName;
    // side effect is only for fields
    if(!callStack.empty()) {
        string calledFunctionName = this->getCurrentCall();
        if (true || lhs.field != "") {
            FunctionDeclaration *currentFunction = getFunction(calledFunctionName);

            string newVariableName = currentFunction->getCallArgument(variableName);
            if (newVariableName != "") {
                variableName = newVariableName;
            }
        } else {
            callName = calledFunctionName;
        }
    }

    return FullVariableName(callName, variableName, lhs.field);
}

FullVariableNameOccurrence SatStaticAnalyzer::getFullVariableNameOccurrence(const NIdentifier &nIdentifier) {

    FullVariableName fullVariableName = getFullVariableName(nIdentifier);

    return getFullVariableNameOccurrence(fullVariableName);

}

FullVariableNameOccurrence SatStaticAnalyzer::getFullVariableNameOccurrence(FullVariableName &fullVariableName) const {
    unsigned int occurrence = getOccurrences(fullVariableName);

    FullVariableNameOccurrence key = make_pair(fullVariableName, occurrence);

    return key;
}

void SatStaticAnalyzer::addClauses(FullVariableName &lhs, FullVariableName &rhs) {
    const int variableCount = 2;
    vector<unsigned int> nVars(variableCount);
    // it is important to first get old variable and then introduce a new variable
    // Use case x = x
    FullVariableNameOccurrence keyRhs = getFullVariableNameOccurrence(rhs);
    nVars[1] = getRhsSatVariable(keyRhs);

    FullVariableNameOccurrence keyLhs = getFullVariableNameOccurrence(lhs);
    nVars[0] = addLhsSatVariable(keyLhs);

    vector<unsigned int> clause(variableCount);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        for (int j = 0; j < variableCount; j++) {
            clause[j] = nVars[j] + i;
        }
        solver->add_xor_clause(clause, false);
    }

    if (doDebug == 1) {
        cout << "\t" << keyLhs;
        cout << " = " << keyRhs;
        cout << endl;
    }
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NIdentifier &rhs) {

    FullVariableName fullLhs = getFullVariableName(lhs);
    FullVariableName fullRhs = getFullVariableName(rhs);
    addClauses(fullLhs, fullRhs);
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NInteger &nInteger) {

    int value = NInteger::intMapping[nInteger.value];

    FullVariableNameOccurrence keyLhs = getFullVariableNameOccurrence(lhs);
    unsigned int nVars = addNewSatVariable(keyLhs);

    vector<Lit> clause(1);

    for (int i = 0; i < numOfBitsPerInt; i++) {
        clause[0] = Lit(nVars + i, (bool) (true - (value & 0b1)));
        solver->add_clause(clause);
        value >>= 1;
    }

    if (doDebug == 1) {
        cout << "\t" << keyLhs;
        cout << " = " << nInteger.value;
        cout << endl;
    }
}

void SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator) {
    // operands of binary operation could not be integers (checked by parser)
    const int variableCount = 3;
    vector<unsigned int> nVars(variableCount+1);

    // create dummy variables for computations
    FullVariableNameOccurrence empty(FullVariableName("", "", ""), 0);
    nVars[0] = addNewSatVariable(empty);
    FullVariableNameOccurrence keyBinLhs = getFullVariableNameOccurrence(nBinaryOperator.lhs);
    nVars[1] = getRhsSatVariable(keyBinLhs);
    FullVariableNameOccurrence keyBinRhs = getFullVariableNameOccurrence(nBinaryOperator.rhs);
    nVars[2] = getRhsSatVariable(keyBinRhs);

    // add new variables to handle output of NBinaryOperator
    unsigned int newVarLast = solver->nVars();
    FullVariableNameOccurrence keyLhs = getFullVariableNameOccurrence(lhs);
    nVars[3] = addLhsSatVariable(keyLhs);

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
        cout << "\t" << keyLhs;
        cout << " = " << keyBinLhs;
        cout << " == " << keyBinRhs;
        cout << endl;
    }
}

void SatStaticAnalyzer::updateAnswers(const string &opName, const NIdentifier &lhs) {
    // TODO: good option here is to introduce a class with undef and int values of return
    int returnValue;
    FullVariableNameOccurrence key = getFullVariableNameOccurrence(lhs);
    if (isConstant(returnValue, key)) {
        cout << lhs.printName() << " from " << opName << " is " << returnValue <<
             " at line " << lhs.lineNumber << endl;
        answers.push_back(make_pair(returnValue, lhs.lineNumber));
    }
}

bool
SatStaticAnalyzer::isConstant(int &returnValue, FullVariableNameOccurrence &key) {

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

    string name = functionName.printName();

    if (functions.count(name) != 0) {
        throw FunctionDefinedTwice(name);
    }

    unique_ptr<FunctionDeclaration> Function(new FunctionDeclaration);

    for(auto i : inputs) {
        // TODO: checks that structures are not allowed should be part of the parser
        if (i->field != "")  {
            throw InputIsAStruct();
        }
        Function->addInput(i->printName());
    }

    currentFunctionName = name;
    if(doDebug == 1) {
        cout << "func " << name << "(";
        vector<string> v = Function->getInputs();
        bool first = true;
        for (auto i: v) {
            if (first) {
                first = false;
            } else {
                cout << ", ";
            }
            cout << i;
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
    for(unsigned int i = 0; i < methodCall.arguments.size(); i++) {
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
        NIdentifier trueOutput = calledFunction->getTrueOutput();
        if (trueOutput.name == "") {
            cerr << "Warning: function " << calledFunctionName << " do not return any thing." << endl;
        }
        addClauses(output, trueOutput);

        updateAnswers("func \"" + calledFunctionName + "\"", output);
    }
}


