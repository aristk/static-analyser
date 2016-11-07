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

class InputIsNotAField: public exception {
    virtual const char* what() const throw() {
        return "Integer input argument is used as a struct.";
    }
};

class genCheckNotImplemented: public exception {
    virtual const char* what() const throw() {
        return "genCheck method is not implemented";
    }
};
