#include "analyzer.hpp"
#include "cryptominisat.h"
#include <unordered_set>

class NBlock;
using namespace CMSat;

typedef tuple<string, string, string> FullVariableName;

class SatFunctionDeclaration {
    // std::unordered_set allow fast check that string is an input
    unordered_set<string> inputsMap;
    vector<string> inputs;
    vector<FullVariableName> outputStructs;
    FullVariableName output;
public:
    SatFunctionDeclaration(): inputs(), outputStructs(), output() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputsMap.emplace(input);
        inputs.push_back(input);
    }

    bool isInput(const string & name) const {
        return inputsMap.count(name) > 0;
    }

    void addOutput(const string &functionName, const string &name, const string &field) {
        outputStructs.push_back(make_tuple(functionName, name, field));
    }

    void addTrueOutput(const string &functionName, const string &name, const string &field) {
        output = make_tuple(functionName, name, field);
    }

    const FullVariableName getTrueOutput() const {
        return output;
    }

    const vector<FullVariableName> getOutputs() const {
        return outputStructs;
    }

    const vector<string> getInputs() const {
        return inputs;
    }
};

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;

    std::unique_ptr<SATSolver> solver;

    // relation of function, struct, field (NIdentifier) to Boolean variables
    map<FullVariableName, unsigned int> variables;

    map<string, std::unique_ptr<SatFunctionDeclaration> > functions;

    string currentFunctionName;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier);
    unsigned int addNewVariable(const FullVariableName &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), functions(), currentFunctionName() {}

    void addClauses(const FullVariableName &lhs, const NInteger &nInteger);
    void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    void addClauses(const FullVariableName &lhs, const NIdentifier &nIdentifier);

    void mapMethodCall(const NMethodCall &methodCall, const FullVariableName &output);

    void addInputs(const VariableList &inputs, const NIdentifier &functionName);

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

    void addOutput(const NIdentifier &variableName);

    void addTrueOutput(const NIdentifier &variableName);

    void tryValue(bool value, const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator,
                  unsigned int newVarLast) const;
};