//
// Created by Thomas Gueutal on 11.05.20.
//

#include "jsonParser.h"

// base definitions of extern variables in Simulation

// the amount of distance subsequent vehicles MUST keep between each other
int minCarDistance = 1;
// the amount of time it takes for traffic lights to instantly cycle to a new pair of streets
int trafficLightMaxCount = 5;

int decisionBufferLength = 10;
// the distance from a STOP signal at which the STOP signal will take effect on the vehicle
int effectiveSTOPdistance = minCarDistance;

int streetLength = 100;



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
    auto& simParams = json["simParameters"];
    if (simParams["redefine"]) {
        auto &simVars = simParams["simVars"];
        Simulation::setMinCarDistance(simVars["mincardistance"]);
        Simulation::setTrafficLightMaxCount(simVars["maxtrafficlightcount"]);
        Simulation::setDecisionBufferLength(simVars["decisionbufferlength"]);
        Simulation::setStreetLength(simVars["streetlength"]);
    }
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

        newVehicle->setSpeed(vehicle["speed"]);

        addOugoingInfluences(cityNetwork, newVehicle, type, vehicle["influence"])    ;

        // assign variables related to the start intersection and start the Street
        std::vector<std::string> startStreet = vehicle["startStreet"];
        const bool twoWay = vehicle["startStreetTwoWay"];
        // TODO: (**)    Intersection* startIntersection = newVehicle->getStartIntersection();
        Intersection* startIntersection = cityNetwork->findIntersection(startStreet[0]);
        newVehicle->setStartIntersection(startIntersection); // TODO: (**)
        Intersection* otherIntersection = cityNetwork->findIntersection(startStreet[1]);
        std::string typeStr = startStreet[2];
        streetType streetType = Street::nameToType(*typeStr.c_str());

        Street* spawnStreet = startIntersection->findStreet(startIntersection, otherIntersection, streetType, twoWay);
        int spawnLane = startIntersection->laneIndexWhenLeaving(spawnStreet);
        // two intersections for the start street are in the wrong order,
        // the spawn street is a one way street, and start intersection, the one to set as a previous intersection,
        // is the next intersection of the street instead
        // ==> switch them
        if (spawnLane == -1) {
            Intersection* temp = startIntersection;
            startIntersection = otherIntersection;
            otherIntersection = temp;
            spawnLane = startIntersection->laneIndexWhenLeaving(spawnStreet);
        }

        setStartAndEnd(cityNetwork, newVehicle, startIntersection, vehicle["endIntersection"]);

        // still bad result, something went wrong
        if (spawnLane == -1) {
            printLaneError(newVehicle, spawnStreet, otherIntersection);
            delete newVehicle;
            continue;
        }

        setMembers(startIntersection, otherIntersection, spawnStreet, newVehicle);

        const double progress = vehicle["progress"];
        // actually put the vehicle into the street (assign it to a lane and set it as the front)
        if (!addToStreet(newVehicle, otherIntersection, spawnStreet, progress, spawnLane)) {
            delete newVehicle;
        }

        // TODO assign:
        //  - starting path


        cityNetwork->addVehicle(newVehicle);
    }
}
void jsonParser::addOugoingInfluences(Network *cityNetwork, Vehicle *newVehicle, const std::string& type, bool hasInfluence) {
    // add a STOP influence if needed
    if (type == "special" and hasInfluence) {
        Influence* STOPsignal = new Influence(STOP);
        // -2 as an argument for a STOP signal will represent that the signal affects the entire street the vehicle
        // is currently in
        STOPsignal->setArgument(-2);
        newVehicle->addOutgoingInfluence(STOPsignal);
    }
}
void jsonParser::setStartAndEnd(Network *cityNetwork, Vehicle *newVehicle, Intersection* startIntersection,
                                const std::string& endIntersectionName) {
    // assign the start and end intersections
    newVehicle->setStartIntersection(startIntersection);
    newVehicle->setEndIntersection(cityNetwork->findIntersection(endIntersectionName));

}
void jsonParser::setMembers(Intersection *startIntersection, Intersection *otherIntersection, Street* spawnStreet,
                            Vehicle* newVehicle) {
    // assign more members
    newVehicle->setPrevIntersection(startIntersection);
    newVehicle->setNextIntersection(otherIntersection);
    newVehicle->setUnderway(true);  // TODO unnecessary member variable ?????

    // set the start street
    newVehicle->setCurrentStreet(spawnStreet);

}
bool jsonParser::addToStreet(Vehicle *newVehicle, Intersection *otherIntersection, Street* spawnStreet, double progress,
                             int spawnLane) {
    Vehicle* prevFrontOccupant = spawnStreet->getFrontOccupant(spawnLane);
    Vehicle* prevBackOccupant  = spawnStreet->getBackOccupant(spawnLane);
    // TODO what if frontOccupant == BackOccupant
    if (prevFrontOccupant != nullptr) {
        // the new vehicle is the front most vehicle in its spawn street
        // the new vehicle is not initiated directly in front of the traffic lights
        if (prevFrontOccupant->getProgress() < progress and progress < Simulation::getStreetLength()) {
            spawnStreet->setFrontOccupant(newVehicle, spawnLane);

            newVehicle->setPrevVehicle(prevFrontOccupant);
            prevFrontOccupant->setNextVehicle(newVehicle);
            // vehicle is all the way in front, progress is valid


        // the new vehicle is either the back most vehicle in its spawn street or in the in between.
        // either way it needs to become the back of the queue
        // if it is in the in between, then other cars will pass it and it will wait for its next, the previous back, to pass
        } else if (progress >= 0) {
            spawnStreet->setBackOccupant(newVehicle, spawnLane);
            newVehicle->setNextVehicle(prevBackOccupant);

            prevBackOccupant->setPrevVehicle(newVehicle);

            // vehicle is always attached to the back. if it was initiated "in the middle of a file", then treat the vehicle
            // as being parked on the side of the road. If its next has a lower progress, then the vehicle should be
            // attempting to drive backwards until its progress is lower than its next. Its own previous should then,
            // upon reaching newVehicle, wait until the newVehicle begins driving
        } else {
            if (progress < 0) {
                std::cerr << "The vehicle " << newVehicle->getLicensePlate() << " was to be instantiated with a progress of "
                          << newVehicle->getProgress() << ", which is not allowed. Vehicle will not be instantiated." << std::endl;
            } else {
                std::cerr << "Something went wrong when adding the vehicle " << newVehicle->getLicensePlate() << " to the street from "
                          << prevBackOccupant->getPrevIntersection()->getName() << " to " << prevBackOccupant->getPrevIntersection()->getName()
                          << "(" << prevBackOccupant->getCurrentStreet()->typeToName() << ", "
                          << prevBackOccupant->getCurrentStreet()->getTwoWayString() << "). Vehicle will not be instantiated";
            }
            return false;
        }
    // the front and thus also the back are nullptr, assign the new front and back
    } else {
        spawnStreet->setFrontOccupant(newVehicle, spawnLane);
    }
    newVehicle->setProgress(progress);
    spawnStreet->requestInfluences(newVehicle); // TODO just added
    otherIntersection->requestSignal(newVehicle);
    return true;
}
void jsonParser::printLaneError(Vehicle *newVehicle, Street *spawnStreet, Intersection *otherIntersection) {
    std::cerr << "Error while parsing json: The vehicle (" << newVehicle->classToName() << ") with start intersection "
              << newVehicle->getStartIntersection()->getName() << " and end intersection "
              << newVehicle->getEndIntersection()->getName() << " wanted to start in the street from "
              << newVehicle->getStartIntersection()->getName() << " to " << otherIntersection->getName();
    if (spawnStreet != nullptr) {
        std::cerr
                << " (" << spawnStreet->typeToName()
                << ")\nbut getting put into the lane happened from an invalid intersection."
                << " This is possibly because the street is a one way street and the start and other intersection are the wrong way around."
                << std::endl;
    } else {
        std::cerr << " but the spawn street was not found within the network." << std::endl;
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





