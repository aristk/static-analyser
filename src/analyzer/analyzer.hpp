#include <vector>
#include <map>

#include "assignment.hpp"
#include "node.h"

using namespace std;

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class FunctionInLanguage {
    vector<string> inputs;
    vector<string> outputs;
    map<string, Assignment *> variables;
    string name;
public:
    FunctionInLanguage(const string& name): name(name) {}

    void addInput(NVariableDeclaration* NVariable);

    void processBody(NBlock & block);

    void processAssignment(NStatement *currentStatement);

    Assignment * evaluateAssignment(NExpression *currentExpression);

    string getVariableName(NIdentifier *currentIdentifier);

    ~FunctionInLanguage();
};

class NaiveStaticAnalyzer : public StaticAnalyzer {
    vector<FunctionInLanguage*> functions;
public:
    NaiveStaticAnalyzer(NBlock* programBlock);
    virtual ~NaiveStaticAnalyzer() {
        for(auto i : functions)
            delete(i);
    }
};