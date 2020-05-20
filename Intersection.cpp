//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Intersection.h"
#include "Simulation.h"

#include <utility>

Intersection::Intersection(std::string  name): _name(std::move(name)) {
    _trafficScore = 0;
    _trafficLightCounter = 0;
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


void Intersection::emitInfluences(std::ofstream& trafficLightStream) {
    if (getHasTrafficLights()) {
        emitTrafficLightSignal(trafficLightStream);
    }
}

void Intersection::emitTrafficLightSignal(std::ofstream& trafficLightStream) {
    int tlc = getTrafficLightCounter();
    // cycle the traffic lights to the next pair (green pair -> red  | next red pair -> green)
    if (tlc == Simulation::getTrafficLightMaxCount()) {
        addCycleMessageFront(trafficLightStream);
        // set the two "green streets" to "red" (emit a STOP signal to the front cars of those streets)
        stopCurrentFrontOccupants();
        // set currentTrafficLightPair to the next pair
        cycleTrafficLightsPair();
        // set the two "red streets" to "green" (remove a STOP signal from the front cars of those streets)
        unStopCurrentFrontOccupants();

        setTrafficLightCounter(0);

        addCycleMessageBack(trafficLightStream);
    } else {
        setTrafficLightCounter(getTrafficLightCounter()+1);
        addTLCincrementMessage(trafficLightStream);
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
        // newPair is a completely unique pair, else return
        if (haveIntersection(pair, newPair)) return;
    }
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
        // the current pair has not been found yet, arrived at the back() of the pairsList after searching it entirely
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
