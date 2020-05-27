//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Intersection.h"
#include "Simulation.h"

#include <utility>

Intersection::Intersection(std::string  name): _name(std::move(name)) {
    _trafficScore = 0;
    _trafficLightCounter = 0;
    _multipurposeMarker = false;
    _ownPointer = this;
}


Intersection::~Intersection() {
    _streets.clear();
    _trafficLights = nullptr;
    _ownPointer = nullptr;

    _trafficLightCounter = 0;
    _trafficScore = 0;
    _currentPair = {};
    _multipurposeMarker = false;
}



void Intersection::emitInfluences(std::ofstream& trafficLightStream) {
    if (getHasTrafficLights()) {
        emitTrafficLightSignal(trafficLightStream);
    }
}

void Intersection::emitTrafficLightSignal(std::ofstream& trafficLightStream) {
    int tlc = getTrafficLightCounter();
    // cycle the traffic lights to the next pair (green pair -> red  | next red pair -> green)
    if (tlc == Simulation::getTrafficLightMaxCount()) {
        if (getTrafficLightPairs().size() > 1) {
            addCycleMessageFront(trafficLightStream);
            // set the two "green streets" to "red" (emit a STOP signal to the front cars of those streets)
            stopCurrentFrontOccupants();
            // set currentTrafficLightPair to the next pair
            cycleTrafficLightsPair();
            // set the two "red streets" to "green" (remove a STOP signal from the front cars of those streets)
            unStopCurrentFrontOccupants();

            setTrafficLightCounter(0);

            addCycleMessageBack(trafficLightStream);
        } else if (getTrafficLightPairs().size() == 1) {
            addSingleTLCycleMessageFront(trafficLightStream);

            invertFrontSTOPStates();

            setTrafficLightCounter(0);

            addSingleTLCycleMessageBack(trafficLightStream);
        }
    } else {
        setTrafficLightCounter(getTrafficLightCounter()+1);
        if (getTrafficLightPairs().size() > 1) {
            addTLCincrementMessage(trafficLightStream);
        } else {
            addSingleTLCincrementMessage(trafficLightStream);
        }
    }
}

// function should be used by a vehicle: it receives the STOP signal
void Intersection::requestSignal(Vehicle *requestingVehicle) const {
    if (requestingVehicle != nullptr) {
        requestingVehicle->receiveInfluence(getTrafficLightInfluence());
    }
}
void Intersection::requestEntrantSignal(Vehicle *requestingVehicle) const {
    if (requestingVehicle != nullptr) {
        requestingVehicle->addEntrantInfluence(getTrafficLightInfluence());
    }
}


void Intersection::calculateTrafficScore() {
    int outgoingScore = 0, incomingScore = 0;
    int* score = &outgoingScore;

    for (Street* street : getStreets()) {
        bool isLeaving = isLeavingStreet(street);
        // differentiate between the score calculated from leaving streets and incoming streets
        if (isLeaving) score = &outgoingScore;
        else score = &incomingScore;

        // TODO can be made more dynamic, check func
        //  - checking for special STOP signal should happen lane based ?
        addInfluenceScores(score, street->getInfluences());

        // leaving lane
        const int indexWhenLeaving = laneIndexWhenLeaving(street);
        *score += hasTLtoScore(getHasTrafficLights());
        *score += vehicleAmountToScore(street->getLanes()[indexWhenLeaving].size());


        // two lanes to take into account
        if (street->isTwoWay()) {
            // switch the score ptr, other lane of the same street runs in opposite direction
            if (isLeaving) score = &incomingScore;
            else score = &outgoingScore;

            // entering lane
            int indexWhenEntering = 1;
            if (indexWhenLeaving == 1) indexWhenEntering = 0;
            *score += hasTLtoScore(getOtherIntersection(street)->getHasTrafficLights());  // already used other intersection
            *score += vehicleAmountToScore(street->getLanes()[indexWhenEntering].size());
        }
    }
    // give the incoming and outgoing scores a different weight
    setTrafficScore((outgoingScore*1) + (incomingScore*1));
}

