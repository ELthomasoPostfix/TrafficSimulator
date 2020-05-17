//
// Created by Thomas Gueutal on 11.05.20.
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

    Network* jsonToSimulationParameters(nlohmann::json& json);

    void jsonToIntersections(nlohmann::json& json, Network* cityNetwork);

    void addTrafficLightPairs(nlohmann::json& json, Network* cityNetwork);

    void jsonToStreets(nlohmann::json& json, Network* cityNetwork);

    void jsonToVehicles(nlohmann::json& json, Network* cityNetwork);

    Vehicle* createVehicle(const std::string & vehicleClass, const Simulation& sim);

    bool typeIsAllowed(const streetType& streetType, const Network* cityNetwork);
};


#endif //TRAFFICSIMULATOR_JSONPARSER_H
