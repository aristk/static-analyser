
typedef pair<string, string> localVariableName;


class Transition {
    // type of Transition
    int type;
public:
    Transition(int type) : type(type) {}

    const int getType() const {
        return type;
    }
    virtual ~Transition() {}
};

// a = x
class VariableTransition : public Transition {
    localVariableName lhs;
    localVariableName rhs;
public:
    VariableTransition(const localVariableName &lhs, const localVariableName &rhs) : Transition(2), lhs(lhs), rhs(rhs) {}
};

class IntegerTransition : public Transition {
    localVariableName lhs;
    int rhs;
public:
    IntegerTransition(const localVariableName &lhs, const int rhs) : Transition(3), lhs(lhs), rhs(rhs) {}
};

// a = x == y
// a = x != y
class EquationTransition : public Transition {
    localVariableName lhs;
    localVariableName rhs1;
    localVariableName rhs2;
public:
    EquationTransition(const localVariableName &lhs, const localVariableName &rhs1, const localVariableName &rhs2,
    const int &type): Transition(type), lhs(lhs), rhs1(rhs1), rhs2(rhs2) { }
};
