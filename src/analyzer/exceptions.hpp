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

class isParserCrashed: public exception
{
public:
    virtual const char* what() const throw()
    {
        return "Result of parsing is NULL.";
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

class SatVariableAreAlreadyDefinened: public exception {
    virtual const char* what() const throw() {
        return "Current variable was already used.";
    }
};

class functionIsNotImplemented: public exception {
    string output;
    virtual const char* what() const throw() {
        return output.c_str();
    }
public:
    functionIsNotImplemented(const string& name, const string& input):
        output(name + " method for \"" + input + "\" is not implemented")
    { }

};

class couldNotOpenFile: public exception {
    string output;

    virtual const char *what() const throw() {
        return output.c_str();
    }

public:
    couldNotOpenFile(const char *fileName) : output("couldn’t open ") {
        output.append(fileName);
    }
};

class isAlreadyAnInput: public exception {
    string output;
    virtual const char* what() const throw() {
        return output.c_str();
    }
public:
    isAlreadyAnInput(const string& input) :
            output("Variable \"" + input + "\" already defined as input") { }
};

class isWrongModel: public exception {
    virtual const char* what() const throw() {
        return "Model could not be unsat.";
    }
};
