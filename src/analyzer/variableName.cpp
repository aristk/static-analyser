//
// Created by arist on 17/11/2016.
//
#include "variableName.hpp"

std::ostream& operator<<(std::ostream& os, const FullVariableName& obj) {
    os << get<0>(obj) << ".";
    string field = getVariableField(obj);
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
    os << obj.first;
    os << "(" << obj.second << ")";
    return os;
}

string getVariableName(const FullVariableName &obj) {
    return get<1>(obj);
}

string getVariableName(const FullVariableNameOccurrence &obj) {
    return getVariableName(obj.first);
}

string getVariableField(const FullVariableName &obj) {
    return get<2>(obj);
}

string getVariableField(const FullVariableNameOccurrence &obj) {
    return getVariableField(obj.first);
}