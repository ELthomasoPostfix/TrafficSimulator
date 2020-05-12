//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Street.h"
#include "Intersection.h"

Street::Street(Intersection* prev, Intersection* next, streetType type) : _prevIntersection(prev), _nextIntersection(next),
                                                                          _type(type) {
    _hasSpeedLimit = false;
}


void Street::requestInfluences(Vehicle* requestingVehicle) const {
    for (const Influence* streetInfl : getInfluences()) {
        requestingVehicle->receiveInfluence(streetInfl);
    }
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
}

const std::vector<std::vector<Vehicle *>> &Street::getLanes() const {
    return _lanes;
}


const std::vector<std::vector<Vehicle *>> &Street::getEntrants() const {
    return _entrants;
}


Vehicle *Street::getFrontOccupant(const int index) const {
    return _frontOccupant[index];
}
void Street::setFrontOccupant(Vehicle *frontOccupant, int index) {
    // index: when becoming the front occupant, obviously enter the street
    // one way street: always enter via lane 0
    // two way street: if enter from prev Intersection, lane 0; if entering from next intersection, lane 1

    if (index == 0) {   // enter into lane 0, will always be from prev intersection
        getNextIntersection()->requestSignal(frontOccupant);
    } else if (index == 1) {
        getPrevIntersection()->requestSignal(frontOccupant);
    }
    _frontOccupant[index] = frontOccupant;
}

Vehicle *Street::getBackOccupant(int index) const {
    return _backOccupant[index];
}
void Street::setBackOccupant(Vehicle *backOccupant, int lane) {
    // if new back occupant, connect to the previous back occupant
    if (backOccupant != getFrontOccupant(lane)) {
        Vehicle* currBackOcc = getBackOccupant(lane);

        currBackOcc->setPrevVehicle(backOccupant);
        backOccupant->setNextVehicle(currBackOcc);
    }
    _backOccupant[lane] = backOccupant;
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


