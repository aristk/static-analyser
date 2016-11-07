#include <iostream>
#include <vector>
#include <typeinfo>

#ifndef _NODE_H_
#define _NODE_H_

#include "analyzer/exceptions.hpp"

class SatStaticAnalyzer;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NIdentifier;

// TODO: use shared_ptr for deleting Assignments
typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
    virtual ~Node() {}

    std::string name() const { return typeid(*this).name(); }

    virtual void genCheck(SatStaticAnalyzer& context) {
        std::cerr << name() << std::endl;
        throw genCheckNotImplemented();
    }

    virtual void addClauses(NIdentifier& nIdentifier, SatStaticAnalyzer& context) {
        std::cerr << name() << std::endl;
        throw genCheckNotImplemented();
    }
};

class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
    int value;
    NInteger(int value) : value(value) { }
    virtual void genCheck(SatStaticAnalyzer& context)
    {
        throw genCheckNotImplemented();
    }

    virtual void addClauses(NIdentifier& nIdentifier, SatStaticAnalyzer& context);
};

class NIdentifier : public NExpression {
public:
    std::string name;
    std::string field;
    NIdentifier(const std::string& name) : name(name) { }
    NIdentifier(const std::string& name, const std::string& field) : name(name), field(field) { }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
            id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id) { }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBinaryOperator : public NExpression {
public:
    boolpi op;
    NIdentifier& lhs;
    NIdentifier& rhs;
    NBinaryOperator(NIdentifier& lhs, int op, NIdentifier& rhs) :
            lhs(lhs), rhs(rhs), op(op) { }

    virtual void addClauses(NIdentifier& nIdentifier, SatStaticAnalyzer& context);
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) :
            lhs(lhs), rhs(rhs) { }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }

    virtual void genCheck(SatStaticAnalyzer& context);
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
            expression(expression) { }
//    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NReturnStatement: public NStatement {
public:
    NIdentifier& variable;
    NReturnStatement(NIdentifier& variable) :
            variable(variable) { }
};

class NVariableDeclaration : public NStatement {
public:
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(NIdentifier& id) :
            id(id) { }
    NVariableDeclaration(NIdentifier& id, NExpression *assignmentExpr) :
            id(id), assignmentExpr(assignmentExpr) { }

    virtual void genCheck(SatStaticAnalyzer& context);
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& id,
                         const VariableList& arguments, NBlock& block) :
            id(id), arguments(arguments), block(block) { }

    virtual void genCheck(SatStaticAnalyzer& context);
};

#endif