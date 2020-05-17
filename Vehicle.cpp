//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Vehicle.h"
#include "Street.h"
#include "Intersection.h"
#include "Simulation.h"

Vehicle::Vehicle(vehicleClass vClass, const std::string licensePlate) : _class(vClass), _licensePlate(licensePlate) {
    _isLimited = false;
    _isStopped = false;
}

void Vehicle::drive(std::ofstream& ofstream) {
    if (ofstream.is_open()) {
        // the exact location from whereon a vehicle's possessed STOP signal will take effect
        const double effectiveSTOPLocation = getArgument(STOP) - Simulation::getEffectiveSTOPdistance();
        // vehicle is not under influence of a STOP signal
        if (!isStopped() or (isStopped() and getProgress() < effectiveSTOPLocation and effectiveSTOPLocation >= 0)) {
            const double decisionStartPoint = effectiveSTOPLocation - Simulation::getDecisionBufferLength();
            if (getProgress() >= decisionStartPoint) {
                // TODO vehicle may decide to adjust its path from here on out
                const std::vector<Street *> &leavingStreets = getNextIntersection()->getAllLeavingStreets();
                Street *randomNextStreet;
                int rand;
                int counter = 0; // TODO @@@@@@@@@@@@@@@@@@@@@@@@@@@@éé delete ##################################
                do {;
                rand = random() % leavingStreets.size();
                randomNextStreet = leavingStreets[rand];
                std::cout << "chose randomly (" << counter << ") (" << randomNextStreet->typeToName() << ")" << std::endl;
                counter++;
                } while (!Simulation::vehicleTypeCanEnterStreetType(getClass(), randomNextStreet->getType()));
                setNextStreet(randomNextStreet);
            }
            // TODO replace '100'% ????
            if (getProgress() == 100) {
                if (isUnderway()) setUnderway(false);
                // TODO vehicle is at an intersection,
                //  do decision making

                // TODO decision making done, move into new street
                enterStreet(ofstream);
            // simply move the car, no decisions made
            } else {
                adjustProgress(ofstream);
            }
        } else {
            ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
                     << " is " << "stopped: \"" << Util::boolToString(isStopped());
            if (isStopped()) {
                ofstream << "\"  (distance to effective STOP location " << effectiveSTOPLocation-getProgress() << "/"
                         << "distance to STOP signal location " << Simulation::getEffectiveSTOPdistance() << ")";
            }
            ofstream << "\n";
        }
    } else {
        std::cerr << "file was not opened." << std::endl;
    }
}

void Vehicle::adjustProgress(std::ofstream& ofstream) {
    const double progress = getProgress();
    if (getNextVehicle() != nullptr) {
        // vars describing position between cars in terms of progress
        const double distanceToNextCar = getNextVehicle()->getProgress() - progress;
        const double minCarDistance = Simulation::getMinCarDistance();     // default 1m
        const double maxAllowedDriveDistance = distanceToNextCar - minCarDistance;
        // the room between cars is at least 2x greater then the min car distance required
        if (maxAllowedDriveDistance >= minCarDistance) {     // c1 __(2m)__ c2   ==>  c1 __(1m)__ c2
            const double maxPossibleDriveDistance = getMaxDriveDistance();  // the maximum possible drive distance in 1 time unit
            if (maxAllowedDriveDistance >= maxPossibleDriveDistance) {
                addProgress(maxPossibleDriveDistance);
            } else {
                addProgress(maxAllowedDriveDistance);
                std::cerr << maxAllowedDriveDistance << std::endl;
            }
            // the room between cars is lesser than the min car distance required
            // ==> move as close to the next car as possible
        } else {
            // the two cars are at less than 2x min car distance apart, but more than to 1x min car distance apart
            if (maxAllowedDriveDistance > 0) {              // c1 __(1.5m)__ c2  ==>  c1 __(1m)__ c2
                addProgress(maxAllowedDriveDistance);
            // the two cars are less than 1x min car distance apart
            } else {
                // if there is a previous car, make sure it's not in the way of backing up
                if (getPrevVehicle() != nullptr) {

                    const double distanceToPreviousCar = progress - getPrevVehicle()->getProgress();
                    // maxAllowedDriveDistance is negative: you are closer to the car than is allowed, drive backwards
                    const double distanceToDriveBackwards = minCarDistance - maxAllowedDriveDistance;
                    const double freeZone = distanceToPreviousCar - minCarDistance;
                    if (freeZone >= distanceToDriveBackwards) {
                        // maxCarDist is negative, so drive backwards
                        addProgress(maxAllowedDriveDistance);
                    }
                // no previous car in the way, always back up
                } else {
                    addProgress(maxAllowedDriveDistance);
                }
            }
        }
    } else {
        addProgress(getMaxDriveDistance());
    }
    ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
             << " has travelled " << getProgress()-progress << "  (" << getProgress() << "%)" << " max distance: "
             << getMaxDriveDistance() << "\n";
}



