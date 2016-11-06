#include "cryptominisat.h"
#include "solvertypesmini.h"

using std::vector;
using namespace CMSat;

extern int core(int argc, char **argv);

int main(int argc, char **argv)
{
    SATSolver solver;
    vector<Lit> clause;
    solver.new_vars(3);

    //adds "1 0"
    clause.push_back(Lit(0, false));
    solver.add_clause(clause);

    //adds "-2 0"
    clause.clear();
    clause.push_back(Lit(1, true));
    solver.add_clause(clause);

    //adds "-1 2 3 0"
    clause.clear();
    clause.push_back(Lit(0, true));
    clause.push_back(Lit(1, false));
    clause.push_back(Lit(2, false));
    solver.add_clause(clause);

    lbool ret = solver.solve();

    return core(argc, argv);
}