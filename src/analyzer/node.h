#include <iostream>
#include <map>
#include <memory>
#include <typeinfo>
#include <utility>
#include <vector>

#ifndef _NODE_H_
#define _NODE_H_

#include "exceptions.hpp"
#include "variableName.hpp"

class IncrementalSatStaticAnalyzer;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NIdentifier;

// TODO(arist): use shared_ptr
using StatementList = std::vector<NStatement *>;
using ExpressionList = std::vector<NExpression *>;
using VariableList = std::vector<NIdentifier *>;

class Node {
public:
    virtual ~Node() = default;

    std::string name() const { return typeid(*this).name(); }

    virtual void genCheck(IncrementalSatStaticAnalyzer & /*context*/) const {
        throw functionIsNotImplemented("genCheck", name());
    }

    virtual void addClauses(const NIdentifier & /*nIdentifier*/, IncrementalSatStaticAnalyzer & /*context*/) const {
        throw functionIsNotImplemented("addClauses", name());
    }
};

class NExpression : public Node {
public:
    virtual void processCallInput(unsigned int  /*inputId*/, IncrementalSatStaticAnalyzer & /*context*/) {
        throw functionIsNotImplemented("processCallInput", name());
    }

    virtual void processCallOutput(unsigned int  /*inputId*/, IncrementalSatStaticAnalyzer & /*context*/) {
        throw functionIsNotImplemented("processCallOutput", name());
    }
};

class NStatement : public Node {
public:
    virtual bool isFunctionDeclaration () {
        return false;
    }
};

class NInteger : public NExpression {
public:
    int value;
    static map<int, unsigned int> intMapping;
    static unsigned int differentIntCount;
    explicit NInteger(int value) : value(value) {
        if (intMapping.count(value) == 0) {
            intMapping[value] = differentIntCount;
            differentIntCount++;
        }
    }

    void processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) override;

    void processCallOutput(unsigned int  /*inputId*/, IncrementalSatStaticAnalyzer & /*context*/) override {

    }

    void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const override;

};

class NIdentifier : public NExpression {
public:
    std::string name;
    std::string field;
    unsigned int lineNumber;
    NIdentifier(std::string  name, unsigned int lineNumber) : name(std::move(name)), lineNumber(lineNumber) { }
    NIdentifier(std::string name, std::string field, unsigned int lineNumber) :
            name(std::move(name)), field(std::move(field)), lineNumber(lineNumber) { }

    void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context ) const override ;

    std::string printName() const {
        if(field  != "") {
            return name + "." + field;
        }
            return name;

    }

    void processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) override;

    void processCallOutput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) override;
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
            id(id), arguments(arguments) { }

    void genCheck(IncrementalSatStaticAnalyzer& context) const override;

    void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const override;
};

class NBinaryOperator : public NExpression {
public:
    bool op;
    NIdentifier& lhs;
    NIdentifier& rhs;
    NBinaryOperator(NIdentifier &lhs, bool op, NIdentifier &rhs) :
            lhs(lhs), rhs(rhs), op(op) { }

    void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const override;
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) :
            lhs(lhs), rhs(rhs) { }
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }

    void genCheck(IncrementalSatStaticAnalyzer &context) const override;
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    explicit NExpressionStatement(NExpression& expression) :
            expression(expression) { }

    void genCheck(IncrementalSatStaticAnalyzer& context) const override {
        expression.genCheck(context);
    }
};

class NReturnStatement: public NStatement {
public:
    NIdentifier& variable;
    explicit NReturnStatement(NIdentifier& variable) :
            variable(variable) { }

    void genCheck(IncrementalSatStaticAnalyzer& context) const override;
};

class NVariableDeclaration : public NStatement {
public:
    NIdentifier& id;
    unique_ptr<NExpression> assignmentExpr;
    explicit NVariableDeclaration(NIdentifier& id) :
            id(id) { }
    NVariableDeclaration(NIdentifier& id, NExpression *assignmentExpr) :
            id(id), assignmentExpr(assignmentExpr) { }

    void genCheck(IncrementalSatStaticAnalyzer& context) const override;
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& id, VariableList arguments, NBlock& block) :
            id(id), arguments(std::move(arguments)), block(block) { }

    void genCheck(IncrementalSatStaticAnalyzer& context) const override;

    bool isFunctionDeclaration () override {
        return true;
   }
};

#endif