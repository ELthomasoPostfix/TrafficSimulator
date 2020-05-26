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


Vehicle::~Vehicle() {
}



Street* Vehicle::chooseRandomStreet() {
    const std::vector<Street *> &leavingStreets = getNextIntersection()->getAllLeavingStreets();
    if (vehicleCanLeaveIntersection(leavingStreets)) {
        Street *randomNextStreet;
        int rand;
        // choose a random street
        do {
            rand = random() % leavingStreets.size();
            randomNextStreet = leavingStreets[rand];
        } while (!Simulation::vehicleTypeCanEnterStreetType(getClass(), randomNextStreet->getType()));
        setNextStreet(randomNextStreet);
        return randomNextStreet;
    // there is no viable street to leave the intersection through
    } else {
        std::cerr << "There is no viable street to leave the intersection " << getNextIntersection()->getName()
                  << " through. The vehicle " << getLicensePlate() << " (" << getClass() << ") from "
                  << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
                  << " (" << getCurrentStreet()->typeToName() << ", " << Util::isTwoWayToString(getCurrentStreet()->isTwoWay()) << ") "
                  << " is stuck and needs to change vehicle Class." << std::endl << "An might occur as the result returned"
                  << " by this 'chooseRandomStreet()' function is nullptr." << std::endl;
        return nullptr;
    }
}

void Vehicle::drive(std::ofstream& ofstream) {
    if (ofstream.is_open()) {
        const double argument = getLowestRelevantArgument(STOP);
        // the exact location from whereon a vehicle's possessed STOP signal will take effect
        const double effectiveSTOPLocation = argument - Simulation::getEffectiveSTOPdistance();
        // vehicle is not under influence of a STOP signal or it is not in range of the STOP signal
        // if the argument is -2, then the argument affects the whole street/lane
        if (mayDrive(effectiveSTOPLocation, argument)) {
            const double decisionStartPoint = effectiveSTOPLocation - Simulation::getDecisionBufferLength();
            if (getProgress() >= decisionStartPoint) {
                // TODO vehicle may decide to adjust its path from here on out
                alterPath();
            }
            // the base STOP argument, which is the same as the argument of all traffic lights,
            // is  streetLength + minCarDistance
            // ==> a vehicle will always stop minCarDistance away from the stop signal
            // ==> the vehicle will always stop at streetLength
            if (getProgress() == Simulation::getStreetLength()) {
                if (isUnderway()) setUnderway(false);
                // TODO vehicle is at an intersection,
                //  decision making already done, move into new street
                enterStreet(ofstream);
            // simply move the car, no decisions made
            } else {
                adjustProgress(ofstream);
            }
        } else {
            addSTOPMessage(ofstream, effectiveSTOPLocation);
        }
    } else {
        std::cerr << "file was not open upon calling drive. Please open the ofstream before calling drive." << std::endl;
    }
}

