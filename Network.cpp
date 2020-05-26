//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Network.h"
#include "StateElimination.h"

Network::Network() {
    _simulation = new Simulation();
}

Network::~Network() {
    delete _simulation;
    for (Intersection* intersection : _network) {
        delete intersection;
    }
    for (Vehicle* vehicle : _vehicles) {
        delete vehicle;
    }
}


void Network::doMainLoop(const int duration, std::string& ofName, std::string& ofName2, std::string& ofName3) {
    // ---- pre loop ----
    Simulation* sim = _simulation;
    std::ofstream driveStream;
    std::ofstream trafficLightStream;
    std::ofstream vehicleChainStream;
    driveStream.open(ofName);
    trafficLightStream.open(ofName2);
    vehicleChainStream.open(ofName3);

    // ---- loop ----
    for (int dur = 0; dur < duration; ++dur) {
        driveStream        << "---- " << dur << " ----" << std::endl;
        trafficLightStream << "---- " << dur << " ----" << std::endl;
        vehicleChainStream << "\n---- " << dur-1 << " ----\n" << std::endl;
        // let intersections with traffic lights cycle through the traffic light pairs (of Street*s) and send out
        // signals when cycling from one pair to the next.
        // vehicles will request traffic light influence upon calling the setFrontOccupant() function of a street
        const std::vector<Intersection*>& inflIntersecs = getInfluencingIntersections();
        for (Intersection* influencingIntersection : inflIntersecs) {
            influencingIntersection->emitInfluences(trafficLightStream);
            // add fluff text to increase readability
            if (influencingIntersection != inflIntersecs.back()) {
                trafficLightStream << "###\n";
            }
        }

        for (Intersection* intersection : getNetwork()) {
            intersection->calculateTrafficScore();
        }

        writeAllVehicleChains(vehicleChainStream);

        tryRandomVehicleSpawn();

        // let all vehicles drive and if they can, send out signals
        for (auto vehicle = getVehicles().begin(); vehicle != getVehicles().end(); vehicle++) {
            if ((*vehicle.base())->drive(driveStream)) {
                std::cout << " drove (" << dur << ")" << std::endl;
                (*vehicle.base())->accident();
            // the vehicle has reached their end point and should be deleted
            // preperations for the deletion are done in drive
            } else {
                _vehicles.erase(vehicle--);
            }
        }
    }

    // ---- post loop  ----
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
                    std::string extraLabel = street->getPrevIntersection()->getName();
                    extraLabel += Util::boolToArrowString(street->isTwoWay(), true);
                    extraLabel += street->getNextIntersection()->getName();
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

void Network::findAndSetPath(Vehicle *recipient, Intersection *startIntersection, Intersection *endIntersection) {

    // Make a copied subNetwork from the already existing network and perform State Elimination
    // on the sub network. Find all possible paths that exist from the start intersection to the end intersection
    // and return them
    std::vector<std::vector<const Street*>> paths = findAllPaths(startIntersection, endIntersection);

    // TODO \/ select path
    // select and set one of the available paths
    selectPath(paths, startIntersection, recipient);

    // set the vehicle's member variables according to the chosen path
    recipient->setCurrentStreet(const_cast<Street *>(recipient->getPath().front()));
    if (recipient->getPath().size() > 1) {
        recipient->setNextStreet(const_cast<Street *>(recipient->getPath()[1]));
    } else {
        recipient->setNextStreet(nullptr);
    }
    recipient->setPrevIntersection(startIntersection);
    recipient->setNextIntersection(recipient->getCurrentStreet()->getOtherIntersection(startIntersection));
    // TODO /\ select path
}

std::vector<std::vector<const Street *>> Network::findAllPaths(Intersection *startIntersection, Intersection *endIntersection) {

    // find a subnetwork from the start to end intersection + extra steps for more volume
    Network* subNetwork = this->getSubNetwork(startIntersection, endIntersection, 3);

    // TODO \/ post subnetwork constructed: toDot
    std::ofstream STELstream;
    std::string subNetworkOUTPUT = "subNet";

    STELstream.open(subNetworkOUTPUT + ".dot");
    subNetwork->toDot(STELstream);
    STELstream.close();
    // TODO /\  post subnetwork constructed: toDot


    Intersection* subStart = nullptr;
    Intersection* subEnd = nullptr;

    // assign the copied start and end states for state elimination
    assignSubStartAndEnd(subStart, subEnd, subNetwork, startIntersection, endIntersection);

    // do state elimination on the copied network
    StateElimination stateElimination;
    stateElimination.eliminate(subNetwork, subStart, subEnd);

    // TODO \/ post StateElim on SubNetwork: toDotElim
    std::string stateElimOUTPUT = "STELlarger";

    STELstream.open(stateElimOUTPUT + ".dot");
    subNetwork->toDot(STELstream);
    STELstream.close();

    STELstream.open(stateElimOUTPUT + "Elim.dot");
    subNetwork->toDotElim(STELstream);
    STELstream.close();
    // TODO /\ post StateElim on SubNetwork: toDotElim


    // the final list of lists where each list of streets represents a single path
    return subNetwork->elimStreetsToPaths(subStart);
}






void Network::tryRandomVehicleSpawn() {
    Simulation* sim = getSimulation();
    if (sim->getSpawnTimer() == Simulation::getVehicleSpawnRate()) {
        addVehicle(createVehicle());
        sim->setSpawnTimer(0);
    } else {
        sim->incrementSpawnTimer();
    }
}
Vehicle* Network::createVehicle() {
    // create random vehicle
    const int randomClass = random()%2 + 1;
    vehicleClass vehicleClass = Vehicle::intToClass(randomClass);
    Vehicle* spawnedVehicle = getSimulation()->createVehicleObj(vehicleClass);

    // randomly choose a start and end intersection
    Intersection* startIntersection = getRandomIntersection();
    Intersection* endIntersection = getRandomIntersection();
    const std::vector<Intersection*>& network = getNetwork();

    // if there is only one intersection, don't do the loop to get other intersections
    // if there is more than one intersection, make sure that you get two different intersections for a start and end
    if (network.size() != 1) {
        while (startIntersection == endIntersection) {
            endIntersection = getRandomIntersection();
        }
    }
    // check if intersections are viable
    spawnedVehicle->setStartIntersection(startIntersection);
    spawnedVehicle->setEndIntersection(endIntersection);

    // find a path from the start point to the end point and set it as the vehicle's path
    findAndSetPath(spawnedVehicle, startIntersection, endIntersection);

    // check for possible faulty behavior
    if (spawnedVehicle->getPath().empty()) {
        std::cerr << "warning in Network class, createVehicle() function: even after findAndSetPath(), the spawned vehicle's path is empty." << std::endl;
    }

    // set the starting progress
    const int progress = random() % Simulation::getMinCarDistance() + 10;
    spawnedVehicle->setProgress(progress);
    // set speed
    const int speed = random() % 80 + 1;
    spawnedVehicle->setSpeed(speed);

    // actually enter the street (prepare variables)
    Street* currStreet = spawnedVehicle->getCurrentStreet();
    const int currStreetLane = spawnedVehicle->getPrevIntersection()->laneIndexWhenLeaving(currStreet);

    // check for possible faulty behavior
    if (currStreetLane == -1) {
        std::cerr << "warning in Network class, createVehicle() function: the lane of the current street cannot be"
                     " entered from the previous intersection. This should be possible, as it is taken into"
                     " account during pathfinding." << std::endl;
    }

    // actually enter the street (actual)
    currStreet->requestInfluences(spawnedVehicle);
    currStreet->setBackOccupant(spawnedVehicle, currStreetLane);

    return spawnedVehicle;
}
void Network::selectPath(const std::vector<std::vector<const Street *>> &paths, const Intersection *startIntersection,
                         Vehicle *spawnedVehicle) {

    unsigned int selectedPathIndex = 0;  // index of the path with the lowest traffic score
    int counter = 0;    // keeps track of current street index for each single path
    int lowestTrafficScore = 1000000;   // start score

    // TODO check path viablility:
    //    + streetTypes

    // compare each path to each other to find the most "suitable" one
    for (unsigned int pathIndex = 0; pathIndex < paths.size(); ++pathIndex) {
        int pathScore = 0;  // accumulate all the traffic scores of all intersections on a single path
        const std::vector<const Street*>& path = paths[pathIndex];  // the path, in the form of a list of streets
        const Intersection* prevIntersec = startIntersection;   // the start point of every
        // compare the total score of each path to find the lowest score
        for (const Street* street : path) {
            // a street is a pair of intersections, so to collect the score of all intersections
            // on the path always add the score of the "left" intersection
            pathScore += prevIntersec->getTrafficScore();

            // go to the next (right) intersection to move to the following street's "start" intersection
            prevIntersec = street->getOtherIntersection(prevIntersec);
            // if last street on the path, because only "left" intersection scores have been added to the total,
            // now also add the last (right intersection of the last street) intersection's traffic score
            if (counter == path.size()-1) {
                pathScore += prevIntersec->getTrafficScore();
            }
            ++counter;
        }
        // check if this path's score is lower than the lowest so far
        if (pathScore < lowestTrafficScore) {
            lowestTrafficScore = pathScore;
            selectedPathIndex = pathIndex;
        }
        pathScore = 0;
        counter = 0;
    }
    // set the selected path
    spawnedVehicle->setPath(paths[selectedPathIndex]);
}



Intersection *Network::getRandomIntersection() const {
    const int randomNum = random()%getNetwork().size();
    return getNetwork()[randomNum];
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
                            copiedStreet->clearLanes();
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
                            cpdStreet.clearLanes();
                            Street* copiedStreet = new Street(nullptr, nullptr, A);
                            copiedStreet->clearLanes();
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

std::vector<std::vector<const Street*>> Network::elimStreetsToPaths(Intersection* startIntersection) {

    // startIntersection is the copied counterpart of the original start intersection
    Intersection* prevIntersection = startIntersection;

    std::vector<std::vector<const Intersection*>> intersectionPath;
    std::vector<std::vector<const Street*>> streetPath;
    const std::vector<Street*>& streets = startIntersection->getAllLeavingStreets();
    // for each final street in the eliminated/minimised network
    for (Street* elimStreet : streets) {
        // if the street is an elimStreet class street, then process all the intersections contained within
        if (elimStreet->isEStreet()) {

            const std::vector<const Street*>& elimStreets = *elimStreet->getStreets();
            // for each street street contained within the elimStreet
            // (== each "character" of the "RE" on a final transition)
            for (const Street* street : elimStreets) {
                // front street, make a new list, which represents a single path after elimination
                if (street == elimStreets.front()) {
                    std::vector<const Intersection*> singlePathIntersec = {prevIntersection->getOwnPointer()};
                    intersectionPath.emplace_back(singlePathIntersec);

                    std::vector<const Street*> singlePathStreet = {street->getOwnPointer()};
                    streetPath.emplace_back(singlePathStreet);
                    // next intersection
                    prevIntersection = prevIntersection->getOtherIntersection(street);
                // a path list already exists, simply add to it
                } else {
                    intersectionPath.back().emplace_back(prevIntersection->getOwnPointer());

                    streetPath.back().emplace_back(street->getOwnPointer());
                    // next intersection
                    prevIntersection = prevIntersection->getOtherIntersection(street);
                    if (street == elimStreets.back()) {
                        intersectionPath.back().emplace_back(prevIntersection->getOwnPointer());
                    }
                }
            }
            prevIntersection = startIntersection;
        } else {
            const std::vector<const Intersection*> singlePathIntersec = {elimStreet->getPrevIntersection(), elimStreet->getNextIntersection()};
            intersectionPath.emplace_back(singlePathIntersec);
            const std::vector<const Street*> singlePathStreet = {elimStreet->getOwnPointer()};
            streetPath.emplace_back(singlePathStreet);
        }
    }

    // TODO  \/ delete, testing code
    testPathCorrectnessManuel(intersectionPath, streetPath, startIntersection);
    // TODO  /\ delete, testing code

    return streetPath;
}

void
Network::testPathCorrectnessManuel(const std::vector<std::vector<const Intersection *>> &intersectionPath,
        const std::vector<std::vector<const Street *>> &streetPath, Intersection* startIntersection) {
    std::vector<std::string> intersStrVec;
    std::string singlePath;

    // make a path string based on the list of intersections
    int counter = 0;
    for (const std::vector<const Intersection*>& path : intersectionPath) {
        for (const Intersection* intersection : path) {
            singlePath += intersection->getName();
            if (counter < path.size()-1) {
                singlePath += "->";
            }
            ++counter;
        }
        counter = 0;
        intersStrVec.emplace_back(singlePath);
        singlePath = "";
    }
    // make a path string based on the list of streets
    std::vector<std::string> streetStrVec;
    counter = 0;

    for (const std::vector<const Street*>& path : streetPath) {
        const Intersection* prevIntersec = startIntersection->getOwnPointer();
        for (const Street* street : path) {
            singlePath += prevIntersec->getName();

            prevIntersec = prevIntersec->getOtherIntersection(street);
            if (counter < path.size()-1) {
                singlePath += "->";
            } else {
                singlePath += "->" + prevIntersec->getName();
            }
            ++counter;
        }
        counter = 0;
        streetStrVec.emplace_back(singlePath);
        singlePath = "";
    }
    std::cout << std::endl << "Compare the path created based on each final elimStreets' intersections list with the"
              << "path path created based on each final elimStreets' streets list.\nIf the two paths are found to be the same,"
              << " then pass, else fail." << std::endl << "------------------------" << std::endl;
    // check whether both path strings are the same each corresponding path
    for (unsigned int index = 0; index < intersStrVec.size(); ++index) {
        std::cout << "path" << index <<  ": ";
        if (intersStrVec[index] == streetStrVec[index]) {
            std::cout << "pass      (" << streetStrVec[index] << ")" << std::endl;
        } else {
            std::cout << "fail      (" << intersStrVec[index] << "  vs  " << streetStrVec[index] << ")" << std::endl;
        }
    }
    std::cout << "------------------------" << std::endl;
}



void Network::removeAllMultipurposeMarkers() const {
    for (Intersection* intersection : getNetwork()) {
        intersection->removeAllMArkings();
    }
}


// private

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


void Network::assignSubStartAndEnd(Intersection *&subStart, Intersection *&subEnd, Network *subNetwork,
                                    Intersection* startIntersection, Intersection* endIntersection) const {
    for (Intersection* subIntersec : subNetwork->getNetwork()) {
        if (subIntersec->getOwnPointer() == startIntersection) {
            subStart = subIntersec;
        } else if (subIntersec->getOwnPointer() == endIntersection) {
            subEnd = subIntersec;
        }
    }
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
void Network::removeIntersection(const Intersection *toRemoveIntersec) {
    std::vector<Intersection*>::iterator intersecIt;
    for (intersecIt = _network.begin(); intersecIt != _network.end(); ++intersecIt) {
        if (*intersecIt == toRemoveIntersec) {
            _network.erase(intersecIt);
            break;
        }
    }
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
    if (vehicle != nullptr) {
        _vehicles.emplace_back(vehicle);
        _simulation->incrementTotalSpawnedVehicles();
    } else {
        std::cerr << "The vehicle that was to be added to the network was a nullptr." << std::endl;
    }
}
void Network::removeVehicle(const Vehicle *toRemoveVehicle) {
    std::vector<Vehicle*>::iterator vehicleIt;
    for (vehicleIt = _vehicles.begin(); vehicleIt != _vehicles.end(); ++vehicleIt) {
        if (*vehicleIt == toRemoveVehicle) {
            _vehicles.erase(vehicleIt);
            break;
        }
    }
}


const std::vector<Intersection *> &Network::getInfluencingIntersections() const {
    return _influencingIntersections;
}
void Network::addInfluencingIntersection(Intersection *influencingIntersection) {
    _influencingIntersections.emplace_back(influencingIntersection);
}




