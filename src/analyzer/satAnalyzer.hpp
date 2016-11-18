#include "cryptominisat.h"
#include "node.h"
#include <unordered_set>
#include <vector>
#include <map>
#include <list>
#include <math.h>
#include <cassert>

class NBlock;
using namespace CMSat;
using namespace std;

class FunctionDeclaration {
    // std::unordered_set allow fast check that string is an input
    // TODO: best solution is to use boost::bimap here
    unordered_set<string> inputsMap;
    vector<string> inputs;
    // map to keep correspondence between original arguments and call arguments for later substitution
    // since recursive calls are not allowed at all, it is save to store just one copy of mapping between function
    // invocation inputs and function declaration inputs
    map<string, string> callInputMap;

    map<string, unordered_set<string> > usageOfInputs;
    map<string, unordered_set<string> > outputOfInputs;
    NIdentifier output;
    unique_ptr<NBlock> body;
public:
    FunctionDeclaration(): inputs(), callInputMap(), usageOfInputs(), outputOfInputs(), output("", "", 0), body() {}

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

    void mapCallInput(const string &localName, const string &nameInCall) {
        callInputMap.emplace(localName, nameInCall);
    }

    const string getCallArgument(const string &localName) const {
        if (callInputMap.count(localName) == 0) {
            return "";
        }
        return callInputMap.at(localName);
    }

    void clearCallInputMap() {
        callInputMap.clear();
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
    virtual ~StaticAnalyzer() {}
};

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on int count from parser)
    unsigned int numOfBitsPerInt;

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
    unsigned int getSatVariable(FullVariableNameOccurrence &nIdentifier);

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
    SatStaticAnalyzer() : solver(new SATSolver), variables(), variableOccurrences(), callStack(),
                          functions(), currentFunctionName(), answers() {
        // we need at least one bit for boolean variable
        numOfBitsPerInt = (unsigned int) ceil(log2(max((unsigned int) 2, NInteger::differentIntCount)));
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

    vector<pair<int, unsigned int> > getAnswers() {
        return answers;
    };
    void updateAnswers(const string &opName, FullVariableName &keyLhs, const NIdentifier &lhs);

    const string getCurrentCall() {
        assert(!callStack.empty());
        return callStack.back();
    }

    const string getTopOfStack() const {
        return currentFunctionName;
    }

    const string getParentCall() {
        if (callStack.size() == 1)
            return currentFunctionName;
        list<string>::reverse_iterator rit=callStack.rbegin();
        ++rit;
        if(rit != callStack.rend())
            return *rit;
        else
            return "";
    }

    FunctionDeclaration *getFunction(const string &name) {
        if (functions.count(name) == 0) {
            throw FunctionIsNotDefined();
        }
        return functions.at(name).get();
    }

    void addTrueOutput(const NIdentifier &variableName);

    virtual bool isConstant(int &returnValue, FullVariableNameOccurrence &nIdentifier);

    virtual ~SatStaticAnalyzer() {}

    unsigned int getRhsSatVariable(FullVariableNameOccurrence &key);

    unsigned int addLhsSatVariable(FullVariableNameOccurrence &key);

    void addClauses(FullVariableName &lhs, FullVariableName &rhs);

    FullVariableNameOccurrence getFullVariableNameOccurrence(FullVariableName &fullVariableName) const;
};