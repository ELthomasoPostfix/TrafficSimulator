//
// Created by elthomaso on 11.05.20.
//

#ifndef TRAFFICSIMULATOR_JSONPARSER_H
#define TRAFFICSIMULATOR_JSONPARSER_H

#include "vector"
#include "string"

#include "json.hpp"
#include "Network.h"


class jsonParser {

public:

    Network* processJSON(const std::string& fileName);

    void jsonToStates(nlohmann::json& json, Network* cityNetwork);

    void jsonToTransitions(nlohmann::json& json, Network* cityNetwork);

    bool charInAlphabet(const char& c, const std::vector<char>& alphabet);
};


#endif //TRAFFICSIMULATOR_JSONPARSER_H
