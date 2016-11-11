#include "cryptominisat.h"
#include <unordered_set>
#include <vector>
#include <map>

class NBlock;
using namespace CMSat;

class SatFunctionDeclaration {
    // std::unordered_set allow fast check that string is an input
    unordered_set<string> inputsMap;
    vector<string> inputs;
    vector<FullVariableName> outputStructs;
    FullVariableName output;
    unique_ptr<NBlock> body;
public:
    SatFunctionDeclaration(): inputs(), outputStructs(), output(), body() {}

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

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;

    std::unique_ptr<SATSolver> solver;

    // relation of function, struct, field (NIdentifier) to Boolean variables
    map<FullVariableName, unsigned int> variables;

    map<string, std::unique_ptr<SatFunctionDeclaration> > functions;

    map<string, unique_ptr<LanguageType> > correspondences;

    string currentFunctionName;

    unsigned int getIdentifierVariables(const FullVariableName &nIdentifier);
    unsigned int addNewVariable(const FullVariableName &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), functions(), correspondences(), currentFunctionName() {}

    void addClauses(const FullVariableName &lhs, const NInteger &nInteger);
    void addClauses(const FullVariableName &lhs, const NBinaryOperator &nBinaryOperator);
    void addClauses(const FullVariableName &lhs, const NIdentifier &nIdentifier);

    const string fullNameToSting(const FullVariableName &lhs) {
        if (get<2>(lhs) != "") {
            return get<1>(lhs) + "." + get<2>(lhs);
        } else {
            return get<1>(lhs);
        }
    }

    void mapMethodCall(const NMethodCall &methodCall, const FullVariableName &output);

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

    void addOutput(const NIdentifier &variableName);

    void addTrueOutput(const NIdentifier &variableName);

    bool isConstant(int &returnValue, const FullVariableName &nIdentifier);
};