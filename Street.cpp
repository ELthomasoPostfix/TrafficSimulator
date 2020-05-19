//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Street.h"
#include "Intersection.h"
#include "Simulation.h"

Street::Street(Intersection* prev, Intersection* next, streetType type) : _prevIntersection(prev), _nextIntersection(next),
                                                                          _type(type) {
    _hasSpeedLimit = false;
}

bool Street::fillEmptyLanes() {
    const int lanesSize = getLanes().size();
    if (lanesSize == 2) return false;
    std::vector<Vehicle*> emptyLane;
    // any street needs at least one empty lane
    if (lanesSize == 0) {
        _lanes.emplace_back(emptyLane);
        if (!isTwoWay()) return true;
    }
    // is the street is two way, add a lane if there is still only one
    if (isTwoWay()) {
        _lanes.emplace_back(emptyLane);
        return true;
    }
    return false;
}
bool Street::fillEmptyEntrantLanes() {
    const int lanesSize = getEntrants().size();
    if (lanesSize == 2) return false;
    std::vector<Vehicle*> emptyLane;
    // any street needs at least one empty lane
    if (lanesSize == 0) {
        _entrants.emplace_back(emptyLane);
        if (!isTwoWay()) return true;
    }
    // is the street is two way, add a lane if there is still only one
    if (isTwoWay()) {
        _entrants.emplace_back(emptyLane);
        return true;
    }
    return false;
}

void Street::requestInfluences(Vehicle* requestingVehicle) const {
    for (const Influence* streetInfl : getInfluences()) {
        requestingVehicle->receiveInfluence(streetInfl);
    }
}
void Street::requestEntrantInfluences(Vehicle *requestingVehicle) const {
    for (const Influence* streetInfl : getInfluences()) {
        requestingVehicle->addEntrantInfluence(streetInfl);
    }
    // also request a STOP signal from the intersection at the other end of the street
    Intersection* otherIntersection = this->getOtherIntersection(requestingVehicle->getNextIntersection());
    otherIntersection->requestEntrantSignal(requestingVehicle);
}


Intersection *Street::getOtherIntersection(const Intersection *intersection) const {
    if (getNextIntersection() == intersection) return getPrevIntersection();
    else if (getPrevIntersection() == intersection) return getNextIntersection();
    else return nullptr; // the passed intersection isn't part of the street (isn't prev or next)
}





// getters and setters


Intersection *Street::getNextIntersection() const {
    return _nextIntersection;
}
void Street::setNextIntersection(Intersection *nextIntersection) {
    _nextIntersection = nextIntersection;
}

Intersection *Street::getPrevIntersection() const {
    return _prevIntersection;
}
void Street::setPrevIntersection(Intersection *prevIntersection) {
    _prevIntersection = prevIntersection;
}

streetType Street::getType() const {
    return _type;
}
std::string Street::typeToName() const {
    switch (getType()) {
        case A:
            return "A";
        case B:
            return "B";
        case T:
            return "T";
    }
}
streetType Street::nameToType(const char& name) {
    switch (name) {
        case 'A':
            return A;
        case 'B':
            return B;
        case 'T':
            return T;
    }
    std::cerr << "Incorrect streetType name passed so streetType 'A' will be passed by default. This should not cause un"
                 "definded behaviour." << std::endl;
    return A;
}

const std::vector<std::vector<Vehicle *>> &Street::getLanes() const {
    return _lanes;
}


const std::vector<std::vector<Vehicle *>> &Street::getEntrants() const {
    return _entrants;
}
void Street::addEntrant(const int indexWhenLeaving, Vehicle *entrant) {
    if (entrant != nullptr and indexWhenLeaving != -1) {
        _entrants[indexWhenLeaving].emplace_back(entrant);
    }
}
bool Street::removeEntrant(int indexWhenLeaving, const Vehicle *entrant) {
    if (entrant != nullptr and indexWhenLeaving != -1) {
        std::vector<Vehicle *>::iterator entrantLaneIt;
        std::vector<Vehicle *> &lane = _entrants[indexWhenLeaving];
        for (entrantLaneIt = lane.begin(); entrantLaneIt != lane.end(); ++entrantLaneIt) {
            if (entrant == *entrantLaneIt) {
                lane.erase(entrantLaneIt);
                return true;
            }
        }
    }
    return false;
}


