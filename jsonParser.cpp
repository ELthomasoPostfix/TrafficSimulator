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

    // assign the traffic light pairs
    assignTrafficLightPairs(json, cityobj);

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
        }
        cityNetwork->addStreetlessIntersection(newIntersection);
    }
}

void jsonParser::assignTrafficLightPairs(nlohmann::json &json, Network *cityNetwork) {
    // recheck every intersection for traffic light pairs
    for (auto& intersection : json["intersections"]) {
        const std::string& name = intersection["name"]; // the checked intersection
        Intersection* intersectionPtr = cityNetwork->findIntersection(name); // its pointer

        for (auto& trafficLightPair : intersection["trafficLightPairs"]) {
            std::pair<Street*, Street*> TLPair;
            for (Street* street : intersectionPtr->getStreets()) {
                // if the street specified by the traffic light pair actually exists and is linked to intersection
                if (street->getPrevIntersection() == intersectionPtr or street->getNextIntersection() == intersectionPtr) {
                    const std::string& otherName1 = trafficLightPair["otherIntersection1"]; // street1 of the pair
                    const std::string& otherName2 = trafficLightPair["otherIntersection2"]; // street2 of the pair
                    const std::string type1 = trafficLightPair["type1"];
                    const std::string type2 = trafficLightPair["type2"];

                    Intersection* otherIntersec = street->getOtherIntersection(intersectionPtr);
                    // the second intersection that Street1 refers to also exists
                    // and is also part of the network
                    if (otherIntersec == cityNetwork->findIntersection(otherName1) and
                        street->getType() == Street::nameToType(*type1.c_str())) {
                        TLPair.first = street;
                    // the second intersection that Street2 refers to also exists
                    // and is also part of the network
                    } else if (otherIntersec == cityNetwork->findIntersection(otherName2) and
                               street->getType() == Street::nameToType(*type2.c_str())) {
                        TLPair.second = street;
                    }
                }
            }
            if (TLPair.first != nullptr and TLPair.second != nullptr) {
                intersectionPtr->addTrafficLightPair(TLPair);
                const std::pair<Street*,Street*>& currentPair = intersectionPtr->getCurrentPair();
                if (currentPair.first == NULL and currentPair.second == NULL) {
                    intersectionPtr->setCurrentPair(TLPair);
                }
            }
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
            newStreet->fillEmptyLanes();    // add the correct amount of empty lanes to the Street
            newStreet->fillEmptyEntrantLanes();
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
        // assign variables related to the start intersection and start the Street
        std::vector<std::string> startStreet = vehicle["startStreet"];

        Intersection* startIntersection = cityNetwork->findIntersection(startStreet[0]);
        Intersection* otherIntersection = cityNetwork->findIntersection(startStreet[1]);
        std::string typeStr = startStreet[2];
        streetType streetType = Street::nameToType(*typeStr.c_str());

        Street* spawnStreet = startIntersection->findStreet(startIntersection, otherIntersection, streetType);
        const int spawnLane = startIntersection->laneIndexWhenLeaving(spawnStreet);
        // two intersections for the start street are in the wrong order,
        // switch them
        if (spawnLane == -1) {
            Intersection* temp = startIntersection;
            startIntersection = otherIntersection;
            otherIntersection = temp;
        }
        // still bad result, something went wrong
        if (spawnLane == -1) {
            std::cerr << "Error while parsing json: The vehicle (" << newVehicle->classToName() << ") with start intersection "
                      << newVehicle->getStartIntersection()->getName() << " and end intersection "
                      << newVehicle->getEndIntersection()->getName() << " wanted to start in the street from "
                      << newVehicle->getStartIntersection()->getName() << " to " << otherIntersection->getName()
                      << " (" << spawnStreet->typeToName() << ")\nbut getting put into the lane happened from an invalid intersection."
                      << " This is possibly because the street is a one way street and the start and other intersection are the wrong way around." << std::endl;
            continue;
        }
        // assign the start and end intersections
        newVehicle->setStartIntersection(startIntersection);
        newVehicle->setEndIntersection(cityNetwork->findIntersection(vehicle["endIntersection"]));

        // assign more members
        newVehicle->setPrevIntersection(startIntersection);
        newVehicle->setNextIntersection(otherIntersection);
        newVehicle->setUnderway(true);  // TODO unnecessary member variable ?????
        const int progress = vehicle["progress"];
        newVehicle->setProgress(progress);

        // set the start street
        newVehicle->setCurrentStreet(spawnStreet);

        // actually put the vehicle into the street (assign it to a lane and set it as the front)
        Vehicle* prevFrontOccupant = spawnStreet->getFrontOccupant(spawnLane);
        Vehicle* prevBackOccupant  = spawnStreet->getBackOccupant(spawnLane);
        if (prevFrontOccupant != nullptr) {
            // the new vehicle is the front most vehicle in its spawn street
            if (prevFrontOccupant->getProgress() < progress) {
                spawnStreet->setFrontOccupant(newVehicle, spawnLane);
                newVehicle->setPrevVehicle(prevFrontOccupant);

                prevFrontOccupant->setNextVehicle(newVehicle);
                // the new vehicle is either the back most vehicle in its spawn street or in the in between.
                // either way it needs to become the back of the queue
                // if it is in the in between, then other cars will pass it and it will wait for its next, the previous back, to pass
            } else {
                spawnStreet->setBackOccupant(newVehicle, spawnLane);
                newVehicle->setNextVehicle(prevBackOccupant);

                prevBackOccupant->setPrevVehicle(newVehicle);
            }
        // the front and thus also the back are nullptr, assign the new front and back
        } else {
            spawnStreet->setFrontOccupant(newVehicle, spawnLane);
        }

        otherIntersection->requestSignal(newVehicle);

        // TODO assign:
        //  - starting path


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
