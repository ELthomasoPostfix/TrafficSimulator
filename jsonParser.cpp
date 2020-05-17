//
// Created by Thomas Gueutal on 11.05.20.
//

#include "jsonParser.h"


Network* jsonParser::processJSON(const std::string& fileName) {
    std::fstream inputFile(fileName);
    inputFile.open(fileName);
    nlohmann::json json = nullptr;

    // deserialize the json file
    inputFile >> json;

    // initialise the network obj
    Network* cityobj = jsonToSimulationParameters(json);

    for (std::string symbolStr: json["alphabet"]) {
        cityobj->addAllowedStreetType(Street::nameToType(symbolStr[0]));
    }

    // add intersections/states to the network
    jsonToIntersections(json, cityobj);

    // add all streets/transitions to the states of the network
    jsonToStreets(json, cityobj);

    // add all vehicles to the network
    jsonToVehicles(json, cityobj);

    return cityobj;
}


Network *jsonParser::jsonToSimulationParameters(nlohmann::json &json) {
    const double minCarDistance = json["simParameters"]["mincardistance"];
    const double maxTrafficLightCount = json["simParameters"]["maxtrafficlightcount"];
    const double decisionBufferLen = json["simParameters"]["decisionbufferlength"];
    return new Network();
}


void jsonParser::jsonToIntersections(nlohmann::json& json, Network* cityNetwork) {

    for (auto& intersection : json["intersections"]) {

        Intersection* newIntersection = new Intersection(intersection["name"]);

        // add traffic lights if necessary
        if (intersection["trafficlights"]) {
            // get the base STOP signal for traffic signals
            newIntersection->setTrafficLights(cityNetwork->getSimulation()->getInfluence(STOP));
            cityNetwork->addInfluencingIntersection(newIntersection);
            cityNetwork->addIntersection(newIntersection);
        } else {
            cityNetwork->addIntersection(newIntersection);
        }
    }
}

void jsonParser::jsonToStreets(nlohmann::json& json, Network* cityNetwork) {

    for (auto& street : json["streets"]) {

        const std::string& typeName = street["type"];
        const streetType type = Street::nameToType(*typeName.c_str());
        // only construct streets whose type is allowed
        if (typeIsAllowed(type, cityNetwork)) {
            // create the street
            Street *newStreet = new Street(cityNetwork->findIntersection(street["previousintersection"]),
                                           cityNetwork->findIntersection(street["nextintersection"]),
                                           type);
            newStreet->setIsTwoWay(street["twowaystreet"]);

            // add a limit influence if needed
            const int limit = street["limit"];
            if (limit > 0) {
                Influence *newLimit = new Influence(LIMIT);
                newLimit->setArgument(limit);
                newStreet->addInfluence(newLimit);
            }

            // assign the street to the two states it's attached to
            newStreet->getNextIntersection()->addStreet(newStreet);
            newStreet->getPrevIntersection()->addStreet(newStreet);
        }
    }
}

void jsonParser::jsonToVehicles(nlohmann::json &json, Network *cityNetwork) {
    for (auto& vehicle : json["vehicles"]) {
        std::string type = vehicle["type"];

        // create the vehicle
        Vehicle* newVehicle = createVehicle(type, *cityNetwork->getSimulation());
        // add a start and end intersection
        int startIndex, endIndex;
        do {
            startIndex = random() % cityNetwork->getNetwork().size();
            endIndex = random() % cityNetwork->getNetwork().size();
        } while (startIndex == endIndex);

        newVehicle->setStartIntersection(cityNetwork->getNetwork()[startIndex]);
        newVehicle->setEndIntersection(cityNetwork->getNetwork()[endIndex]);

        newVehicle->setSpeed(vehicle["speed"]);

        // add a STOP influence if needed
        if (type == "special" and vehicle["influence"]) {
            Influence* STOPsignal = new Influence(STOP);
            // -1 as an argument for a STOP signal will represent that the signal affects the entire street the vehicle
            // is currently in
            STOPsignal->setArgument(-1);
            newVehicle->addOutgoingInfluence(STOPsignal);
        }

        // TODO assign:
        //  - starting path
        //  - currentStreet
        //  - prev intersection
        //  - next intersection
        //  - isUnderWay
        //  - progress

        cityNetwork->addVehicle(newVehicle);
        cityNetwork->getSimulation()->incrementTotalSpawnedVehicles();
    }

}


Vehicle *jsonParser::createVehicle(const std::string &vehicleClass, const Simulation& sim) {
    // create the license plate
    std::stringstream ss;
    ss << sim.getTotalSpawnedVehicles() << rand() % 9 + 1 << rand() % 9 + 1;
    std::string licensePlate = ss.str();

    if (vehicleClass == "personal") {
        return new Vehicle(personal, licensePlate);
    } else if (vehicleClass == "transport") {
        return new TransportVehicle(licensePlate);
    } else if (vehicleClass == "special") {
        return new SpecialVehicle(licensePlate);
    } else {
        return nullptr;
    }
}

bool jsonParser::typeIsAllowed(const streetType& streetType, const Network* cityNetwork) {
    for (const enum streetType& allowedType : cityNetwork->getAllowedStreetTypes()) {
        if (allowedType == streetType) return true;
    }
    return false;
}