Vehicle *Street::getFrontOccupant(const int index) const {
    return _frontOccupant[index];
}
void Street::setFrontOccupant(Vehicle *frontOccupant, int lane) {
    // index: when becoming the front occupant, obviously enter the street
    // one way street: always enter via lane 0
    // two way street: if entering from prev Intersection, lane 0; if entering from next intersection, lane 1
    if (lane == 0) {   // enter into lane 0, will always be from prev intersection
        if (getNextIntersection()->getHasTrafficLights()) {
            getNextIntersection()->requestSignal(frontOccupant);
        }
    } else if (lane == 1) {    // entering into lane 1, will always be from next intersection
        if (getPrevIntersection()->getHasTrafficLights()) {
            getPrevIntersection()->requestSignal(frontOccupant);
        }
    }
    if (lane != -1) {      // index == -1  means an invalid attempt at entering a street
        // the previous front was a nullptr so the frontOccupant is the sole vehicle in the street, meaning prev is nullptr
        // else it is part of a file of cars and it becomes the new front after the previous front enters a new street;
        // then its prev must remain the same
        if (_frontOccupant[lane] == nullptr) {
            frontOccupant->setPrevVehicle(nullptr);
        }
        frontOccupant->setNextVehicle(nullptr);
        _frontOccupant[lane] = frontOccupant;
        // also set the back vehicle if needed
        if (_backOccupant[lane] == nullptr) {
            _backOccupant[lane] = frontOccupant;
        }
    }
}

Vehicle *Street::getBackOccupant(int index) const {
    return _backOccupant[index];
}
void Street::setBackOccupant(Vehicle *newBackOccupant, int lane) {
    Vehicle* currBackOcc = getBackOccupant(lane);
    // if a back occupant exists, add the new back occupant to the vehicle chain
    if (currBackOcc != nullptr) {
        currBackOcc->setPrevVehicle(newBackOccupant);
        newBackOccupant->setNextVehicle(currBackOcc);
        newBackOccupant->setPrevVehicle(nullptr);
        _backOccupant[lane] = newBackOccupant;
    // wrongly called setBackOccupant(), call setFrontOccupant() instead.
    // no back vehicle exists, so the new back must also be the new front vehicle
    } else {
        setFrontOccupant(newBackOccupant, lane);
    }
}

bool Street::isTwoWay() const {
    return _isTwoWay;
}
void Street::setIsTwoWay(bool isTwoWay) {
    _isTwoWay = isTwoWay;
}

const std::vector<const Influence *> &Street::getInfluences() const {
    return _influences;
}
bool Street::addInfluence(const Influence* newInfluence) {
    for (const Influence* influence : getInfluences()) {
        if (influence->getType() == newInfluence->getType()) return false;
    }
    _influences.emplace_back(newInfluence);
    return true;
}
bool Street::removeInfluence(influenceType influenceType) {
    // there are no influences to delete
    if (getInfluences().empty()) return false;
    std::vector<const Influence*> newVector;
    for (const Influence* influence : getInfluences()) {
        if (influence->getType() != influenceType) {
            newVector.emplace_back(influence);
        } else {
            if (influence->getType() == STOP) {
                setHasStopSignal(false);
            } else if (influence->getType() == LIMIT) {
                setHasSpeedLimit(false);
            }
        }
    }
    // no influence was deleted
    if (getInfluences().size() == newVector.size()) return false;
    _influences = newVector;
    return true;
}


bool Street::hasSpeedLimit() const {
    return _hasSpeedLimit;
}
void Street::setHasSpeedLimit(bool hasSpeedLimit) {
    _hasSpeedLimit = hasSpeedLimit;
}

bool Street::hasStopSignal() const {
    return _hasStopSignal;
}
void Street::setHasStopSignal(bool hasStopSignal) {
    _hasStopSignal = hasStopSignal;
}




