//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Network.h"

Network::Network() {
    _simulation = new Simulation();
}


void Network::doMainLoop(const int duration, std::string& ofName, std::string& ofName2, std::string& ofName3) {
    Simulation* sim = _simulation;
    std::ofstream driveStream;
    std::ofstream trafficLightStream;
    std::ofstream vehicleChainStream;
    driveStream.open(ofName);
    trafficLightStream.open(ofName2);
    vehicleChainStream.open(ofName3);

    for (int dur = 0; dur < duration; ++dur) {
        driveStream        << "---- " << dur << " ----" << std::endl;
        trafficLightStream << "---- " << dur << " ----" << std::endl;
        vehicleChainStream << "\n---- " << dur-1 << " ----\n" << std::endl;
        // let intersections with traffic lights cycle through the traffic light pairs (of Street*s) and send out
        // signals when cycling from one pair to the next.
        // vehicles will request traffic light influence upon calling the setFrontOccupant() function of a street
        for (Intersection* influencingIntersection : getInfluencingIntersections()) {
            influencingIntersection->emitInfluences(trafficLightStream);
        }

        for (Intersection* intersection : getNetwork()) {
            intersection->calculateTrafficScore();
        }

        writeAllVehicleChains(vehicleChainStream);
        // let all vehicles drive and if they can, send out signals
        for (Vehicle* vehicle : getVehicles()) {
            vehicle->drive(driveStream);
            std::cout << " drove (" << dur << ")" << std::endl;
        }
    }
    vehicleChainStream << "\n---- " << duration-1 << " ----" << std::endl;
    writeAllVehicleChains(vehicleChainStream);

    driveStream.close();
    trafficLightStream.close();
    vehicleChainStream.close();
    sim->addTotalTimeSimulated(duration);
}


void Network::toDot(std::ofstream &outputFile) const {

    if (outputFile.is_open()) {
        outputFile << "digraph {\nrankdir=LR\n";

        std::vector<Intersection *> allIntersections = getNetwork();

        for (Intersection *intersection : allIntersections) {
            outputFile << "\"" << intersection->getName() << "\"" << "\n";
            for (Street *street : intersection->getStreets()) {
                // a single street only has one previous intersection pointer, so even if two intersections
                // share a street, it will only be added to the file once
                if (street->getPrevIntersection() == intersection) {
                    std::string both;
                    if (street->isTwoWay()) both = " dir=\"both\"";
                    std::string extraLabel = street->getPrevIntersection()->getName() +
                                             Util::boolToArrowString(street->isTwoWay(), true) +
                                             street->getNextIntersection()->getName();
                    outputFile << "\"" << intersection->getName() << "\"" << "->" << "\""
                             << street->getNextIntersection()->getName() << "\""
                             << " [label=\"" << street->typeToName() << " " << extraLabel << " \"" << both << "]" << "\n";
                }
            }
        }
        outputFile << "}";
    } else {
        std::cerr << "You cannot write to a closed ofstream." << std::endl;
    }
}
void Network::toDotElim(std::ofstream &outputFile) const {

    if (outputFile.is_open()) {
        outputFile << "digraph {\nrankdir=LR\n";

        std::vector<Intersection *> allIntersections = getNetwork();

        for (Intersection *intersection : allIntersections) {
            outputFile << "\"" << intersection->getName() << "\"" << "\n";
            for (Street *street : intersection->getStreets()) {
                if (street != nullptr) {
                    // a single street only has one previous intersection pointer, so even if two intersections
                    // share a street, it will only be added to the file once
                    if (street->getPrevIntersection() == intersection) {
                        std::string both;
                        if (street->isTwoWay()) both = " dir=\"both\"";
                        outputFile << "\"" << intersection->getName() << "\"" << "->" << "\""
                                   << street->getNextIntersection()->getName() << "\""
                                   << " [label=\"" << street->getEStreetName() << "\"" << both << "]" << "\n";
                    }
                }
            }
        }
        outputFile << "}";
    } else {
        std::cerr << "You cannot write to a closed ofstream." << std::endl;
    }
}
void Network::toPNG(const std::string &dotFileName) const {
    std::string commandStr = "dot -Tpng -O " + dotFileName;
    const char *command = commandStr.c_str();
    system(command);
}

