#include <vector>
#include <map>

#include "assignment.hpp"
#include "node.h"

using namespace std;

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class FunctionInLanguage : public Assignment {
    vector<string> inputs;
    vector<string> outputs;
    map<string, Assignment *> variables;
    string nameOfFunction;
public:
    FunctionInLanguage(const string& name): Assignment(4), nameOfFunction(name) {}

    void addInput(NVariableDeclaration* NVariable);

    void addOutput(string nameWithField);

    void addVariable(string name, Assignment *value);

    Assignment * evaluateAssignment(NExpression *currentExpression);
    Assignment * evaluateAssignment(NIdentifier *currentExpression);

    bool checkIfIsInput(NIdentifier *currentIdentifier);

    string getVariableName(NIdentifier *currentIdentifier);

    ~FunctionInLanguage();
};

class NaiveStaticAnalyzer : public StaticAnalyzer {
    vector<Assignment*> functions;
    FunctionInLanguage *currentFunction;
public:
    NaiveStaticAnalyzer(NBlock* programBlock);

    void processBody(NBlock & block);

    void processAssignment(NStatement *currentStatement);
    void processAssignment(NVariableDeclaration *nAssignment);

    virtual ~NaiveStaticAnalyzer() {
        // TODO: use smart pointers as functions
    }
};