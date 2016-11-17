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
    map<string, string> callInputMap;
    NIdentifier output;
    unique_ptr<NBlock> body;
public:
    FunctionDeclaration(): inputs(), callInputMap(), output("", "", 0), body() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputsMap.emplace(input);
        inputs.push_back(input);
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

    std::unique_ptr<SATSolver> solver;

    // relation of function, struct, field (NIdentifier) to Boolean variables
    map<FullVariableNameOccurrence, unsigned int> variables;
    map<FullVariableName, unsigned int> variableOccurrences;

    // stack do not support iterators that we need for requrences search
    list<string> callStack;

    map<string, std::unique_ptr<FunctionDeclaration> > functions;

    string currentFunctionName;

    vector<pair<int, unsigned int> > answers;

    unsigned int addNewSatVariable(const NIdentifier &nIdentifier);
    unsigned int getSatVariable(const NIdentifier &nIdentifier);

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
    void updateAnswers(const string &opName, const NIdentifier &lhs);

    const string getCurrentCall() {
        assert(!callStack.empty());
        return callStack.back();
    }

    FunctionDeclaration *getFunction(const string &name) {
        if (functions.count(name) == 0) {
            throw FunctionIsNotDefined();
        }
        return functions.at(name).get();
    }

    void addTrueOutput(const NIdentifier &variableName);

    virtual bool isConstant(int &returnValue, const NIdentifier &nIdentifier);

    virtual ~SatStaticAnalyzer() {}

};