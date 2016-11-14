#include "cryptominisat.h"
#include "node.h"
#include <unordered_set>
#include <vector>
#include <map>
#include <stack>

class NBlock;
using namespace CMSat;
using namespace std;

class SatFunctionDeclaration {
    // std::unordered_set allow fast check that string is an input
    unordered_set<string> inputsMap;
    vector<string> inputs;
    vector<FullVariableName> outputStructs;
    NIdentifier output;
    unique_ptr<NBlock> body;
public:
    SatFunctionDeclaration(): inputs(), outputStructs(), output("", "", 0), body() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputsMap.emplace(input);
        inputs.push_back(input);
    }

    void addBody(NBlock *FunctionBody) {
        body = move(unique_ptr<NBlock>(FunctionBody));
    }

    NBlock *getBody() {
        return body.get();
    }

    bool isInput(const string & name) const {
        return inputsMap.count(name) > 0;
    }

    void addOutput(const string &functionName, const string &name, const string &field) {
        outputStructs.push_back(FullVariableName(functionName, name, field));
    }

    void addTrueOutput(const NIdentifier &output) {
        this->output = output;
    }

    const NIdentifier getTrueOutput() const {
        return output;
    }

    const vector<string> getInputs() const {
        return inputs;
    }
};

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;

    std::unique_ptr<SATSolver> solver;

    // relation of function, struct, field (NIdentifier) to Boolean variables
    map<FullVariableNameOccurrence, unsigned int> variables;
    map<FullVariableName, unsigned int> variableOccurrences;

    stack<string> callStack;

    map<string, std::unique_ptr<SatFunctionDeclaration> > functions;

    map<string, NExpression* > correspondences;

    string currentFunctionName;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier);
    unsigned int addNewVariable(const NIdentifier &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), variableOccurrences(), callStack(), functions(), correspondences(), currentFunctionName() {}

    virtual void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    virtual void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    virtual void addClauses(const NIdentifier &lhs, const NIdentifier &rhs);

    const unsigned int getOccurrences(const FullVariableName &variableName) const {
        unsigned int answer = 0;
        if (variableOccurrences.count(variableName) > 0) {
            answer = variableOccurrences.at(variableName);
        }
        return answer;
    }

    void setOccurrences(const FullVariableName &variableName, unsigned int occurence) {
        variableOccurrences[variableName] = occurence;
    }

    const FullVariableName NIdentifierToFullName(const NIdentifier &lhs) {
        return FullVariableName(currentFunctionName, lhs.name, lhs.field);
    }

    void mapMethodCall(const NMethodCall &methodCall, const NIdentifier &output);

    void addInputs(const VariableList &inputs, const NIdentifier &functionName);

    void addBody(NBlock *block, const string &functionName) {
        functions[functionName]->addBody(block);
    }

    void generateCheck(const NBlock& root);

    bool isCurrentInput(const NIdentifier &nIdentifier);

    const string getCurrentFunctionName() const {
        return currentFunctionName;
    }

    SATSolver *getSolver() const {
        return solver.get();
    }

    SatFunctionDeclaration *getFunction(const string &name) {
        if (functions.count(name) == 0) {
            throw FunctionIsNotDefined();
        }
        return functions.at(name).get();
    }

    NExpression * mapToInput(const NIdentifier &nIdentifier);

    void addOutput(const NIdentifier &variableName);

    void addTrueOutput(const NIdentifier &variableName);

    virtual bool isConstant(int &returnValue, const NIdentifier &nIdentifier);

    virtual ~SatStaticAnalyzer() {}

    FullVariableNameOccurrence getFullVariableNameOccurrence(const NIdentifier &nIdentifier);
};