void Intersection::onWrite(std::ofstream &outputFile, const std::string &indent) {
    outputFile << indent << "name: \"" << getName() << "\"\n"
               << indent << indent   << "TL: " << Util::boolToString(getHasTrafficLights());
    // add traffic light pairs to the file
    for (const std::pair<Street*,Street*>& tlPair : getTrafficLightPairs()) {
        Intersection* commonIntersection = getCommonIntersection(tlPair);
        if (commonIntersection != nullptr and commonIntersection == this) {
            outputFile << indent << indent << indent << tlPair.first->getOtherIntersection(commonIntersection)->getName()
                       << " -> " << commonIntersection->getName() << " -> "
                       << tlPair.second->getOtherIntersection(commonIntersection)->getName() << "\n";
        } else {
            outputFile << indent << indent << indent << tlPair.first->getPrevIntersection()->getName() << " -> "
                       << tlPair.first->getNextIntersection()->getName() << "  and  " << tlPair.second->getPrevIntersection()->getName()
                       << " -> " << tlPair.second->getNextIntersection()->getName() << "   (incorrect pair)\n";
        }
    }
    outputFile << "\n" << indent << indent << "Streets:\n";
    if (!getStreets().empty()) {
        for (Street *street : getStreets()) {
            const Intersection *otherIntersection = street->getOtherIntersection(this);
            if (otherIntersection != nullptr) {
                street->onWrite(outputFile, indent);
                outputFile << indent << indent << indent << "---------\n";
            }
        }
    } else {
        outputFile << indent << indent << indent << "None\n";
    }
}
Intersection* Intersection::getCommonIntersection(const std::pair<Street *, Street *> &TLpair) {
    Street* first = TLpair.first;
    Street* second = TLpair.second;
    // find the intersection that the two streets have in common
    if (first->getPrevIntersection() == second->getPrevIntersection()) {
        return first->getPrevIntersection();
    } else if (first->getNextIntersection() == second->getPrevIntersection()) {
        return first->getNextIntersection();
    // both first's streets have been compared
    // the first street cannot have any intersections in common with the second street anymore
    } else {
        return nullptr;
    }
}




// helper functions to calculate individual scores

int Intersection::hasTLtoScore(const bool hasTL) const {
    if (hasTL) {
        return 2;
    }
    return 0;
}
int Intersection::vehicleAmountToScore(int amount) const {
    if (amount != 0) {
        return sqrt(amount);
    }
    return 0;
}
void Intersection::addInfluenceScores(int* score, const std::vector<const Influence *> &influences) const {
    for (const Influence* influence : influences) {
        *score += influence->toScore();
    }
}




void Intersection::stopCurrentFrontOccupants() const {

    const std::pair<Street*,Street*>& currentGreenPair = getCurrentPair();
    // the index of the lane, which vehicles who will enter the intersection will use
    // -1 means that the current (this) intersection cannot be entered from the designated street
    int enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.first);
    stopSingleFrontOccupant(enteringLaneIndex, currentGreenPair.first->getFrontOccupant(enteringLaneIndex));

    enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.second);
    stopSingleFrontOccupant(enteringLaneIndex, currentGreenPair.second->getFrontOccupant(enteringLaneIndex));
}
void Intersection::stopSingleFrontOccupant(const int enteringLaneIndex, Vehicle* frontOccupant) const {
    // emit a STOP signal to the front occupant of the street, if it exists
    if (enteringLaneIndex != -1) {
        if (frontOccupant != nullptr) {
            frontOccupant->receiveInfluence(getTrafficLightInfluence());
        }
    }
}

void Intersection::unStopCurrentFrontOccupants() const {
    const std::pair<Street*, Street*>& currentGreenPair = getCurrentPair();

    // the index of the lane, which vehicles who will enter the intersection will use
    // -1 means that the current (this) intersection cannot be entered from the designated street
    int enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.first);
    // remove a STOP signal from the front occupant of the first street, if it exists
    unstopSingleFrontOccupant(enteringLaneIndex,  currentGreenPair.first->getFrontOccupant(enteringLaneIndex));

    enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.second);
    // remove a STOP signal to the front occupant of the second street, if it exists
    unstopSingleFrontOccupant(enteringLaneIndex,  currentGreenPair.second->getFrontOccupant(enteringLaneIndex));

}
void Intersection::unstopSingleFrontOccupant(int enteringLaneIndex, Vehicle *frontOccupant) const {
    if (enteringLaneIndex != -1) {
        if (frontOccupant != nullptr) {
            frontOccupant->removeIncomingInfluence(getTrafficLightInfluence());
        }
    }
}