void Vehicle::alterPath() {
    // this vehicle had already chosen a next street, remove this vehicle from its entrants
    // before we change the next street of this vehicle
    leaveEntrantsList();

    // choose a new next street
    Street* chosenStreet = chooseRandomStreet();
    // add this vehicle to the entrants of the chosen street
    chosenStreet->addEntrant(getNextIntersection()->laneIndexWhenLeaving(chosenStreet), this);
    // request both the street's influences and the possible traffic light influence
    chosenStreet->requestEntrantInfluences(this);
}
void Vehicle::leaveEntrantsList() const {
    if (getNextStreet() != nullptr) {
        const int laneIndexWhenLeaving = getNextIntersection()->laneIndexWhenLeaving(getNextStreet());
        const std::vector<Vehicle*>& entrantLane = getNextStreet()->getEntrants()[laneIndexWhenLeaving];
        if (!entrantLane.empty()) {
            getNextStreet()->removeEntrant(laneIndexWhenLeaving,this);
        }
    }
}
void Vehicle::addSTOPMessage(std::ofstream &ofstream, const double effectiveSTOPLocation) const {
    ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
             << " (" << getCurrentStreet()->typeToName() << ", " << getCurrentStreet()->getTwoWayString()
             << ") is " << "stopped: \"" << Util::boolToString(isStopped());
    if (isStopped()) {
        ofstream << "\"  (distance to effective STOP location: ";
        if (effectiveSTOPLocation-getProgress() >= 0) {
            ofstream << effectiveSTOPLocation - getProgress();
        } else {
            ofstream << "no STOP signal";
        }
        ofstream << "/" << "distance to STOP signal location " << Simulation::getEffectiveSTOPdistance() << ")\n";
    } else {
        ofstream << "\" (The vehicle cannot drive for some reason)\n";
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
            // the vehicle cannot drive as far as it is allowed to in a single time unit
            if (maxAllowedDriveDistance >= maxPossibleDriveDistance) {
                addProgress(maxPossibleDriveDistance);
            // the vehicle is able to drive either exactly or further than it is allowed to in a single time unit
            // it must drive only as far as it is allowed to
            } else {
                addProgress(maxAllowedDriveDistance);
            }
        // the room between cars is lesser than the min car distance required
        // ==> move as close to the next car as possible
        // OR the next car is BEHIND this car:
        // this can happen if a car is instantiated with a progress higher than the backOccupant, but lower than
        // the frontOccupant
        } else {
            // the two cars are at less than 2x min car distance apart, but more than to 1x min car distance apart
            if (maxAllowedDriveDistance > 0) {              // c1 __(1.5m)__ c2  ==>  c1 __(1m)__ c2
                addProgress(maxAllowedDriveDistance);
            // the two cars are less than 1x min car distance apart
            // OR this vehicle is behind its next vehicle
            } else {
                // if there is a previous car, make sure it's not in the way of backing up
                if (getPrevVehicle() != nullptr) {
                    // make all vars neg
                    const double distanceToPreviousVehicle = getPrevVehicle()->getProgress() - progress;
                    const double maxAllowedDriveDistanceBack = distanceToPreviousVehicle + minCarDistance;
                    // maxAllowedDriveDistance is negative: this vehicle is closer to the car than is allowed, drive backwards
                    if (maxAllowedDriveDistanceBack <= maxAllowedDriveDistance) {
                        // there is enough space between this vehicle and previous vehicle
                        // to drive the required amount back

                        // only if the distance to the next car is less than the minCarDistance,,
                        // drive backwards
                        if (maxAllowedDriveDistance > -minCarDistance) {
                            addProgress(maxAllowedDriveDistance);
                        }
                    } else {
                        // there is not enough distance between this vehicle and previous vehicle
                        // to drive the required amount back. Drive back as far as allowed
                        addProgress(maxAllowedDriveDistanceBack);
                    }
                // no previous car in the way, always back up
                } else {
                    // the vehicle wants to back up less than its maximum possible dive distance
                    // in a single time unit, so let it
                    if (maxAllowedDriveDistance > -getMaxDriveDistance()) {
                        addProgress(maxAllowedDriveDistance);
                    // the vehicle wants to back up more than its maximum possible drive distance
                    // in a single time unit, limit it to its maximum posiible drive distance
                    } else {
                        addProgress(getMaxDriveDistance());
                    }
                }
            }
        }
    } else {
        addProgress(getMaxDriveDistance());
    }
    addProgressMessage(ofstream, progress);
}
void Vehicle::addProgressMessage(std::ofstream &ofstream, const double progress) const {
    ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
             << " (" << getCurrentStreet()->typeToName() << ", " << getCurrentStreet()->getTwoWayString()
             << ") has travelled " << getProgress()-progress << "  (" << getProgress()/Simulation::getStreetLength()*100
             << "%)" << " max distance: " << getMaxDriveDistance() << "  (LIMIT: ";
    if (isLimited()) {
        ofstream << getArgument(LIMIT);
    } else {
        ofstream << "None";
    }
    ofstream << ")\n";
}




void Vehicle::emitInfluence() const {

}
void Vehicle::undoSiren() const {

}


void Vehicle::addOutgoingInfluence(const Influence* outgoingInfluence) {

}


