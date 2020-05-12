//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Vehicle.h"
#include "Street.h"
#include "Intersection.h"

Vehicle::Vehicle(vehicleClass vClass) : _class(vClass) {
    _isLimited = false;
    _isStopped = false;
}

void Vehicle::emitInfluence() {

}

void Vehicle::enterStreet() {
    Street* streetToEnter = getNextStreet();
    // TODO remove vehicle from _entrants
    // TODO move vehicle to occupants
    // the index of the correct lane to enter in the next street
    // next intersection, because the next intersection is the intersection it enters the new street from
    // when leaving is about leaving the intersection, it gets the correct lane to leave INTO/WITH
    int laneIndexToEnter = getNextIntersection()->laneIndexWhenLeaving(streetToEnter);
    if (streetToEnter->getFrontOccupant(laneIndexToEnter) == nullptr) {
        streetToEnter->setFrontOccupant(this, laneIndexToEnter);
    }
    streetToEnter->setBackOccupant(this, laneIndexToEnter);
    // TODO change currentStreet to streetToEnter
    // TODO change the previous and next intersections
    // TODO request influences
    streetToEnter->requestInfluences(this);
    // TODO
}

void Vehicle::receiveInfluence(const Influence* incomingInfluence) {
    if (addIncomingInfluence(incomingInfluence)) {
        if (incomingInfluence->getType() == STOP) {
            setIsStopped(true);
        } else if (incomingInfluence->getType() == LIMIT) {
            setIsLimited(true);
        }
    }
}

void Vehicle::accident() {
}

void Vehicle::onWrite(std::ofstream &ofstream) {
    if (ofstream.is_open()) {

        ofstream << "\n#################\n";

        std::string underway = "not underway.";
        if (isUnderway()) underway = "is underway (" + getPrevIntersection()->getName() + "->" +
                                     getNextIntersection()->getName() + ")\n";

        ofstream << underway;
        std::stringstream ss;
        ss << getProgress();
        ofstream << "progress: " << ss.str() << "\n";

        std::string previousVehicle = "false";
        if (getPrevVehicle() != nullptr) previousVehicle = "true";
        ofstream << "isBack: " << previousVehicle << "\n";

        std::string nextVehicle = "false";
        if (getNextVehicle() != nullptr) previousVehicle = "true";
        ofstream << "isFront: " << nextVehicle << "\n";
        std::string isstopped = "false";
        if (isStopped()) isstopped = "true";
        ofstream << "isStopped: " << isstopped;
        ofstream << "\n#################\n";
    } else {
        std::cout << "File was not open" << std::endl;
    }
}








// getters and setters

vehicleClass Vehicle::getClass() const {
    return _class;
}

void Vehicle::setClass(vehicleClass vClass) {
    _class = vClass;
}

const std::vector<const Influence *> &Vehicle::getIncomingInfluences() const {
    return _incomingInfluences;
}
// TODO   add the reroute signal to the incoming influences when a car enters an intersections entrant list,
//  and the intersection is unavailable
bool Vehicle::addIncomingInfluence(const Influence * incomingInfluence) {
    
    if ((incomingInfluence->getType() == STOP  and !isStopped()) or
        (incomingInfluence->getType() == LIMIT and !isLimited())) {
        _incomingInfluences.emplace_back(incomingInfluence);
        return true;
    } else if (incomingInfluence->getType() == REROUTE) {
        return true;
    }
    return false;
}
void Vehicle::removeIncomingInfluence(const Influence * toDeleteInfluence) {
    std::vector<const Influence*> newVector;
    for (const Influence* influence : getIncomingInfluences()) {
        if (influence != toDeleteInfluence) {
            newVector.emplace_back(influence);
        } else {
            if (influence->getType() == STOP) {
                setIsStopped(false);
            } else if (influence->getType() == LIMIT) {
                setIsLimited(false);
            }
        }
    }
    _incomingInfluences = newVector;
}

bool Vehicle::isLimited() const {
    return _isLimited;
}
void Vehicle::setIsLimited(bool isLimited) {
    _isLimited = isLimited;
}

bool Vehicle::isStopped() const {
    return _isStopped;
}
void Vehicle::setIsStopped(bool isStopped) {
    _isStopped = isStopped;
}

Intersection *Vehicle::getStartIntersection() const {
    return _startIntersection;
}
void Vehicle::setStartIntersection(Intersection *startIntersection) {
    _startIntersection = startIntersection;
}

Intersection *Vehicle::getEndIntersection() const {
    return _endIntersection;
}
void Vehicle::setEndIntersection(Intersection *endIntersection) {
    _endIntersection = endIntersection;
}

Street *Vehicle::getCurrentStreet() const {
    return _currentStreet;
}
void Vehicle::setCurrentStreet(Street *currentStreet) {
    _currentStreet = currentStreet;
}

bool Vehicle::isUnderway() const {
    return _underway;
}
void Vehicle::setUnderway(bool underway) {
    _underway = underway;
}

int Vehicle::getProgress() const {
    return _progress;
}
void Vehicle::setProgress(int progress) {
    _progress = progress;
}

Vehicle *Vehicle::getNextVehicle() const {
    return _nextVehicle;
}
void Vehicle::setNextVehicle(Vehicle *nextVehicle) {
    _nextVehicle = nextVehicle;
}

Vehicle *Vehicle::getPrevVehicle() const {
    return _prevVehicle;
}
void Vehicle::setPrevVehicle(Vehicle *prevVehicle) {
    _prevVehicle = prevVehicle;
}

Intersection *Vehicle::getPrevIntersection() const {
    return _prevIntersection;
}
void Vehicle::setPrevIntersection(Intersection *prevIntersection) {
    _prevIntersection = prevIntersection;
}

Intersection *Vehicle::getNextIntersection() const {
    return _nextIntersection;
}
void Vehicle::setNextIntersection(Intersection *nextIntersection) {
    _nextIntersection = nextIntersection;
}

Street *Vehicle::getNextStreet() const {
    return _nextStreet;
}
void Vehicle::setNextStreet(Street *nextStreet) {
    _nextStreet = nextStreet;
}

