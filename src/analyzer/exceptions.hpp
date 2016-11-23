using namespace std;

class notNFunctionDeclaration: public exception
{
    const char* what() const throw() override
    {
        return "Root item is not NFunctionDeclaration.";
    }
};

class NotNVariableDeclaration: public exception
{
    const char* what() const throw() override
    {
        return "Argument of the function is not NVariableDeclaration.";
    }
};

class WrongBinaryOperator: public exception {
    const char* what() const throw() override
    {
        return "Only == and != could be used.";
    }
};

class WrongFunctionStatement: public exception {
    const char* what() const throw() override
    {
        return "In function we could use only variable assignment or procedure call";
    }
};

class WrongFunctionArgument: public exception {
    const char* what() const throw() override
    {
        return "Argument of the function is not single variable.";
    }
};

class FunctionIsNotDefined: public exception {
    const char* what() const throw() override
    {
        return "Function is not defined.";
    }
};

class FunctionDefinedTwice: public exception {
    string output;
    const char* what() const throw() override {
        return output.c_str();
    }
public:
    explicit FunctionDefinedTwice(const string& name):
            output("Function " + name + " defined two time.")
    { }

};

class InputIsAStruct: public exception {
    const char* what() const throw() override {
        return "Integer input argument is used as a struct.";
    }
};

class SatVariableIsNotDefined: public exception {
    const char* what() const throw() override {
        return "Current variable was not used yet.";
    }
};

class SatVariableAreAlreadyDefined: public exception {
    const char* what() const throw() override {
        return "Current variable was already used.";
    }
};

class differentNumberOfArgsInFunctionCall: public exception {
    const char* what() const throw() override {
        return "Call of function has different number of arguments with respect to declaration.";
    }
};


class recursiveCall: public exception {
    string output;
    const char* what() const throw() override {
        return output.c_str();
    }
public:
    explicit recursiveCall(const string& name):
            output("Function " + name + "called recursively")
    { }

};

class functionIsNotImplemented: public exception {
    string output;
    const char* what() const throw() override {
        return output.c_str();
    }
public:
    functionIsNotImplemented(const string& name, const string& input):
        output(name + R"( method for ")" + input + R"(" is not implemented)")
    { }

};

class couldNotOpenFile: public exception {
    string output;

    const char *what() const throw() override {
        return output.c_str();
    }

public:
    explicit couldNotOpenFile(const char *fileName) : output("couldn’t open ") {
        output.append(fileName);
    }
};

class isAlreadyAnInput: public exception {
    string output;
    const char* what() const throw () override {
        return output.c_str();
    }
public:
    explicit isAlreadyAnInput(const string& input) :
            output(R"(Variable ")" + input + R"(" already defined as input)") { }
};


class parserError: public exception {
    string output;
    const char* what() const throw() override {
        return output.c_str();
    }
public:
    explicit parserError(const string& input) :
            output("Parser error: " + input) { }
};

class isWrongModel: public exception {
    const char* what() const throw() override {
        return "Model could not be unsat.";
    }
};
