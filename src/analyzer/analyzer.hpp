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
    vector<string> outputs;
    map<string, Assignment *> variables;
    string nameOfFunction;
public:
    FunctionDeclaration(const string& name): Assignment(4), nameOfFunction(name) {}

    void addInput(NVariableDeclaration* NVariable);

    void addOutput(string nameWithField);

    void addVariable(string name, Assignment *value);

    string getName() {
        return nameOfFunction;
    }

    Assignment * evaluateAssignment(NExpression *currentExpression);
    Assignment * evaluateAssignment(NIdentifier *currentExpression);

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

    Assignment * substitute(FunctionDeclaration *function, ExpressionList arguments);

    virtual ~SymbolicStaticAnalyzer() {
        // TODO: use smart pointers as functions
    }
};