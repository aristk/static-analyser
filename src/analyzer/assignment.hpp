#include <string>

using namespace std;

// TODO: parse to here from file or have a joint structure with node.h


class Assignment {
protected:
    int id;
public:
    Assignment(int i) : id(i) {}

    virtual int getId() {
        return id;
    };
    virtual ~Assignment() {}
};

// CLion bug: cannot rename as refactor action: cannot found occurrences
class InputVariable: public Assignment {
    string name;
public:
    InputVariable(string x) : Assignment(0), name(x) {}
};

class VariableAssignment: public Assignment {
public:
    VariableAssignment() : Assignment(1) {}
};

class FunctionCall: public Assignment {
public:
    FunctionCall() : Assignment(2) {}
};
