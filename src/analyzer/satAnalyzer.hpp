#include "analyzer.hpp"
#include "cryptominisat.h"

class NBlock;
using namespace CMSat;

class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;
    std::unique_ptr<SATSolver> solver;
    // relation of NIdentifier to variables
    map<pair<string, string>, unsigned int> variables;

    map<string, vector<string> > functionInputs;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier);
    unsigned int addNewVariable(const NIdentifier &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables(), functionInputs()  {}

    void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    void addClauses(const NIdentifier &lhs, const NIdentifier &nIdentifier);

    void addInputs(const VariableList &inputs, const NIdentifier &functionName);

    void generateCheck(const NBlock& root);

    SATSolver *getSolver() const {
        return solver.get();
    }

    void tryValue(bool value, const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator,
                  unsigned int newVarLast) const;
};