void Intersection::invertFrontSTOPStates() {
    const std::pair<Street*,Street*>& currentTLpair = getCurrentPair();
    // the current pair is "empty"/"nullptr", the traffic lights are red
    if (currentTLpair.first != nullptr and currentTLpair.second != nullptr) {
        turnTLred(currentTLpair);
    // the currentPair is not "empty"/"nullptr", the traffic lights are green
    } else {
        turnTLgreen();
    }
}
void Intersection::turnTLgreen() {
    const std::vector<std::pair<Street*,Street*>>& TLPairs = getTrafficLightPairs();
    if (!TLPairs.empty()) {
        const std::pair<Street *, Street *> &greenPair = TLPairs.front();
        // make the traffic lights GREEN now
        setCurrentPair(greenPair);
        // remove traffic light signal from now stopped front vehicles
        Vehicle* front1 = getIncomingFrontVehicle(greenPair.first);
        if (front1 != nullptr) {
            front1->removeIncomingInfluence(getTrafficLightInfluence());
        }
        Vehicle* front2 = getIncomingFrontVehicle(greenPair.second);
        if (front2 != nullptr) {
            front2->removeIncomingInfluence(getTrafficLightInfluence());
        }
    }
}
void Intersection::turnTLred(const std::pair<Street*,Street*>& currentPair) {
    // remove the traffic light influence
    Vehicle* front1 = getIncomingFrontVehicle(currentPair.first);
    if (front1 != nullptr) {
        front1->receiveInfluence(getTrafficLightInfluence());
    }
    Vehicle* front2 = getIncomingFrontVehicle(currentPair.second);
    if (front2 != nullptr) {
        front2->receiveInfluence(getTrafficLightInfluence());
    }
    // set the current pair to green
    std::pair<Street*,Street*> redPair(nullptr, nullptr);
    setCurrentPair(redPair);
}





Intersection *Intersection::getOtherIntersection(const Street *street) const {
    if (street->getNextIntersection() == this) {
        return street->getPrevIntersection();
    } else {
        return street->getNextIntersection();
    }
}

Vehicle *Intersection::getIncomingFrontVehicle(const Street *street) const {
    int laneIndex = this->laneIndexWhenEntering(street);
    if (laneIndex != -1) {
        return street->getFrontOccupant(laneIndex);
    } else {
        return nullptr;
    }
}



int Intersection::laneIndexWhenLeaving(const Street *street) const {
    if (street == nullptr) {
        return -1;
    }
    // a single way street should only have lane 0
    if (!street->isTwoWay()) {
        // can enter a one way street from 'prev' intersection
        if (street->getPrevIntersection() == this) {
            return 0;
        // cannot enter a one way street from 'next' intersection
        } else {
            return -1;
        }
    // a two way street has two lanes, 0 and 1
    } else {
        // if you want to enter a two way street from the 'previous intersection', then use lane 0
        if (street->getPrevIntersection() == this) {
            return 0;
        // if you want to enter a two way street from the 'next intersection', then use lane 1
        } else if (street->getNextIntersection() == this) {
            return 1;
        // trying to enter a street from an unconnected intersection
        } else {
            return -1;
        }
    }
}
int Intersection::laneIndexWhenEntering(const Street *street) const {
    if (street == nullptr) {
        return -1;
    }
    // a sinle way street should only have lane 0
    if (!street->isTwoWay()) {
        // vehicles enter the next state through lane 0
        if (street->getNextIntersection() == this) {
            return 0;
        // no vehicle will ever enter the previous intersection of a one way road
        } else {
            return -1;
        }
    } else {
        // a vehicle enters the next intersection of a two way street through lane 0
        if (street->getNextIntersection() == this) {
            return 0;
        // a vehicle enters the prev intersection of a two way street through lane 1
        } else if (street->getPrevIntersection() == this) {
            return 1;
        // trying to enter a street from an unconnected intersection
        } else {
            return -1;
        }
    }
    return 0;
}



