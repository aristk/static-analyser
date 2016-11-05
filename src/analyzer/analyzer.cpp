#include "analyzer.hpp"
#include "exceptions.hpp"

#include <string>

extern const std::string tokenIdToName(int value);

using namespace std;

SymbolicStaticAnalyzer::SymbolicStaticAnalyzer(NBlock *programBlock) {
    for(auto i : programBlock->statements) {
        // check that all root items are NFunctionDeclaration
        // TODO: usage of dynamic_cast is not good idea
        NFunctionDeclaration *nFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (nFunction == 0) {
            throw notNFunctionDeclaration();
        }

        // TODO: look like better way is to all follows in construct of FunctionDeclaration
        currentFunction = new FunctionDeclaration(nFunction->id.name);
        functions.push_back(currentFunction);

        // add input variable
        for(auto j : nFunction->arguments) {
            // note that all inputs are of NVariableDeclaration type by parsing
            // TODO: add assertion with check that j is of NVariableDeclaration type
            currentFunction->addInput(j);
        }

        processBody(nFunction->block);
    }
}

void SymbolicStaticAnalyzer::processBody(NBlock &block) {
    for(auto i : block.statements) {
        // could be
        // 1. NVariableDeclaration or
        // 2. NReturnStatement or
        // 3. NExpressionStatement with NMethodCall
        processAssignment(i);
    }
}

void SymbolicStaticAnalyzer::processAssignment(NStatement *currentStatement) {
    // TODO: get rid of dynamic_cast, as alternative call processAssignment from NStatement
    NVariableDeclaration *nAssignment = dynamic_cast<NVariableDeclaration *>(currentStatement);

    if (nAssignment != 0) {
        processAssignment(nAssignment);
        return;
    }

    NReturnStatement *nReturnStatement = dynamic_cast<NReturnStatement *>(currentStatement);

    if (nReturnStatement != 0) {
        string nameWithField = currentFunction->getVariableName(&(nReturnStatement->variable));
        currentFunction->addOutput(nameWithField);
        return;
    }

    NExpressionStatement *nExpressionStatement = dynamic_cast<NExpressionStatement *>(currentStatement);

    // here it could be only method call
    // TODO: change parser to avoid addition convention
    if(nExpressionStatement !=0 ) {
        NMethodCall *nMethodCall = dynamic_cast<NMethodCall *>(&(nExpressionStatement->expression));

        // call without return
        if(nMethodCall !=0 ) {
            string callName = nMethodCall->id.name;
            FunctionDeclaration *pFunctionDeclaration;
            // TODO: better structure for functions (to have better search)?
            for(auto i : functions)
            {
                if (i->getName() == callName) {
                    pFunctionDeclaration = i;
                    break;
                }
            }

        }
        return;
    }

    throw WrongFunctionStatement();
}

void SymbolicStaticAnalyzer::processAssignment(NVariableDeclaration *nAssignment) {
    string name = nAssignment->id.name;
    string field = nAssignment->id.field;
    string nameWithField = currentFunction->getVariableName(&(nAssignment->id));
    // dedicated case if input variable is a structure and having assignment to its field
    if (currentFunction->checkIfIsInput(&(nAssignment->id)) && field != "") {
        currentFunction->addOutput(nameWithField);
    }

    currentFunction->addVariable(nameWithField, currentFunction->evaluateAssignment(nAssignment->assignmentExpr));
    /* TODO: case of
    *  X.Y = Z
    *  W.S = V
    *  X = W
    * when X is input variable should be accurately checked
    */
}

Assignment *SymbolicStaticAnalyzer::substitute(FunctionDeclaration *function, ExpressionList arguments) {
    for(auto i : arguments) {

    }
}

void FunctionDeclaration::addInput(NVariableDeclaration* NVariable) {
    if (NVariable->id.field != "" || NVariable->assignmentExpr != NULL) {
        throw WrongFunctionArgument();
    }
    string name = NVariable->id.name;
    inputs.push_back(name);
    variables.emplace(name, new InputVariable(name));
}

FunctionDeclaration::~FunctionDeclaration() {
    // TODO: use shared_ptr for deleting Assignments
}

Assignment *FunctionDeclaration::evaluateAssignment(NIdentifier *currentIdentifier) {
    // simple case: we have identifier in rhs
    string field = currentIdentifier->field;
    string name = currentIdentifier->name;
    string fullName = getVariableName(currentIdentifier);
    // if name is input and structure, init as input
    if (variables.count(fullName) == 0 && checkIfIsInput(currentIdentifier) && field != "") {
        variables.emplace(fullName, new InputVariable(fullName));
    }
    return variables[fullName];
}

Assignment *FunctionDeclaration::evaluateAssignment(NExpression *currentExpression) {
    NIdentifier *currentIdentifier = dynamic_cast<NIdentifier *>(currentExpression);

    if (currentIdentifier != 0) {
        return evaluateAssignment(currentIdentifier);
    }

    NBinaryOperator *currentOp = dynamic_cast<NBinaryOperator *>(currentExpression);

    if (currentOp != 0) {
        Assignment *lhs = evaluateAssignment(&(currentOp->lhs));
        Assignment *rhs = evaluateAssignment(&(currentOp->rhs));
        const string op = tokenIdToName(currentOp->op);
        if (lhs->isEqualTo(rhs)) {
            if (op == "==") {
                return new IntegerAssignment(1);
            } else if (op == "!="){
                // case of !=
                return new IntegerAssignment(0);
            } else {
                throw WrongBinaryOperator();
            }
        } else {
            return new LogicAssignment(op, lhs, rhs);
        }
    }

    return nullptr;
    // TODO: check other options
}

string FunctionDeclaration::getVariableName(NIdentifier *currentIdentifier) {
    if (currentIdentifier->field != "") {
        return currentIdentifier->name + "." + currentIdentifier->field;
    } else {
        return currentIdentifier->name;
    }

}

bool FunctionDeclaration::checkIfIsInput(NIdentifier *currentIdentifier) {
    string name = currentIdentifier->name;

    if (variables.count(name) > 0) {
        Assignment *currentVariable = variables[name];
        // process input
        if (currentVariable != 0 && currentVariable->getId() == 0) {
            return true;
        }
    }
    return false;
}

void FunctionDeclaration::addOutput(string nameWithField) {
    // TODO: make outputs a map
    outputs.push_back(nameWithField);
}

void FunctionDeclaration::addVariable(string name, Assignment *value) {
    variables[name] = value;
}

