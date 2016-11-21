#include <iostream>
#include <vector>
#include <typeinfo>
#include <map>
#include <memory>

#ifndef _NODE_H_
#define _NODE_H_

#include "exceptions.hpp"
#include "variableName.hpp"

class IncrementalSatStaticAnalyzer;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NIdentifier;

// TODO: use shared_ptr
typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NIdentifier*> VariableList;

class Node {
public:
    virtual ~Node() {}

    std::string name() const { return typeid(*this).name(); }

    virtual void genCheck(IncrementalSatStaticAnalyzer &context) const {
        throw functionIsNotImplemented("genCheck", name());
    }

    virtual void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const {
        throw functionIsNotImplemented("addClauses", name());
    }
};

class NExpression : public Node {
public:
    virtual void processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {
        throw functionIsNotImplemented("processCallInput", name());
    }

    virtual void processCallOutput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {
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
    NInteger(int value) : value(value) {
        if (intMapping.count(value) == 0) {
            intMapping[value] = differentIntCount;
            differentIntCount++;
        }
    }

    virtual void processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context);

    virtual void processCallOutput(unsigned int inputId, IncrementalSatStaticAnalyzer &context) {

    }

    virtual void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const;

};

class NIdentifier : public NExpression {
public:
    std::string name;
    std::string field;
    unsigned int lineNumber;
    NIdentifier(const std::string& name, unsigned int lineNumber) : name(name), lineNumber(lineNumber) { }
    NIdentifier(const std::string& name, const std::string& field, unsigned int lineNumber) :
            name(name), field(field), lineNumber(lineNumber) { }

    virtual void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const;

    std::string printName() const {
        if(field != "")
            return name + "." + field;
        else
            return name;
    }

    virtual void processCallInput(unsigned int inputId, IncrementalSatStaticAnalyzer &context);

    virtual void processCallOutput(unsigned int inputId, IncrementalSatStaticAnalyzer &context);
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
            id(id), arguments(arguments) { }

    virtual void genCheck(IncrementalSatStaticAnalyzer& context) const;

    virtual void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const;
};

class NBinaryOperator : public NExpression {
public:
    bool op;
    NIdentifier& lhs;
    NIdentifier& rhs;
    NBinaryOperator(NIdentifier &lhs, bool op, NIdentifier &rhs) :
            lhs(lhs), rhs(rhs), op(op) { }

    virtual void addClauses(const NIdentifier &nIdentifier, IncrementalSatStaticAnalyzer &context) const;
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

    virtual void genCheck(IncrementalSatStaticAnalyzer &context) const;
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
            expression(expression) { }

    virtual void genCheck(IncrementalSatStaticAnalyzer& context) const {
        expression.genCheck(context);
    }
};

class NReturnStatement: public NStatement {
public:
    NIdentifier& variable;
    NReturnStatement(NIdentifier& variable) :
            variable(variable) { }

    virtual void genCheck(IncrementalSatStaticAnalyzer& context) const;
};

class NVariableDeclaration : public NStatement {
public:
    NIdentifier& id;
    unique_ptr<NExpression> assignmentExpr;
    NVariableDeclaration(NIdentifier& id) :
            id(id) { }
    NVariableDeclaration(NIdentifier& id, NExpression *assignmentExpr) :
            id(id), assignmentExpr(assignmentExpr) { }

    virtual void genCheck(IncrementalSatStaticAnalyzer& context) const;
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& id,
                         const VariableList& arguments, NBlock& block) :
            id(id), arguments(arguments), block(block) { }

    virtual void genCheck(IncrementalSatStaticAnalyzer& context) const;

    virtual bool isFunctionDeclaration () {
        return true;
    }
};

#endif