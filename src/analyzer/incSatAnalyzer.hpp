#include "cryptominisat.h"
#include "node.h"
#include <unordered_set>
#include <list>
#include <cassert>
#include <cmath>
#include <map>
#include <vector>

class NBlock;
using namespace CMSat;
using namespace std;

class FunctionDeclaration {
    // std::unordered_set allow fast check that string is an input
    // TODO(arist): best solution is to use boost::bimap here
    unordered_set<string> inputsMap;
    vector<string> inputs;

    map<string, unordered_set<string> > usageOfInputs;
    map<string, unordered_set<string> > outputOfInputs;
    NIdentifier output;
    unique_ptr<NBlock> body;
public:
    FunctionDeclaration(): inputs(), usageOfInputs(), outputOfInputs(), output("", "", 0), body() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputsMap.emplace(input);
        inputs.push_back(input);
    }

    void addRhsUsage(string &name, string &field) {
        usageOfInputs[name].insert(field);
    }

    unordered_set<string> getUsageOfInputs(string &name) {
        return usageOfInputs[name];
    }

    unordered_set<string> getOutputOfInputs(string &name) {
        return outputOfInputs[name];
    }

    void addLhsUsage(string &name, string &field) {
        outputOfInputs[name].insert(field);
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

    void addTrueOutput(const NIdentifier &output) {
        this->output = output;
    }

    const NIdentifier getTrueOutput() const {
        return output;
    }

    const vector<string> getInputs() const {
        return inputs;
    }

    const string getInput(const unsigned int id) const {
        return inputs[id];
    }
};

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() = default;
};

class IncrementalSatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on int count from parser)
    unsigned int numOfBitsPerInt;

private:

    int doDebug = 0;

    std::unique_ptr<SATSolver> solver;

    // relation of struct, field (NIdentifier) to Boolean variables
    map<FullVariableNameOccurrence, unsigned int> variables;
    map<FullVariableName, unsigned int> variableOccurrences;

    // stack do not support iterators that we need for recurrence search
    list<string> callStack;

    map<string, std::unique_ptr<FunctionDeclaration> > functions;

    string currentFunctionName;

    vector<pair<int, unsigned int> > answers;

    unsigned int addNewSatVariable(FullVariableNameOccurrence &nIdentifier);

    const FullVariableName getFullVariableName(const NIdentifier &lhs);
    FullVariableNameOccurrence getFullVariableNameOccurrence(const NIdentifier &nIdentifier);

    const unsigned int getOccurrences(const FullVariableName &variableName) const {
        unsigned int answer = 0;
        if (variableOccurrences.count(variableName) > 0) {
            answer = variableOccurrences.at(variableName);
        }
        return answer;
    }
    void setOccurrences(const FullVariableName &variableName, unsigned int occurrence) {
        variableOccurrences[variableName] = occurrence;
    }

public:
    unsigned int getSatVariable(FullVariableNameOccurrence &nIdentifier);

    SATSolver *getSolver() {
        return solver.get();
    }

    IncrementalSatStaticAnalyzer() : solver(new SATSolver), variables(), variableOccurrences(), callStack(),
                          functions(), currentFunctionName(), answers() {
        // we need at least one bit for boolean variable
        numOfBitsPerInt = static_cast<unsigned int>( ceil(log2(max(static_cast<unsigned int>( 2), NInteger::differentIntCount))));
    }

    void addInputs(const VariableList &inputs, const NIdentifier &functionName);

    void addBody(NBlock *block, const string &functionName) {
        functions[functionName]->addBody(block);
    }

    virtual void addClauses(const NBlock &root);
    virtual void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    virtual void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    virtual void addClauses(const NIdentifier &lhs, const NIdentifier &rhs);

    void mapMethodCall(const NMethodCall &methodCall, const NIdentifier &output);

    virtual vector<pair<int, unsigned int> > getAnswers() {
        return answers;
    };

    virtual void updateAnswers(const string &opName, FullVariableName &keyLhs, const NIdentifier &lhs);

    const string getCurrentCall() {
        if(callStack.empty()){
            return currentFunctionName;
        }
        return callStack.back();
    }

    const string getTopOfStack() const {
        return currentFunctionName;
    }

    const string getParentCall() {
        if (callStack.size() == 1) {
            return currentFunctionName;
}
        auto rit=callStack.rbegin();
        ++rit;
        if(rit != callStack.rend()) {
            return *rit;
        }  {
            return "";

}}

    FunctionDeclaration *getFunction(const string &name) {
        if (functions.count(name) == 0) {
            throw FunctionIsNotDefined();
        }
        return functions.at(name).get();
    }

    bool isTopLevelCall() const {
        return !callStack.empty();
    }

    void addTrueOutput(const NIdentifier &variableName);

    virtual bool isConstant(int &returnValue, FullVariableNameOccurrence &nIdentifier);

    ~IncrementalSatStaticAnalyzer() override = default;

    unsigned int getRhsSatVariable(FullVariableNameOccurrence &key);

    unsigned int addLhsSatVariable(FullVariableNameOccurrence &key);

    void addClauses(FullVariableName &lhs, FullVariableName &rhs);

    FullVariableNameOccurrence getFullVariableNameOccurrence(FullVariableName &fullVariableName) const;
};