bool Intersection::isEnteringStreet(const Street *street) const {
    // an entering street is a street that allows you to enter "this" intersection
    // you cannot reach an intersection through a one way street if you start at the next intersection or, in other words,
    // if the intersection you want to reach is the previous intersection
    return !(!street->isTwoWay() and street->getPrevIntersection() == this);
}
std::vector<Street *> Intersection::getAllEnteringStreets() const {
    std::vector<Street*> enteringStreets;
    for (Street* street : getStreets()) {
        // as long as the current intersection isn't the prev intersection of a one way street,
        // it is an entering street
        if (isEnteringStreet(street))
            enteringStreets.emplace_back(street);
    }
    return enteringStreets;
}
bool Intersection::isLeavingStreet(const Street *street) const {
    // two streets that are not leaving:
    // ==> one way street that starts in (prev ==) another intersection and ends in (next ==) this intersection
    // ==> one way street that starts in (prev ==) this intersection and ends in (next ==) this intersection
    return !(!street->isTwoWay() and street->getNextIntersection() == this);
}
std::vector<Street *> Intersection::getAllLeavingStreets() const {
    std::vector<Street*> leavingStreets;
    for (Street* street : getStreets()) {
        // as long as the current intersection isn't the next intersection of a one way street,
        // it is a leaving street
        if (isLeavingStreet(street))
            leavingStreets.emplace_back(street);
    }
    return leavingStreets;
}


// some message functions

void Intersection::addCycleMessageFront(std::ofstream &trafficLightStream) const {
    trafficLightStream << "Traffic light at intersection " << getName() << " with current pair:\n{"
                       << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                       << getName() << " (" << getCurrentPair().first->typeToName() << ", "
                       << getCurrentPair().first->getTwoWayString() << "), "
                       << getCurrentPair().second->getOtherIntersection(this)->getName()
                       << "->" << getName() << " (" << getCurrentPair().second->typeToName()
                       << ", " << getCurrentPair().second->getTwoWayString() << ")} has reached the traffic light counter max value ("
                       << getTrafficLightCounter() << ")\n and cycles its pair to:\n{";
}
void Intersection::addCycleMessageBack(std::ofstream &trafficLightStream) const {
    trafficLightStream << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                       << getName() << " (" << getCurrentPair().first->typeToName()
                       << ", " << getCurrentPair().first->getTwoWayString() << "), "
                       << getCurrentPair().second->getOtherIntersection(this)->getName()
                       << "->" << getName() << " (" << getCurrentPair().second->typeToName() << ", "
                       << getCurrentPair().second->getTwoWayString() << ")"
                       << "} with new TLC counter value being (" << getTrafficLightCounter() << ")\n";
}
void Intersection::addTLCincrementMessage(std::ofstream &trafficLightStream) const {
    trafficLightStream << "Traffic light at intersection " << getName() << " with current pair:\n{"
                       << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                       << getName() << " (" << getCurrentPair().first->typeToName() << ", "
                       << getCurrentPair().first->getTwoWayString() << "), "
                       << getCurrentPair().second->getOtherIntersection(this)->getName()
                       << "->" << getName() << " (" << getCurrentPair().second->typeToName()
                       << ", " << getCurrentPair().second->getTwoWayString()
                       << ")} increased their counter to (" << getTrafficLightCounter() << ")\n";
}
void Intersection::addSingleTLCincrementMessage(std::ofstream &trafficLightStream) const {
    trafficLightStream << "Traffic light at intersection " << getName() << " with current pair:\n";
    const std::pair<Street*,Street*>& currentPair = getCurrentPair();
    if (currentPair.first != nullptr) {
        trafficLightStream  << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                << getName() << " (" << getCurrentPair().first->typeToName() << ", "
                << getCurrentPair().first->getTwoWayString() << "), ";
    } else {
        trafficLightStream << "{nullptr, ";
    }
    if (currentPair.second != nullptr) {
        trafficLightStream << getCurrentPair().second->getOtherIntersection(this)->getName()
                << "->" << getName() << " (" << getCurrentPair().second->typeToName()
                << ", " << getCurrentPair().second->getTwoWayString()
                << ")}";
    } else {
        trafficLightStream << "nullptr}";
    }
    trafficLightStream << " increased their counter to (" << getTrafficLightCounter() << ")\n";
}
void Intersection::addSingleTLCycleMessageFront(std::ofstream &trafficLightStream) const {
    trafficLightStream << "Traffic light at intersection " << getName() << " with current pair:\n{";
    const std::pair<Street*,Street*>& currentPair = getCurrentPair();
    if (currentPair.first != nullptr) {
        trafficLightStream  << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                << getName() << " (" << getCurrentPair().first->typeToName() << ", "
                << getCurrentPair().first->getTwoWayString() << "), ";
    } else {
        trafficLightStream << "nullptr, ";
    }
    if (currentPair.second != nullptr) {
        trafficLightStream << getCurrentPair().second->getOtherIntersection(this)->getName()
                << "->" << getName() << " (" << getCurrentPair().second->typeToName()
                << ", " << getCurrentPair().second->getTwoWayString()
                << ")}";
    } else {
        trafficLightStream << "nullptr}";
    }
    trafficLightStream << " has cycled to new current pair:\n";
}
void Intersection::addSingleTLCycleMessageBack(std::ofstream &trafficLightStream) const {
    const std::pair<Street*,Street*>& currentPair = getCurrentPair();
    if (currentPair.first != nullptr) {
        trafficLightStream  << "{" << getCurrentPair().first->getOtherIntersection(this)->getName() << "->"
                            << getName() << " (" << getCurrentPair().first->typeToName() << ", "
                            << getCurrentPair().first->getTwoWayString() << "), ";
    } else {
        trafficLightStream << "{nullptr, ";
    }
    if (currentPair.second != nullptr) {
        trafficLightStream << getCurrentPair().second->getOtherIntersection(this)->getName()
                           << "->" << getName() << " (" << getCurrentPair().second->typeToName()
                           << ", " << getCurrentPair().second->getTwoWayString()
                           << ")}\n";
    } else {
        trafficLightStream << "nullptr}\n";
    }
    if (currentPair.first == nullptr and currentPair.second == nullptr) {
        trafficLightStream << "===> The sole traffic light pair for this intersection has turned red.\n";
    } else {
        trafficLightStream << "===> The sole traffic light pair for this intersection has turned green.\n";
    }
}