void Network::onWrite(std::ofstream &networkOUTPUT) {
    networkOUTPUT << "------- network -------\n"
                  << "\ndictionary:" << "   TL = trafficLights" << "\n\n";

    networkOUTPUT << "Intersections:\n";
    for (Intersection* intersection : getNetwork()) {
        intersection->onWrite(networkOUTPUT, "   ");
        networkOUTPUT << "---------\n";
    }
    networkOUTPUT << "Vehicles:\n";
    for (Vehicle* vehicle : getVehicles()) {
        vehicle->onWrite(networkOUTPUT);
        networkOUTPUT << "---------\n";
    }
}



Network *Network::getSubNetwork(Intersection *startIntersection, Intersection *endIntersection, const float extraStepModifier) {
    int streetsToEnd = 0;
    int extraSteps = 1;
    bool found = false;
    Network *subNetwork = new Network();
    std::vector<Intersection *> currentIntersections;
    // a list of <copiedIntersection, otherIntersection> Intersection pairs
    std::vector<std::pair<Intersection*, Intersection*>> existingCopies;
    // copied version of current Intersections
    std::vector<Intersection *> copiedIntersections;
    // the new list of copied intersections, replaces copiedIntersections at the end of a curr Intersec loop iteration
    std::vector<Intersection *> nextCopiedIntersections;
    if (startIntersection != endIntersection) {
        // add copied start intersection
        Intersection* newStartIntersection = new Intersection(startIntersection->getName());
        *newStartIntersection = *startIntersection;
        startIntersection->setMultipurposeMarker(true);
        subNetwork->addIntersection(newStartIntersection);

        newStartIntersection->clearStreets();

        std::pair<Intersection*, Intersection*> pair(newStartIntersection, startIntersection);

        existingCopies.emplace_back(pair);
        currentIntersections.emplace_back(startIntersection);
        copiedIntersections.emplace_back(newStartIntersection);

        found = isIn(endIntersection, currentIntersections);

        // keep adding to the sub network until the end intersection has been reached
        while (!found or streetsToEnd < extraSteps) {
            std::cout << "--------" << streetsToEnd <<  "--------" << std::endl;
            std::vector<Intersection *> nextIntersections;
            // take every leaving transition/street in order to find the end intersection.
            // But never add the same transitions twice.
            int currIntersecCounter = 0;
            for (Intersection *currIntersec : currentIntersections) {
                Intersection* currCopiedIntersec = copiedIntersections[currIntersecCounter];
                for (Street *street : currIntersec->getStreets()) {
                    // only handle leaving streets
                    if (currIntersec->isLeavingStreet(street)) {
                        Intersection *otherIntersec = street->getOtherIntersection(currIntersec);
                        // other intersection exists and has not yet been visited/added to the network
                        if (otherIntersec != nullptr and !otherIntersec->isMultipurposeMarker()) {
                    // TODO      currIntersec -----street-----> otherIntersec
                    //     currCopiedIntersec --copiedStreet--> copiedIntersec

                            // copy existing components
                            Intersection* copiedIntersection = new Intersection(otherIntersec->getName());
                            *copiedIntersection = *otherIntersec;
                            Street* copiedStreet = new Street(nullptr, nullptr, A);
                            *copiedStreet = *street;  // still has vehicle pointers, etc ...
                            street->setMultipurposeMarker(true);

                            // the new current intersections
                            nextIntersections.emplace_back(otherIntersec);
                            // the copies that will act as prev or next for the following copies
                            nextCopiedIntersections.emplace_back(copiedIntersection);

                            std::pair<Intersection*, Intersection*> pair2(copiedIntersection, otherIntersec);
                            existingCopies.emplace_back(pair2);

                            // only gradually add
                            copiedIntersection->clearStreets();
                            copiedIntersection->addStreet(copiedStreet);

                            currCopiedIntersec->addStreet(copiedStreet);

                            // change the old prev and next pointers to new ones
                            if (otherIntersec == copiedStreet->getNextIntersection()) {
                                copiedStreet->setPrevIntersection(currCopiedIntersec);
                                copiedStreet->setNextIntersection(copiedIntersection);
                             } else {
                                copiedStreet->setPrevIntersection(copiedIntersection);
                                copiedStreet->setNextIntersection(currCopiedIntersec);
                            }
                            otherIntersec->setMultipurposeMarker(true);
                            subNetwork->addIntersection(copiedIntersection);

                        // the current intersec has already been copied and so has the street's other intersec
                        // ==> only copy a new street for both intersections
                        // ==> don't do this if the street has already been copied
                        } else if (otherIntersec != nullptr and otherIntersec->isMultipurposeMarker() and
                                   !street->isMultipurposeMarker()) {
                    // TODO     currCopiedIntersec  --copiedStreet-->  otherCopiedIntersec

                            Intersection* otherCopiedIntersec;
                            for (const std::pair<Intersection*, Intersection*>& copyPair : existingCopies) {
                                if (copyPair.second == otherIntersec) {
                                    otherCopiedIntersec = copyPair.first;
                                    break;
                                }
                            }

                            // only add new street????
                            Street cpdStreet = *street;
                            Street* copiedStreet = new Street(nullptr, nullptr, A);
                            *copiedStreet = cpdStreet;
                            street->setMultipurposeMarker(true);

                            currCopiedIntersec->addStreet(copiedStreet);
                            otherCopiedIntersec->addStreet(copiedStreet);


                            if (street->getPrevIntersection() == currIntersec) {
                                copiedStreet->setPrevIntersection(currCopiedIntersec);
                                copiedStreet->setNextIntersection(otherCopiedIntersec);
                            } else {
                                copiedStreet->setNextIntersection(currCopiedIntersec);
                                copiedStreet->setPrevIntersection(otherCopiedIntersec);
                            }
                        }
                    }
                }
                ++currIntersecCounter;
            }
            currentIntersections = nextIntersections;
            // if no more reachable intersections exist, stop.
            if (nextIntersections.empty()) {
                break;
            }
            copiedIntersections = nextCopiedIntersections;
            nextIntersections.clear();
            nextCopiedIntersections.clear();
            if (!found) {
                found = isIn(endIntersection, currentIntersections);
                ++streetsToEnd;
                ++extraSteps;
                // if the modifier is 1 or less, no additional steps can ever be taken,
                // if modifier is high enough to let extraSteps exceed the 1 step difference,
                // more additional steps will executed
                if (found) {
                    extraSteps =  streetsToEnd * extraStepModifier;
                }
            } else {
                ++streetsToEnd;
            }
        }
    }
    for (Intersection* intersection : getNetwork()) {
        for (Street* street : intersection->getStreets()) {
            street->setMultipurposeMarker(false);
        }
        intersection->setMultipurposeMarker(false);
    }
    // if nullptr, then the start intersection and tne end intersection are already the same
    return subNetwork;
}

