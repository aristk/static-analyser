//
// Created by arist on 19/11/2016.
//

#include "incSatAnalyzer.hpp"

#ifndef STATICANALYZERROOT_SATSTATICANALYZER_HPP
#define STATICANALYZERROOT_SATSTATICANALYZER_HPP

// TODO(arist): inheritance should be reverse
class SatStaticAnalyzer : public IncrementalSatStaticAnalyzer {
    // key + operator name + position in the file
    vector<tuple<FullVariableNameOccurrence, string, int> > variablesToCheck;
public:
    SatStaticAnalyzer(): IncrementalSatStaticAnalyzer(), variablesToCheck() { }

    void updateAnswers(const string &opName, FullVariableName &keyLhs, const NIdentifier &lhs) override;

    vector<pair<int, unsigned int> > getAnswers() override;
};
#endif //STATICANALYZERROOT_SATSTATICANALYZER_HPP
