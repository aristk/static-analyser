#include "smtAnalyzer.hpp"

using namespace z3;

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NInteger &nInteger) {
    context &c = Z3solver->ctx();
    expr x = c.int_const(lhs.printName().c_str());
    expr y = c.int_val(nInteger.value);
    expr relation = (x == y);
    Z3solver->add(relation);
}

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator) {
    context &c = Z3solver->ctx();
    expr x = c.bool_const(lhs.printName().c_str());
    expr y = c.int_const(nBinaryOperator.lhs.printName().c_str());
    expr z = c.int_const(nBinaryOperator.rhs.printName().c_str());
    expr relation = (x == (y == z));
    Z3solver->add(relation);

    expr assumptions[1] = { x };
    check_result result = Z3solver->check(1, assumptions);

    cout << result << endl;

    expr assumptions1[1] = { !x };
    result = Z3solver->check(1, assumptions1);

    cout << result << endl;


    cout << Z3solver->assertions() << endl;

    cout <<  Z3solver->statistics() << endl;
}

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NIdentifier &rhs) {
    context &c = Z3solver->ctx();
    expr x = c.int_const(lhs.printName().c_str());
    expr y = c.int_const(rhs.printName().c_str());
    expr relation = (x == y);
    Z3solver->add(relation);
}

bool Z3SatStaticAnalyzer::isConstant(int &returnValue, const NIdentifier &nIdentifier) {
    return false;
}
