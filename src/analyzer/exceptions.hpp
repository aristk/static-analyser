using namespace std;

class notNFunctionDeclaration: public exception
{
    virtual const char* what() const throw()
    {
        return "Root item is not NFunctionDeclaration.";
    }
};

class NotNVariableDeclaration: public exception
{
    virtual const char* what() const throw()
    {
        return "Argument of the function is not NVariableDeclaration.";
    }
};

class WrongBinaryOperator: public exception {
    virtual const char* what() const throw()
    {
        return "Only == and != could be used.";
    }
};

class WrongFunctionStatement: public exception {
    virtual const char* what() const throw()
    {
        return "In function we could use only variable assignment or procedure call";
    }
};

class WrongFunctionArgument: public exception {
    virtual const char* what() const throw()
    {
        return "Argument of the function is not single variable.";
    }
};

class FunctionIsNotDefined: public exception {
    virtual const char* what() const throw()
    {
        return "Function is not defined.";
    }
};

class InputIsAStruct: public exception {
    virtual const char* what() const throw() {
        return "Integer input argument is used as a struct.";
    }
};

class SatVariableIsNotDefinened: public exception {
    virtual const char* what() const throw() {
        return "Current variable was not used yet.";
    }
};

class genCheckNotImplemented: public exception {
    virtual const char* what() const throw() {
        return "genCheck method is not implemented";
    }
};

class isAlreadyAnInput: public exception {
    string name;
    virtual const char* what() const throw() {
        return name.c_str();
    }
public:
    isAlreadyAnInput(const string& input) {
        name = "Variable \"" + input + "\" already defined as input";
    }
};

class addClausesNotImplemented: public exception {
    virtual const char* what() const throw() {
        return "addClauses method is not implemented";
    }
};
