#include <string>

using namespace std;

// TODO: parse to here from file or have a joint structure with node.h


class Assignment {
protected:
    int id;
public:
    Assignment(int i) : id(i) {}

    virtual bool isEqualTo(Assignment *rhs) {
        return false;
    }

    bool isEqualId(Assignment *rhs) {
        if (this->id == rhs->getId()) {
            return true;
        }
        return false;
    }

    virtual int getId() {
        return id;
    };
    virtual ~Assignment() {}
};

// CLion bug: cannot rename as refactor action: cannot found occurrences
class InputVariable: public Assignment {
    string name;
public:
    string getName() {
        return name;
    }
    InputVariable(string x) : Assignment(0), name(x) {}

    bool isEqualTo(InputVariable *rhs) {
        if (isEqualId(rhs) && this->name == rhs->getName()) {
            return true;
        }
        return false;
    }
};

class IntegerAssignment: public Assignment {
    int value;
public:
    IntegerAssignment(int value) : Assignment(1), value(value) {}

    int getValue() {
        return value;
    }

    bool isEqualTo(IntegerAssignment *rhs) {
        if (isEqualId(rhs) && this->value == rhs->getValue()) {
            return true;
        }
        return false;
    }
};

class LogicAssignment: public Assignment {
    int logicOperator;
    Assignment *lhs;
    Assignment *rhs;
public:
    LogicAssignment(int logicOperator, Assignment *lhs, Assignment *rhs) :
            Assignment(2), logicOperator(logicOperator), lhs(lhs), rhs(rhs) {}
};

class FunctionCall: public Assignment {
public:
    FunctionCall() : Assignment(3) {}
};