bool Network::isIn(const Intersection *intersection, const std::vector<Intersection *> &intersections) {
    for (Intersection* intersec : intersections) {
        if (intersec == intersection) {
            return true;
        }
    }
    return false;
}



void Network::writeAllVehicleChains(std::ofstream &vehicleChainStream) const {
    // front vehicles of their own current street
    std::vector<const Vehicle*> frontVehicles;
    // whether or not the chain of the front vehicle has been written to the file already
    std::vector<bool> chainWasWritten;

    findFrontVehicles(frontVehicles, chainWasWritten);

    writeChain(vehicleChainStream, frontVehicles, chainWasWritten, nullptr, 0);
}
void Network::findFrontVehicles(std::vector<const Vehicle *> &frontVehicles, std::vector<bool>& chainWasWritten) const {
    for (const Vehicle* vehicle : getVehicles()) {
        if (vehicle->isFront()) {
            frontVehicles.emplace_back(vehicle);
            chainWasWritten.emplace_back(false);
        }
    }
}
void Network::writeChain(std::ofstream &vehicleChainStream, std::vector<const Vehicle *> &frontVehicles,
                         std::vector<bool> &chainWasWritten, const Street* currentStreet, unsigned int startIndex) const {

    for (unsigned int index = startIndex; index < frontVehicles.size(); ++index) {
        const Vehicle* currChainVehicle = frontVehicles[index];
        // currentStreet only exists if recursion is used to find the second lane of a two way street
        if ((!chainWasWritten[index] and currentStreet == nullptr) or
            (!chainWasWritten[index] and currentStreet != nullptr and currentStreet == currChainVehicle->getCurrentStreet())) {

            writeChainString(vehicleChainStream, currChainVehicle);

            chainWasWritten[index] = true;

            // currentStreet only exists if recursion is used to find the second lane of a two way street
            if (currChainVehicle->getCurrentStreet()->isTwoWay() and currentStreet == nullptr) {
                writeChain(vehicleChainStream, frontVehicles, chainWasWritten, currChainVehicle->getCurrentStreet(), index+1);
            // second lane found, no further search needed
            } else if (currentStreet != nullptr) {
                break;
            }
        }
    }
}
void Network::writeChainString(std::ofstream& vehicleChainStream, const Vehicle *&currChainVehicle) const {
    std::string chain;

    Intersection* prevIntersection = currChainVehicle->getPrevIntersection();
    Intersection* nextIntersection = currChainVehicle->getNextIntersection();

    vehicleChainStream << prevIntersection->getName() << " ==> ";
    chain += currChainVehicle->getLicensePlate();
    while (currChainVehicle->getPrevVehicle() != nullptr) {
        chain = currChainVehicle->getPrevVehicle()->getLicensePlate() + " -> " + chain;
        currChainVehicle = currChainVehicle->getPrevVehicle();
    }
    Street* currStreet = currChainVehicle->getCurrentStreet();
    vehicleChainStream << chain  << " ==> " << nextIntersection->getName() << "  (" << currStreet->typeToName() << ", "
                       << Util::isTwoWayToString(currStreet->isTwoWay()) << ")" << "\n";
}






