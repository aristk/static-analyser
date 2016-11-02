#include "node.h"
#include <vector>

using namespace std;

class notNFunctionDeclaration: public exception
{
    virtual const char* what() const throw()
    {
        return "Root item is not NFunctionDeclaration.";
    }
};

class StaticAnalyzer {
public:
    virtual ~StaticAnalyzer() {}
};

class FunctionInLanguage {
    vector<string> inputs;
    vector<string> outputs;
    string name;
public:
    FunctionInLanguage(const string& name): name(name) {}
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