// getters and setters

const std::string &Intersection::getName() const {
    return _name;
}


int Intersection::getTrafficScore() const {
    return _trafficScore;
}
void Intersection::setTrafficScore(int trafficScore) {
    _trafficScore = trafficScore;
}
void Intersection::adjustTrafficScore(int additional) {
    _trafficScore += additional;
}

const std::vector<Street *> &Intersection::getStreets() const {
    return _streets;
}
bool Intersection::addStreet(Street *newStreet) {
    if (newStreet != nullptr) {
        _streets.emplace_back(newStreet);
        return true;
    }
    return false;
}
Street *Intersection::findStreet(const Intersection *intersection1, const Intersection *intersection2,
                                 const streetType& streetType, bool twoWay) const {
    for (Street* street : getStreets()) {
        if ((street->getPrevIntersection() == intersection1 or street->getPrevIntersection() == intersection2) and
            (street->getNextIntersection() == intersection1 or street->getNextIntersection() == intersection2) and
             street->getType() == streetType and street->isTwoWay() == twoWay) {
            return street;
        }
    }
    return nullptr;
}
bool Intersection::removeStreet(Street *toRemove) {
    std::vector<Street*>::iterator streetIt;
    for (streetIt = _streets.begin(); streetIt != _streets.end(); ++streetIt) {
        if (*streetIt == toRemove) {
            _streets.erase(streetIt);
            return true;
        }
    }
    return false;
}
bool Intersection::removeStreet(const Street *toRemove) {
    std::vector<Street*>::iterator streetIt;
    for (streetIt = _streets.begin(); streetIt != _streets.end(); ++streetIt) {
        if (*streetIt == toRemove) {
            _streets.erase(streetIt);
            return true;
        }
    }
    return false;
}
void Intersection::clearStreets() {
    _streets.clear();
}


void Intersection::setTrafficLights(const Influence *trafficLights) {
    if (_trafficLights == nullptr) {
        _trafficLights = trafficLights;
    }
}
const Influence *Intersection::getTrafficLightInfluence() const {
    return _trafficLights;
}
bool Intersection::getHasTrafficLights() const {
    return _trafficLights != nullptr;
}
void Intersection::removeTrafficLights() {
    _trafficLights = nullptr;
    _trafficLightPairs.clear();
}


