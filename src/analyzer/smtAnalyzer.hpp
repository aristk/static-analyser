#include "satAnalyzer.hpp"
#include "c++/z3++.h"

class Z3SatStaticAnalyzer : public SatStaticAnalyzer {
    unique_ptr<z3::solver> Z3solver;
public:
    Z3SatStaticAnalyzer(z3::context &c) : SatStaticAnalyzer(), Z3solver(new z3::solver(c)) {
    }

    virtual void addClauses(const NIdentifier &lhs, const NInteger &nInteger);
    virtual void addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator);
    virtual void addClauses(const NIdentifier &lhs, const NIdentifier &rhs);

    virtual bool isConstant(int &returnValue, const NIdentifier &nIdentifier);

    virtual ~Z3SatStaticAnalyzer() {
    }

};