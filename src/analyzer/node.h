#include <iostream>
#include <vector>
#include <typeinfo>
#include <map>

#ifndef _NODE_H_
#define _NODE_H_

#include "exceptions.hpp"
#include "types.hpp"

class SatStaticAnalyzer;
class NStatement;
class NExpression;
class NVariableDeclaration;
class NIdentifier;

// TODO: use shared_ptr for deleting Assignments
typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NIdentifier*> VariableList;
typedef tuple<string, string, string> FullVariableName;

class Node {
public:
    virtual ~Node() {}

    std::string name() const { return typeid(*this).name(); }

    virtual void genCheck(SatStaticAnalyzer &context) const {
        // TODO: bad practice, should be caught during compilation
        throw functionIsNotImplemented("genCheck", name());
    }

    virtual void addClauses(FullVariableName& nIdentifier, SatStaticAnalyzer& context) {
        throw functionIsNotImplemented("addClauses", name());
    }
};

class NExpression : public Node {
public:
    virtual unique_ptr<LanguageType> mapVariables(const string &functionName, const string &inputName,
                                                  SatStaticAnalyzer &context) {
        // TODO: throw that only int and variables allowed here
        throw functionIsNotImplemented("mapVariables", name());
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
    NInteger(int value) : value(value) { }

    virtual void addClauses(FullVariableName& nIdentifier, SatStaticAnalyzer& context);

    virtual unique_ptr<LanguageType> mapVariables(const string &functionName, const string &inputName,
                                                  SatStaticAnalyzer &context);
};

class NIdentifier : public NExpression {
public:
    std::string name;
    std::string field;
    NIdentifier(const std::string& name) : name(name) { }
    NIdentifier(const std::string& name, const std::string& field) : name(name), field(field) { }

    virtual void addClauses(FullVariableName& nIdentifier, SatStaticAnalyzer& context);

    std::string printName() const {
        if(field != "")
            return name + "." + field;
        else
            return name;
    }

    virtual unique_ptr<LanguageType> mapVariables(const string &functionName, const string &inputName,
                                                  SatStaticAnalyzer &context);
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    int lineNumber;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments, int lineNumber) :
            id(id), arguments(arguments), lineNumber(lineNumber) { }

    virtual void genCheck(SatStaticAnalyzer& context) const;

    virtual void addClauses(FullVariableName& nIdentifier, SatStaticAnalyzer& context);
};

class NBinaryOperator : public NExpression {
public:
    bool op;
    NIdentifier& lhs;
    NIdentifier& rhs;
    int lineNumber;
    NBinaryOperator(NIdentifier& lhs, int op, NIdentifier& rhs, int lineNumber) :
            lhs(lhs), rhs(rhs), op(op), lineNumber(lineNumber) { }

    virtual void addClauses(FullVariableName& nIdentifier, SatStaticAnalyzer& context);
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

    virtual void genCheck(SatStaticAnalyzer &context) const;
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
            expression(expression) { }

    virtual void genCheck(SatStaticAnalyzer& context) const {
        expression.genCheck(context);
    }
};

class NReturnStatement: public NStatement {
public:
    NIdentifier& variable;
    NReturnStatement(NIdentifier& variable) :
            variable(variable) { }

    virtual void genCheck(SatStaticAnalyzer& context) const;
};

class NVariableDeclaration : public NStatement {
public:
    NIdentifier& id;
    unique_ptr<NExpression> assignmentExpr;
    NVariableDeclaration(NIdentifier& id) :
            id(id) { }
    NVariableDeclaration(NIdentifier& id, NExpression *assignmentExpr) :
            id(id), assignmentExpr(assignmentExpr) { }

    virtual void genCheck(SatStaticAnalyzer& context) const;
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& id,
                         const VariableList& arguments, NBlock& block) :
            id(id), arguments(arguments), block(block) { }

    virtual void genCheck(SatStaticAnalyzer& context) const;

    virtual bool isFunctionDeclaration () {
        return true;
    }
};

#endif