//
// Created by arist on 17/11/2016.
//
#include "variableName.hpp"

std::ostream& operator<<(std::ostream& os, const FullVariableName& obj) {
    os << get<0>(obj) << ".";
    string field = get<2>(obj);
    string name = getVariableName(obj);
    if (field != "") {
        os << name << "." << field;
        return os;
    }
    else {
        return os << name;
    }
}

std::ostream& operator<<(std::ostream& os, const FullVariableNameOccurrence& obj) {
    os << obj.first << "(" << obj.second << ")";
    return os;
}

string getVariableName(const FullVariableName& obj) {
    return get<1>(obj);
}

string getVariableName(const FullVariableNameOccurrence& obj) {
    return getVariableName(obj.first);
}