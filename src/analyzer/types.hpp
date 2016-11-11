class LanguageType {
    int id;
public:
    LanguageType(int id) : id(id) {}
    virtual ~LanguageType() {}
};

class IntegerType : public LanguageType {
    int value;
public:
    IntegerType(const int value) : LanguageType(0), value(value) { }
};

class VariableType : public LanguageType {
    string name;
    string field;
public:
    VariableType(const string &name, const string &field) : LanguageType(1), name(name), field(field) { }
};