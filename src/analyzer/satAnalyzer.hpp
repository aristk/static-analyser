#include "cryptominisat.h"
#include "node.h"
#include <unordered_set>
#include <vector>
#include <map>

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
    map<string, int> variableOccurences;
public:
    SatFunctionDeclaration(): inputs(), outputStructs(), output("", "", 0), body(), variableOccurences() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputsMap.emplace(input);
        inputs.push_back(input);
    }

    const unsigned int getOccurences(const string &variableName) const {
        unsigned int answer = 0;
        if (variableOccurences.count(variableName) > 0) {
            answer = variableOccurences.at(variableName);
        }
        return answer;
    }

    void setOccurences(const string &variableName, unsigned int occurence) {
        variableOccurences[variableName] = occurence;
    }

    const string makeUniqueName(const string &variableName, const int occurence) {
        return to_string(occurence) + "_" + variableName;
    }

    const string incUniqueName(const string &variableName) {
        unsigned int lhsOccurence = getOccurences(variableName) + 1;
        string lhsName = makeUniqueName(variableName, lhsOccurence);
        setOccurences(variableName, lhsOccurence);
        return lhsName;
    }

    const string getUniqueName(const string &variableName) {
        unsigned int lhsOccurence = getOccurences(variableName);
        string lhsName = makeUniqueName(variableName, lhsOccurence);
        return lhsName;
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

    void addTrueOutput(const NIdentifier &output) {
        this->output = output;
    }

    const NIdentifier getTrueOutput() const {
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

    map<string, NExpression* > correspondences;

    string currentFunctionName;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier);
    unsigned int addNewVariable(const NIdentifier &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), functions(), correspondences(), currentFunctionName() {}

    virtual void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    virtual void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    virtual void addClauses(const NIdentifier &lhs, const NIdentifier &rhs);

    const FullVariableName NIdentifierToFullName(const NIdentifier &lhs) {
        return make_tuple(currentFunctionName, lhs.name, lhs.field);
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

};