// getters and setters

const std::vector<Intersection *> &Network::getNetwork() const {
    return _network;
}
bool Network::addStreetlessIntersection(Intersection *newStreetlessIntersection) {
    if (newStreetlessIntersection != nullptr) {
        _network.emplace_back(newStreetlessIntersection);
        return true;
    }
    return false;
}
bool Network::addIntersection(Intersection* newIntersection) {
    if (newIntersection != nullptr and !newIntersection->getStreets().empty()) {
        for (Street* street : newIntersection->getStreets()) {
            if (street->getNextIntersection() == nullptr or street->getPrevIntersection() == nullptr) return false;
        }
        _network.emplace_back(newIntersection);
        return true;
    } else {
        return false;
    }
}
Intersection *Network::findIntersection(const std::string &name) const {
    for (Intersection* intersection : _network) {
        if (intersection->getName() == name) return intersection;
    }
    return nullptr;
}

Simulation *Network::getSimulation() const {
    return _simulation;
}

const std::vector<streetType> &Network::getAllowedStreetTypes() const {
    return _allowedStreetTypes;
}
bool Network::typeIsAllowed(const streetType type) const {
    for (const streetType allowedType: getAllowedStreetTypes()) {
        if (allowedType == type) return true;
    }
    return false;
}
void Network::addAllowedStreetType(const streetType newAllowedStreetType) {
    if (!typeIsAllowed(newAllowedStreetType)) {
        _allowedStreetTypes.emplace_back(newAllowedStreetType);
    }
}

const std::vector<Vehicle *> &Network::getVehicles() const {
    return _vehicles;
}
void Network::addVehicle(Vehicle * vehicle) {
    _vehicles.emplace_back(vehicle);
    _simulation->incrementTotalSpawnedVehicles();
}

const std::vector<Intersection *> &Network::getInfluencingIntersections() const {
    return _influencingIntersections;
}
void Network::addInfluencingIntersection(Intersection *influencingIntersection) {
    _influencingIntersections.emplace_back(influencingIntersection);
}
