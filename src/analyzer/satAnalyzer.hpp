#include "analyzer.hpp"
#include "cryptominisat.h"

class NBlock;
using namespace CMSat;

// TODO: output should be line number in the file + value
class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const unsigned int numOfBitsPerInt;
    std::unique_ptr<SATSolver> solver;
    // relation of NIdentifier to variables
    map<pair<string, string>, unsigned int> variables;

    unsigned int getIdentifierVariables(const NIdentifier &nIdentifier) const;
    unsigned int addNewVariable(const NIdentifier &nIdentifier);
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables()  {}

    void addClauses(const NIdentifier &nIdentifier, const NInteger &nInteger);
    void addClauses(const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator);

    void generateCheck(const NBlock& root);

    SATSolver *getSolver() const {
        return solver.get();
    }

    void tryValue(bool value, const NIdentifier &nIdentifier, const NBinaryOperator &nBinaryOperator,
                  unsigned int newVarLast) const;
};