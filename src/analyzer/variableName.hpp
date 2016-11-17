// name, field
#include <string>
#include <iostream>

using namespace std;

typedef pair<string, string> FullVariableName;

typedef pair<FullVariableName, int> FullVariableNameOccurrence;

std::ostream& operator<<(std::ostream& os, const FullVariableName& obj);

std::ostream& operator<<(std::ostream& os, const FullVariableNameOccurrence& obj);