//
// Created by arist on 19/11/2016.
//

#include "SatStaticAnalyzer.hpp"

void SatStaticAnalyzer::updateAnswers(const string &opName, FullVariableName &keyLhs, const NIdentifier &lhs) {
    // skip not top level checks
    if(isTopLevelCall())
        return;

    FullVariableNameOccurrence key = getFullVariableNameOccurrence(keyLhs);
    variablesToCheck.push_back(make_tuple(key, opName, lhs.lineNumber));
}

vector<pair<int, unsigned int> > SatStaticAnalyzer::getAnswers() {
    vector<pair<int, unsigned int> > answers;
    int returnValue;
    unsigned int lineNumber;
    string opName;

    lbool result = getSolver()->solve();
    assert(result == l_True);

    for (auto i : variablesToCheck) {
        FullVariableNameOccurrence key = get<0>(i);
        opName = get<1>(i);
        lineNumber = get<2>(i);
        if (isConstant(returnValue, key)) {
            cout << key << " from " << opName << " is " << returnValue <<
                 " at line " << lineNumber << endl;
            answers.push_back(make_pair(returnValue, lineNumber));
        }
    }
    return answers;
}
