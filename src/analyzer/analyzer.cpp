#include "analyzer.hpp"
#include "exceptions.hpp"

// TODO: import does not work here... Find a better way, as example, modifying the parser
#define TCEQ 260
#define TCNE 261

using namespace std;

NaiveStaticAnalyzer::NaiveStaticAnalyzer(NBlock *programBlock) {

    // TODO: implement better verbosity usage
    cout << "We are in NaiveStaticAnalyzer constructor" << endl;
    for(auto i : programBlock->statements) {
        // check that all root items are NFunctionDeclaration
        // TODO: usage of dynamic_cast is not good idea
        NFunctionDeclaration *NFunction = dynamic_cast<NFunctionDeclaration*>(i);
        if (NFunction == 0) {
            throw notNFunctionDeclaration();
        }

        // TODO: look like better way is to all follows in construct of FunctionInLanguage
        this->functions.push_back(new FunctionInLanguage(NFunction->id.name));
        FunctionInLanguage *currentFunction = this->functions.back();

        // add input variable
        for(auto j : NFunction->arguments) {
            // note that all inputs are of NVariableDeclaration type by parsing
            // TODO: add assertion with check that j is of NVariableDeclaration type
            currentFunction->addInput(j);
        }
        currentFunction->processBody(NFunction->block);
    }
}

void FunctionInLanguage::addInput(NVariableDeclaration* NVariable) {
    if (NVariable->id.field != "" || NVariable->assignmentExpr != NULL) {
        throw WrongFunctionArgument();
    }
    string name = NVariable->id.name;
    this->inputs.push_back(name);
    this->variables.emplace(name, new InputVariable(name));
}

void FunctionInLanguage::processBody(NBlock &block) {
    for(auto i : block.statements) {
        // could be NVariableDeclaration or NExpressionStatement with NMethodCall
        this->processAssignment(i);
    }
}

FunctionInLanguage::~FunctionInLanguage() {
    // TODO: use shared_ptr for deleting Assignments
}

void FunctionInLanguage::processAssignment(NVariableDeclaration *nAssignment) {
    string name = nAssignment->id.name;
    string field = nAssignment->id.field;
    string nameWithField = getVariableName(&(nAssignment->id));
    // dedicated case if input variable is a structure and having assignment to its field
    if (checkIfIsInput(&(nAssignment->id)) && field != "") {
        // TODO: make outputs a map
        this->outputs.push_back(nameWithField);
    }
    variables[nameWithField] = evaluateAssignment(nAssignment->assignmentExpr);
    /* TODO: case of
    *  X.Y = Z
    *  W.S = V
    *  X = W
    * when X is input variable should be accurately checked
    */
}

void FunctionInLanguage::processAssignment(NStatement *currentStatement) {
    // TODO: get rid of dynamic_cast, as alternative call processAssignment from NStatement
    NVariableDeclaration *nAssignment = dynamic_cast<NVariableDeclaration *>(currentStatement);

    if (nAssignment != 0) {
        processAssignment(nAssignment);
        return;
    }


    NReturnStatement *nReturnStatement = dynamic_cast<NReturnStatement *>(currentStatement);

    if (nReturnStatement != 0) {
        string nameWithField = getVariableName(&(nReturnStatement->variable));
        this->outputs.push_back(nameWithField);
        return;
    }

    NExpressionStatement *nExpressionStatement = dynamic_cast<NExpressionStatement *>(currentStatement);

    // here it could be only method call
    // TODO: change parser to avoid addition convention
    if(nExpressionStatement !=0 ) {
        NMethodCall *nMethodCall = dynamic_cast<NMethodCall *>(&(nExpressionStatement->expression));

        // call without return
        if(nMethodCall !=0 ) {
            // TODO: for substitution we need access to all functions from StaticAnalyzer
        }
        return;
    }

    throw WrongFunctionStatement();
}

Assignment *FunctionInLanguage::evaluateAssignment(NIdentifier *currentIdentifier) {
    // simple case: we have identifier in rhs
    string field = currentIdentifier->field;
    string name = currentIdentifier->name;
    string fullName = getVariableName(currentIdentifier);
    // if name is input and structure, init as input
    if (variables.count(fullName) == 0 && checkIfIsInput(currentIdentifier) && field != "") {
        this->variables.emplace(fullName, new InputVariable(fullName));
    }
    return variables[fullName];
}

Assignment *FunctionInLanguage::evaluateAssignment(NExpression *currentExpression) {
    NIdentifier *currentIdentifier = dynamic_cast<NIdentifier *>(currentExpression);

    if (currentIdentifier != 0) {
        return evaluateAssignment(currentIdentifier);
    }

    NBinaryOperator *currentOp = dynamic_cast<NBinaryOperator *>(currentExpression);

    if (currentOp != 0) {
        Assignment *lhs = evaluateAssignment(&(currentOp->lhs));
        Assignment *rhs = evaluateAssignment(&(currentOp->rhs));
        if (lhs->isEqualTo(rhs)) {
            // TCEQ from parser.hpp is equivalent to ==
            // TODO: it is better way to check?
            if (currentOp->op == TCEQ) {
                return new IntegerAssignment(1);
            } else if (currentOp->op == TCNE){
                // case of !=
                return new IntegerAssignment(0);
            } else {
                throw WrongBinaryOperator();
            }
        } else {
            return new LogicAssignment(currentOp->op, lhs, rhs);
        }
    }

    return nullptr;
    // TODO: check other options
}

string FunctionInLanguage::getVariableName(NIdentifier *currentIdentifier) {
    if (currentIdentifier->field != "") {
        return currentIdentifier->name + "." + currentIdentifier->field;
    } else {
        return currentIdentifier->name;
    }

}

bool FunctionInLanguage::checkIfIsInput(NIdentifier *currentIdentifier) {
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
