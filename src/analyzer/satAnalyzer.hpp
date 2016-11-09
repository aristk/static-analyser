#include "analyzer.hpp"
#include "cryptominisat.h"
#include <set>

class NBlock;
using namespace CMSat;

class SatFunctionDeclaration {
    // std::set allow fast check that string is an input
    set<string> inputs;
    vector<string> outputs;
public:
    SatFunctionDeclaration(): inputs(), outputs() {}

    void addInput(const string &input) {
        if (isInput(input)) {
            throw isAlreadyAnInput(input);
        }
        inputs.emplace(input);
    }

    bool isInput(const string & name) const {
        return inputs.count(name) > 0;
    }

    void addOutput(const string &outputVariable) {
        outputs.push_back(outputVariable);
    }
};

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;

    std::unique_ptr<SATSolver> solver;

    // relation of NIdentifier to variables
    map<pair<string, string>, unsigned int> variables;

    map<string, std::unique_ptr<SatFunctionDeclaration> > functions;

    string currentFunctionName;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier);
    unsigned int addNewVariable(const NIdentifier &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), functions(), currentFunctionName() {}

    void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    void addClauses(const NIdentifier &lhs, const NIdentifier &nIdentifier);

    void addInputs(const VariableList &inputs, const NIdentifier &functionName);

    void generateCheck(const NBlock& root);

    bool isCurrentInput(const NIdentifier &nIdentifier);

    SATSolver *getSolver() const {
        return solver.get();
    }

    SatFunctionDeclaration *getFunction(const string &name) {
        if (functions.count(name) == 0) {
            throw FunctionIsNotDefined();
        }
        return functions.at(name).get();
    }

    void addReturn(const NIdentifier &variableName);

    void tryValue(bool value, const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator,
                  unsigned int newVarLast) const;
};