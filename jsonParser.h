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

    void assignTrafficLightPairs(nlohmann::json& json, Network* cityNetwork);

    void jsonToStreets(nlohmann::json& json, Network* cityNetwork);

    void jsonToVehicles(nlohmann::json& json, Network* cityNetwork);
    void addOugoingInfluences(Network *cityNetwork, Vehicle *newVehicle, const std::string& type, bool hasInfluence);
    void setStartAndEnd(Network *cityNetwork, Vehicle *newVehicle, Intersection* startIntersection,
                        const std::string& endIntersectionName);
    void setMembers(Intersection* startIntersection, Intersection* otherIntersection, Street* spawnStreet,
                    Vehicle* newVehicle);
    bool addToStreet(Vehicle* newVehicle, Intersection* otherIntersection, Street* spawnStreet, double progress, int spawnLane);
    void printLaneError(Vehicle* newVehicle, Street* spawnStreet, Intersection* otherIntersection);

    Vehicle* createVehicle(const std::string & vehicleClass, Simulation& sim);

    bool typeIsAllowed(const streetType& streetType, const Network* cityNetwork);
};


#endif //TRAFFICSIMULATOR_JSONPARSER_H
