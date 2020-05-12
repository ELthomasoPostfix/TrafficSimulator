//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Intersection.h"

#include <utility>

Intersection::Intersection(std::string  name): _name(std::move(name)) {
    _trafficScore = 0;
    _trafficLightCounter = 0;
}

// function should be used by a vehicle: it receives the STOP signal
void Intersection::requestSignal(Vehicle *requestingVehicle) const {
    requestingVehicle->receiveInfluence(getTrafficLightInfluence());
}


void Intersection::emitInfluences() {
    if (getHasTrafficLights()) {
        emitTrafficLightSignal();
    }
}

void Intersection::emitTrafficLightSignal() {
    int tlc = getTrafficLightCounter();
    // cycle the traffic lights to the next pair (green pair -> red  | next red pair -> green)
    if (tlc == 5) {
        // set the two "green streets" to "red" (emit a STOP signal to the front cars of those streets)
        stopCurrentFrontOccupants();
        // set currentTrafficLightPair to the next pair
        cycleTrafficLightsPair();
        // set the two "red streets" to "green" (remove a STOP signal from the front cars of those streets)
        unStopCurrentFrontOccupants();

        setTrafficLightCounter(0);
    } else {
        setTrafficLightCounter(getTrafficLightCounter()+1);
    }
}

void Intersection::stopCurrentFrontOccupants() const {

    const std::pair<Street*,Street*>& currentGreenPair = getCurrentPair();
    // the index of the lane, which vehicles who will enter the intersection will use
    int enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.first);
    // emit a STOP signal to the front occupant of the street, if it exists
    if (enteringLaneIndex != -1) {
        Vehicle* frontOccupant = currentGreenPair.first->getFrontOccupant(enteringLaneIndex);
        if (frontOccupant != nullptr) {
            frontOccupant->receiveInfluence(getTrafficLightInfluence());
        }
    }
    enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.second);
    // emit a STOP signal to the front occupant of the street, if it exists
    if (enteringLaneIndex != -1) {
        Vehicle* frontOccupant = currentGreenPair.second->getFrontOccupant(enteringLaneIndex);
        if (frontOccupant != nullptr) {
            frontOccupant->receiveInfluence(getTrafficLightInfluence());
        }
    }
}

void Intersection::unStopCurrentFrontOccupants() const {
    const std::pair<Street*, Street*>& currentGreenPair = getCurrentPair();

    // the index of the lane, which vehicles who will enter the intersection will use
    int enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.first);
    // emit a STOP signal to the front occupant of the first street, if it exists
    if (enteringLaneIndex != -1) {
        Vehicle* frontOccupant = currentGreenPair.first->getFrontOccupant(enteringLaneIndex);
        if (frontOccupant != nullptr) {
            frontOccupant->removeIncomingInfluence(getTrafficLightInfluence());
        }
    }
    enteringLaneIndex = laneIndexWhenEntering(currentGreenPair.second);
    // emit a STOP signal to the front occupant of the second street, if it exists
    if (enteringLaneIndex != -1){
        Vehicle* frontOccupant = currentGreenPair.second->getFrontOccupant(enteringLaneIndex);
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
    if ((first != nullptr and !isEnteringStreet(first)) or (second != nullptr and !isEnteringStreet(second))) return;
    // each street, part of a traffic light pair, is exclusively part of a single pair.
    // No Street can be part of multiple traffic light pairs
    for (const std::pair<Street*, Street*>& pair : _trafficLightPairs) {
        // newPair is a completely unique pair, else return
        if (haveIntersection(pair, newPair))
            return;
    }
    _trafficLightPairs.emplace_back(newPair);
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
        if (allPairs.back() == allPairs[index]) {
            if (allPairs.back() == currentPair) {
                setCurrentPair(allPairs.front());
                break;
            } else {
                std::cerr << "When cycling the traffic lights at intersection \"" << getName() << "\", the current"
                          << " trafficLightPair was not found." << std::endl;
            }
        } else if (allPairs[index] == currentPair) {
            setCurrentPair(allPairs[index+1]);
            break;
        }
    }
}




