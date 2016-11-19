//
// Created by arist on 19/11/2016.
//

#include "incSatAnalyzer.hpp"

#ifndef STATICANALYZERROOT_SATSTATICANALYZER_HPP
#define STATICANALYZERROOT_SATSTATICANALYZER_HPP

// TODO: inheritance should be reverse
class SatStaticAnalyzer : public IncrementalSatStaticAnalyzer {
    // key + operator name + position in the file
    vector<tuple<FullVariableNameOccurrence, string, int> > variablesToCheck;
public:
    SatStaticAnalyzer(): IncrementalSatStaticAnalyzer(), variablesToCheck() { }

    virtual void updateAnswers(const string &opName, FullVariableName &keyLhs, const NIdentifier &lhs);

    virtual vector<pair<int, unsigned int> > getAnswers();
};
#endif //STATICANALYZERROOT_SATSTATICANALYZER_HPP
