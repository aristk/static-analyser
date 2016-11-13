#include "smtAnalyzer.hpp"

using namespace z3;

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NInteger &nInteger) {
    SatFunctionDeclaration *currentFunction =  getFunction(getCurrentFunctionName());

    string lhsUniqueName = currentFunction->incUniqueName(lhs.printName());

    context &c = Z3solver->ctx();
    expr x = c.int_const(lhsUniqueName.c_str());
    expr y = c.int_val(nInteger.value);
    expr relation = (x == y);
    Z3solver->add(relation);
}

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NBinaryOperator &nBinaryOperator) {
    SatFunctionDeclaration *currentFunction =  getFunction(getCurrentFunctionName());

    string lhsUniqueName = currentFunction->incUniqueName(lhs.printName());

    string binLhsUniqueName = currentFunction->getUniqueName(nBinaryOperator.lhs.printName());
    string binRhsUniqueName = currentFunction->getUniqueName(nBinaryOperator.rhs.printName());

    context &c = Z3solver->ctx();
    expr x = c.bool_const(lhsUniqueName.c_str());
    expr y = c.int_const(binLhsUniqueName.c_str());
    expr z = c.int_const(binRhsUniqueName.c_str());
    expr relation = (x == (y == z));
    Z3solver->add(relation);

    expr assumptions[1] = { x };
    check_result result = Z3solver->check(1, assumptions);

    cout << result << endl;

    expr assumptions1[1] = { !x };
    result = Z3solver->check(1, assumptions1);

    cout << result << endl;

    int returnValue;
    if (isConstant(returnValue, lhs)) {
        cout << lhs.printName() << " from binOp is " << returnValue <<
             " at line " << lhs.lineNumber << endl;
    }

}

void Z3SatStaticAnalyzer::addClauses(const NIdentifier &lhs, const NIdentifier &rhs) {

    SatFunctionDeclaration *currentFunction =  getFunction(getCurrentFunctionName());

    string lhsUniqueName = currentFunction->incUniqueName(lhs.printName());
    string rhsUniqueName = currentFunction->getUniqueName(rhs.printName());

    context &c = Z3solver->ctx();
    expr x = c.int_const(lhsUniqueName.c_str());
    expr y = c.int_const(rhsUniqueName.c_str());
    expr relation = (x == y);
    Z3solver->add(relation);
}

bool Z3SatStaticAnalyzer::isConstant(int &returnValue, const NIdentifier &nIdentifier) {
    check_result result = Z3solver->check();

    if (result == unsat)
    {
        throw isWrongModel();
    }
    model mModel = Z3solver->get_model();

    SatFunctionDeclaration *currentFunction =  getFunction(getCurrentFunctionName());
    string idName = currentFunction->getUniqueName(nIdentifier.printName());

    context &c = Z3solver->ctx();
    expr x = c.int_const(idName.c_str());
    expr value = mModel.get_const_interp(x.decl());

    cout << mModel;

    cout << x << " " << value << endl;

//    expr assumptions1[1] = { (value != x) };
//    result = Z3solver->check(1, assumptions1);

    return result == unsat;
}