int Intersection::getTrafficLightCounter() const {
    return _trafficLightCounter;
}
void Intersection::setTrafficLightCounter(int trafficLightCounter) {
    _trafficLightCounter = trafficLightCounter;
}

const std::vector<std::pair<Street *, Street *>> &Intersection::getTrafficLightPairs() const {
    return _trafficLightPairs;
}
void Intersection::addTrafficLightPair(std::pair<Street *, Street *> newPair) {
    Street* first  = newPair.first;
    Street* second = newPair.second;
    // if any street of the pair is a single way street, its next must be 'this'
    // at least one of the streets in the traffic light pair must exist, so single state pairs are acceptable
    if ((first != nullptr and !isEnteringStreet(first)) or (second != nullptr and !isEnteringStreet(second))) return;
    // each street, part of a traffic light pair, is exclusively part of a single pair.
    // No Street can be part of multiple traffic light pairs
    for (const std::pair<Street*, Street*>& pair : _trafficLightPairs) {
        // the pair is not unique, it shares a street with another pair, don't add new pair
        if (haveIntersection(pair, newPair)) return;
    }
    // newPair is a completely unique pair
    _trafficLightPairs.emplace_back(newPair);
}
std::vector<Street *> Intersection::getUnpairedStreets() const {
    std::vector<Street*> unpairedStreets;
    // for any street
    for (Street* street : getStreets()) {
        // if no pair contains this street, add it to the list
        if (!streetInTrafficLightPairs(street))
        {
            unpairedStreets.emplace_back(street);
        }
    }
    return unpairedStreets;
}
bool Intersection::streetInTrafficLightPairs(const Street *street) const {
    // for any pair
    for (std::pair<Street *, Street *> trafficLightPair : getTrafficLightPairs()) {
        // if the pair contains the street, return true
        if (trafficLightPair.first == street or trafficLightPair.second == street) {
            return true;
        }
    }
    // no pair contained the street
    return false;
}
bool Intersection::haveIntersection(const std::pair<Street *, Street *> &pair1, const std::pair<Street *, Street *> &pair2) {
    // the two traffic light pairs may have no streets in common, nor may they be the same pair
    return pair1.first  == pair2.first or pair1.first  == pair2.second or
           pair1.second == pair2.first or pair1.second == pair2.second;
}


const std::pair<Street *, Street *> &Intersection::getCurrentPair() const {
    return _currentPair;
}
void Intersection::setCurrentPair(const std::pair<Street *, Street *> &currentPair) {
    _currentPair = currentPair;
}
void Intersection::cycleTrafficLightsPair() {
    const std::vector<std::pair<Street*, Street*>>& allPairs = getTrafficLightPairs();
    const std::pair<Street*, Street*>& currentPair = getCurrentPair();

    // find the current pair in all the trafficLightPairs and set current pair to the next pair
    for (unsigned int index = 0; index < allPairs.size(); ++index) {
        // the current pair has not been found yet, arrived at the back() of the pairsList after searching through it entirely
        if (allPairs.back() == allPairs[index]) {
            // the current pair is the back pair, the next pair is the front
            if (allPairs.back() == currentPair) {
                setCurrentPair(allPairs.front());
            } else {
                std::cerr << "When cycling the traffic lights at intersection \"" << getName() << "\", the current"
                          << " trafficLightPair was not found." << std::endl;
            }
        // current has been found, cycle to the next pair
        } else if (allPairs[index] == currentPair) {
            setCurrentPair(allPairs[index+1]);
            break;
        }
    }
}

bool Intersection::isMultipurposeMarker() const {
    return _multipurposeMarker;
}
void Intersection::setMultipurposeMarker(bool multipurposeMark) {
    _multipurposeMarker = multipurposeMark;
}
void Intersection::removeAllMArkings() {
    for (Street* street : getStreets()) {
        street->setMultipurposeMarker(false);
    }
    setMultipurposeMarker(false);
}

Intersection *Intersection::getOwnPointer() const {
    return _ownPointer;
}

void Intersection::setPrevIntersections(const std::vector<Intersection*>& input) {
    _previous_intersections = input;
}
std::vector<Intersection *> Intersection::getPrevIntersections() {
    return _previous_intersections;
}