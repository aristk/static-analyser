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

class WrongFunctionArgument: public exception
{
    virtual const char* what() const throw()
    {
        return "Argument of the function is not single variable.";
    }
};
