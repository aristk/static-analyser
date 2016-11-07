#include "analyzer.hpp"
#include "cryptominisat.h"

class NBlock;
using namespace CMSat;

// TODO: output should be line number in the file + value
class SatStaticAnalyzer : public StaticAnalyzer {
    // how many bits we need per integer (depends on max int from parser)
    const int numOfBitsPerInt;
    std::unique_ptr<SATSolver> solver;
    // relation of NIdentifier to variables
    map<pair<string, string>, int> variables;
public:
    SatStaticAnalyzer() : numOfBitsPerInt(2), solver(new SATSolver), variables()  {}

    void addClauses(NIdentifier &nIdentifier, NInteger &nInteger);

    void generateCheck(NBlock& root);

    SATSolver *getSolver() {
        return solver.get();
    }
};