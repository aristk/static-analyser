#include <string>
#include <iostream>

using namespace std;

// function name, name, field
typedef tuple<string, string, string> FullVariableName;

typedef pair<FullVariableName, unsigned int> FullVariableNameOccurrence;

ostream& operator<<(ostream& os, const FullVariableName& obj);

ostream& operator<<(ostream& os, const FullVariableNameOccurrence& obj);

string getVariableName(const FullVariableName &obj);
string getVariableField(const FullVariableName &obj);

string getVariableName(const FullVariableNameOccurrence &obj);
string getVariableField(const FullVariableNameOccurrence &obj);