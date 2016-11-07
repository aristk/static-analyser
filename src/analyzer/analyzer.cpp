#include "analyzer.hpp"

extern const std::string tokenIdToName(int value);

using namespace std;

SymbolicStaticAnalyzer::SymbolicStaticAnalyzer(NBlock *programBlock) {
    for(auto i : programBlock->statements) {
        // check that all root items are NFunctionDeclaration
        NFunctionDeclaration *nFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (nFunction == 0) {
            throw notNFunctionDeclaration();
        }

        currentFunction = new FunctionDeclaration(nFunction->id.name);
        functions.push_back(currentFunction);

        // add input variable
        for(auto j : nFunction->arguments) {
            // note that all inputs are of NVariableDeclaration type by parsing
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

void SymbolicStaticAnalyzer::processAssignment(NMethodCall *nMethodCall) {
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
    currentFunction->substitute(pFunctionDeclaration, nMethodCall->arguments);
}

void SymbolicStaticAnalyzer::processAssignment(NStatement *currentStatement) {
    NVariableDeclaration *nAssignment = dynamic_cast<NVariableDeclaration *>(currentStatement);

    if (nAssignment != 0) {
        processAssignment(nAssignment);
        return;
    }

    NReturnStatement *nReturnStatement = dynamic_cast<NReturnStatement *>(currentStatement);

    if (nReturnStatement != 0) {
        string name = nReturnStatement->variable.name;
        string field = nReturnStatement->variable.field;
        currentFunction->addOutput(name, field);
        return;
    }

    NExpressionStatement *nExpressionStatement = dynamic_cast<NExpressionStatement *>(currentStatement);

    // here it could be only method call
    // TODO: change parser to avoid addition convention
    if(nExpressionStatement !=0 ) {
        NMethodCall *nMethodCall = dynamic_cast<NMethodCall *>(&(nExpressionStatement->expression));

        // call without return
        if(nMethodCall !=0 ) {
            processAssignment(nMethodCall);
            return;
        }
    }

    throw WrongFunctionStatement();
}

void SymbolicStaticAnalyzer::processAssignment(NVariableDeclaration *nAssignment) {
    string name = nAssignment->id.name;
    string field = nAssignment->id.field;
    string nameWithField = currentFunction->getVariableName(&(nAssignment->id));
    // dedicated case if input variable is a structure and having assignment to its field
    if (currentFunction->checkIfIsInput(&(nAssignment->id)) && field != "") {
        currentFunction->addOutput(name, field);
    }

    NMethodCall *nMethodCall = dynamic_cast<NMethodCall *>(nAssignment->assignmentExpr);

    if (nMethodCall != 0) {
     //   processAssignment(nMethodCall);

    } else {
        Assignment *value = currentFunction->evaluateAssignment(nAssignment->assignmentExpr);
        currentFunction->addVariable(nameWithField, value);
    }
    /* TODO: case of
    *  X.Y = Z
    *  W.S = V
    *  X = W
    * when X is input variable should be accurately checked
    */
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
}

Assignment *FunctionDeclaration::evaluateAssignment(NIdentifier *currentIdentifier) {
    // simple case: we have identifier in rhs
    string field = currentIdentifier->field;
    string name = currentIdentifier->name;
    string fullName = getVariableName(currentIdentifier);
    if (variables.count(fullName) == 0) {
        if(checkIfIsInput(currentIdentifier) && field != "") {
            // if name is input and structure, init as input
            variables.emplace(fullName, new InputVariable(fullName));
        } else {
            // we are here if we evaluating for function invocation and currentIdentifier is an input
            return new InputVariable(fullName);
        }
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

    NInteger *currentInt = dynamic_cast<NInteger *>(currentExpression);

    if (currentInt != 0) {
        return new IntegerAssignment(currentInt->value);
    }

    return nullptr;
}

string FunctionDeclaration::getVariableName(NIdentifier *currentIdentifier) {
    return getVariableName(make_pair(currentIdentifier->name, currentIdentifier->field));
}

string FunctionDeclaration::getVariableName(pair<string,string> output) {
    string name = output.first;
    string field = output.second;

    if (field != "") {
        return name + "." + field;
    } else {
        return name;
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

void FunctionDeclaration::addOutput(const string &name, const string &field) {
    // TODO: make outputs a map
    outputs.push_back(make_pair(name, field));
}

void FunctionDeclaration::addVariable(string name, Assignment *value) {
    variables[name] = value;
}

void FunctionDeclaration::substitute(FunctionDeclaration *function, ExpressionList arguments) {
    for(auto i : function->getOutput()) {
        pair<string, Assignment*> substitution = function->evaluateFunction(i, arguments);
        variables[substitution.first] = substitution.second;
    }
}

pair<string, Assignment *> FunctionDeclaration::evaluateFunction(const pair<string, string> output,
                                                                 ExpressionList arguments) {
    string name = output.first;
    string nameWithField = getVariableName(output);
    Assignment *outputValue = variables[nameWithField];
    map<string, NExpression *> mapOfInputs = mapInputs(arguments);

    // input value
    if(outputValue->getId() == 0) {
        InputVariable *inputVariable = dynamic_cast<InputVariable*>(outputValue);
        Assignment *newValue = evaluateAssignment(mapOfInputs[inputVariable->getName()]);

        NIdentifier *newName = dynamic_cast<NIdentifier *>(mapOfInputs[name]);
        if (newName != 0) {
            if(newName->name != name) {
                nameWithField = newName->name + "." + output.second;
            }
        } else {
            throw InputIsNotAField();
        }

        return make_pair(nameWithField, newValue);
    }
    if (outputValue->getId() == 2) {
    }

}

map<string, NExpression *> FunctionDeclaration::mapInputs(ExpressionList arguments) {
    map<string, NExpression *> answer;
    for(int i = 0; i < arguments.size(); i++) {
        answer[inputs[i]] = arguments[i];
    }
    return answer;
}