void Vehicle::enterStreet(std::ofstream& ofstream) {
    clearIncomingInfluences();
    Street* currentStreet = getCurrentStreet();
    Intersection* currentIntersection = getNextIntersection();
    Street *streetToEnter = getNextStreet();
    Vehicle* prevVehicle = getPrevVehicle();
    bool streetBeginningOccupied = false;

    leaveEntrantsList();

    ofstream << "Vehicle " << getLicensePlate() << " from " << getPrevIntersection()->getName() << " to "
             << getNextIntersection()->getName() << " (" << getCurrentStreet()->typeToName() << ", "
             << getCurrentStreet()->getTwoWayString() << ") ";
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
        setUnderway(true);
        // remove this vehicle from the previous street's lanes
        const int laneWhenEntering = currentIntersection->laneIndexWhenEntering(currentStreet);
        currentStreet->removeFromLane(this, laneWhenEntering);

        streetToEnter->requestInfluences(this);

        // remove the siren STOP signal/influence from other vehicles
        if (getClass() == special) {
            undoSiren();
        }


        // adjust other current vehicle properties
        setCurrentStreet(streetToEnter);
        setNextStreet(nullptr);

        // if special vehicle and siren is active, when entering a new street,
        // emit the STOP signal to the new current street
        if (getClass() == special) {
            emitInfluence();
        }

        setPrevIntersection(currentIntersection);
        setNextIntersection(streetToEnter->getOtherIntersection(currentIntersection));

        // the index of the lane with which the vehicle arrived in the intersection
        int laneIndexWhenEntered = currentIntersection->laneIndexWhenEntering(currentStreet);

        // resolve the vehicle that used to be the prev vehicle of the vehicle that just entered a Street
        if (prevVehicle != nullptr) {

            currentStreet->setFrontOccupant(prevVehicle, laneIndexWhenEntered);
            prevVehicle->setNextVehicle(nullptr);

        } else {
            currentStreet->setFrontNull(laneIndexWhenEntered);
            currentStreet->setBackNull(laneIndexWhenEntered);
        }
        setProgress(0);
        ofstream << " has entered a street\n and is now underway from " << getPrevIntersection()->getName()
                 << " to " << getNextIntersection()->getName() << " (" << streetToEnter->typeToName() << ", "
                 << streetToEnter->getTwoWayString() << ")" << "  (" << getProgress()/Simulation::getStreetLength()*100 << "%)\n";
    } else {
        setUnderway(false);
        ofstream << " tried to enter a street (" << streetToEnter->typeToName() << ", " << streetToEnter->getTwoWayString()
                 << ") from " << getPrevIntersection()->getName() << " to " << getNextIntersection()->getName()
                 << "\n but the entrance to the street was occupied (" << getProgress()/Simulation::getStreetLength()*100 << "%)\n";
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
    const int accident = random()%8000;
    if (accident >= 501 and accident <= 520) {
        std::cout << "accident happened (" << accident << ")" << std::endl;
        // TODO spawn a special vehicle that goes to the scene?
        //  or on drive of any special vehicle, send them to the scene??
    }
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


// boolean functions

bool Vehicle::mayDrive(const double effectiveSTOPLocation, const double argument) const {
    // argument == -1, invalid or non existing influence
    // argument == -2, a STOP signal that directly affects the whole street/lane
    return  !isStopped() or (isStopped() and getProgress() < effectiveSTOPLocation and effectiveSTOPLocation >= 0 and
    (argument != -1 and argument != -2));
}

bool Vehicle::vehicleCanLeaveIntersection(const std::vector<Street*>& leavingStreets) const {
    // if "this" vehicle can leave the (hypothetical) intersection through
    // any of the streets of the "leavingStreets list", return true
    for (const Street* street : leavingStreets) {
        if (Simulation::vehicleTypeCanEnterStreetType(getClass(), street->getType())) return true;
    }
    // no street could be used to leave through
    return false;
}

bool Vehicle::isFront() const {
    return this == getCurrentStreet()->getFrontOccupant(getNextIntersection()->laneIndexWhenEntering(getCurrentStreet()));;
}





// getters and setters

vehicleClass Vehicle::getClass() const {
    return _class;
}
void Vehicle::setClass(vehicleClass vClass) {
    _class = vClass;
}
std::string Vehicle::classToName() const {
    switch (getClass()) {
        case personal:
            return "personal";
        case transport:
            return "transport";
        case special:
            return "special";
    }
}
vehicleClass Vehicle::nameToClass(const std::string& name) {
    if (name == "personal") {
        return personal;
    } else if (name == "special") {
        return special;
    } else if (name == "transport") {
        return transport;
    }
    // if no class recognised, return personal by default
    return personal;
}
vehicleClass Vehicle::intToClass(const int vClass) {
    switch (vClass) {
        case 0:
            return personal;
        case 1:
            return special;
        case 2:
            return transport;
        default:
            return personal;
    }
}

const std::vector<const Influence *> &Vehicle::getIncomingInfluences() const {
    return _incomingInfluences;
}
double Vehicle::getArgument(const influenceType influenceType) const {
    for (const Influence* influence : getIncomingInfluences()) {
        if (influence->getType() == influenceType) return influence->getArgument();
    }
    return -1;
}
double Vehicle::getLowestRelevantArgument(influenceType influenceType) const {
    double lowestRelevantArgument = 200;    // any argument should be lower than 200
    for (const Influence* influence : getIncomingInfluences()) {
        // the influence is a STOP signal, it is lower than the current lowest argument and is relevant
        // ==> even negative arguments can be returned, as is the case for special vehicle's STOP signal
        bool stop = influence->getType() == STOP and influence->getArgument() < lowestRelevantArgument and
                    influence->getArgument() >= getProgress()+Simulation::getMinCarDistance();
        // the influence is a LIMIT signal, and is lower than the current lowest argument
        bool limit = influence->getType() == LIMIT and influence->getArgument() < lowestRelevantArgument;

        if (influence->getType() == influenceType and (stop or limit)) {
            lowestRelevantArgument = influence->getArgument();
        }
    }
    if (lowestRelevantArgument != 200) {
        return lowestRelevantArgument;
    } else {
        return -1;
    }
}
const Influence* Vehicle::getIncomingInfluence(const influenceType& influenceType, const double argument) const {
    for (const Influence* influence : getIncomingInfluences()) {
        if (influence->getType() == influenceType and influence->getArgument() == argument) {
            return influence;
        }
    }
    return nullptr;
}
// TODO   add the reroute signal to the incoming influences when a car enters an intersections entrant list,
//  and the intersection is unavailable
bool Vehicle::addIncomingInfluence(const Influence * incomingInfluence) {
    if (incomingInfluence != nullptr) {
        // either a vehicle doesnt' possess the influence or it does, but it has a different argument from the new one
        if ((incomingInfluence->getType() == STOP  and (!isStopped() or
                                                        (isStopped() and getIncomingInfluence(STOP, incomingInfluence->getArgument()) == nullptr))) or
            (incomingInfluence->getType() == LIMIT and (!isLimited() or
                                                        isLimited() and getIncomingInfluence(LIMIT, incomingInfluence->getArgument()) == nullptr))) {
            _incomingInfluences.emplace_back(incomingInfluence);
            return true;
        } else if (incomingInfluence->getType() == REROUTE) {
            return true;
        }
    }
    return false;
}
void Vehicle::removeIncomingInfluence(const Influence * toRemoveInfluence) {
    std::vector<const Influence*> newVector;
    for (const Influence* influence : getIncomingInfluences()) {
        if (influence != toRemoveInfluence) {
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


const std::vector<const Influence *> &Vehicle::getEntrantInfluences() const {
    return _entrantInfluences;
}
bool Vehicle::addEntrantInfluence(const Influence *entrantInfluence) {
    if (entrantInfluence != nullptr) {
        for (const Influence *influence : getEntrantInfluences()) {
            if (influence->getType() == entrantInfluence->getType() and
                influence->getArgument() == entrantInfluence->getArgument()) {
                return false;
            }
        }
        _entrantInfluences.emplace_back(entrantInfluence);
        return true;
    }
    return false;
}
void Vehicle::clearEntrantInfluences() {
    _entrantInfluences.clear();
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
    // limit the mex progress of vehicles to the streetLength
    if (additionalProgress > (Simulation::getStreetLength()-_progress)) {
        _progress = Simulation::getStreetLength();
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
        double limit = getArgument(LIMIT);  // return the speed limit
        if (limit < _speed) {
            return limit;
        } else {
            return _speed;
        }
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










