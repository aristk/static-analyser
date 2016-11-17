//
// Created by arist on 17/11/2016.
//
#include "variableName.hpp"

std::ostream& operator<<(std::ostream& os, const FullVariableName& obj) {
    if (obj.second != "") {
        os << obj.first << "." << obj.second;
        return os;
    }
    else {
        return os << obj.first;
    }
}

std::ostream& operator<<(std::ostream& os, const FullVariableNameOccurrence& obj) {
    os << obj.first << "(" << obj.second << ")";
    return os;
}