void Vehicle::emitInfluence() {

}

void Vehicle::enterStreet(std::ofstream& ofstream) {
    clearIncomingInfluences();
    Street* currentStreet = getCurrentStreet();
    Intersection* currentIntersection = getNextIntersection();
    Street *streetToEnter = getNextStreet();
    Vehicle* prevVehicle = getPrevVehicle();
    bool streetBeginningOccupied = false;
    // TODO remove vehicle from _entrants
    // TODO move vehicle to occupants
    ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName();
    // the index of the correct lane to enter in the next street
    // next intersection, because the next intersection is the intersection it enters the new street from
    // when leaving is about leaving the intersection, it gets the correct lane to leave INTO/WITH
    int laneIndexToLeaveThrough = currentIntersection->laneIndexWhenLeaving(streetToEnter);
    if (streetToEnter->getFrontOccupant(laneIndexToLeaveThrough) == nullptr) {
        // take care of the current vehicle and it's prev and next vehicle pointer members only
        streetToEnter->setFrontOccupant(this, laneIndexToLeaveThrough);
    // a vehicle that wants to enter a new street cannot enter if another vehicle is already at progress 0
    } else if (streetToEnter->getBackOccupant(laneIndexToLeaveThrough)->getProgress() >= Simulation::getMinCarDistance()) {
        streetToEnter->setBackOccupant(this, laneIndexToLeaveThrough);

    } else {
        streetBeginningOccupied = true;
    }
    if (!streetBeginningOccupied) {
        streetToEnter->requestInfluences(this);

        // adjust other current vehicle properties
        setCurrentStreet(streetToEnter);
        setNextStreet(nullptr);

        setPrevIntersection(currentIntersection);
        setNextIntersection(streetToEnter->getOtherIntersection(currentIntersection));

        // resolve the vehicle that used to be the prev vehicle of the vehicle that just entered a Street
        if (prevVehicle != nullptr) {

            // the index of the lane the vehicle arrived in the intersection through
            int laneIndexWhenEntered = currentIntersection->laneIndexWhenEntering(currentStreet);

            currentStreet->setFrontOccupant(prevVehicle, laneIndexWhenEntered);
            prevVehicle->setNextVehicle(nullptr);

        }
        setProgress(0);
        ofstream << " has entered a street (" << streetToEnter->typeToName() << ")\n and is now underway from " << getPrevIntersection()->getName()
                 << " to " << getNextIntersection()->getName() << "  (" << getProgress() << "%)\n";
    } else {
        ofstream << " tried to enter a street (" << streetToEnter->typeToName() << ") from " << getPrevIntersection()->getName()
                 << " to " << getNextIntersection()->getName() << "\n but the entrance to the street was occupied ("
                 << getProgress() << "%)\n";
    }

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

void Vehicle::onWrite(std::ofstream &ofstream) const {
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
int Vehicle::getArgument(const influenceType influenceType) const {
    for (const Influence* influence : getIncomingInfluences()) {
        if (influence->getType() == influenceType) return influence->getArgument();
    }
    return -1;
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
void Vehicle::clearIncomingInfluences() {
    _incomingInfluences.clear();
    setIsLimited(false);
    setIsStopped(false);
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

double Vehicle::getProgress() const {
    return _progress;
}
void Vehicle::setProgress(double progress) {
    _progress = progress;
}
void Vehicle::addProgress(double additionalProgress) {
    // limit the mex progress of vehicles to 100
    if (additionalProgress > (100-_progress)) {
        _progress = 100;
    } else {
        _progress += additionalProgress;
    }
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

double Vehicle::getSpeed() const {
    if (isLimited()) {
        return getArgument(LIMIT);  // return the speed limit
    } else {
        return _speed;
    }
}
void Vehicle::setSpeed(const double speed) {
    _speed = speed;
}
double Vehicle::getMaxDriveDistance() const {
    return getSpeed()*0.1;
}

const std::string &Vehicle::getLicensePlate() const {
    return _licensePlate;
}




