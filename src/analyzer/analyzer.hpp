#include <vector>
#include <map>

#include "assignment.hpp"
#include "node.h"

using namespace std;

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class FunctionDeclaration : public Assignment {
    vector<string> inputs;
    vector<pair<string,string> > outputs;
    map<string, Assignment *> variables;
    string nameOfFunction;
public:
    FunctionDeclaration(const string& name): Assignment(4), nameOfFunction(name) {}

    void addInput(NVariableDeclaration* NVariable);

    void addOutput(const string &name, const string &field);

    const vector<pair<string, string> > getOutput() {
        return outputs;
    }

    void addVariable(string name, Assignment *value);

    string getName() {
        return nameOfFunction;
    }

    Assignment * evaluateAssignment(NExpression *currentExpression);
    Assignment * evaluateAssignment(NIdentifier *currentExpression);

    pair<string, Assignment *> evaluateFunction(const pair<string, string> output, ExpressionList arguments);

    map<string, NExpression*> mapInputs(ExpressionList arguments);

    void substitute(FunctionDeclaration *function, ExpressionList arguments);

    bool checkIfIsInput(NIdentifier *currentIdentifier);

    string getVariableName(NIdentifier *currentIdentifier);

    ~FunctionDeclaration();
};

class SymbolicStaticAnalyzer : public StaticAnalyzer {
    vector<FunctionDeclaration*> functions;
    FunctionDeclaration *currentFunction;
public:
    SymbolicStaticAnalyzer(NBlock* programBlock);

    void processBody(NBlock & block);

    void processAssignment(NStatement *currentStatement);
    void processAssignment(NVariableDeclaration *nAssignment);

    virtual ~SymbolicStaticAnalyzer() {
        // TODO: use smart pointers as functions
    }
};