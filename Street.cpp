//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Street.h"
#include "Intersection.h"

Street::Street(Intersection* prev, Intersection* next, streetType type) : _prevIntersection(prev), _nextIntersection(next),
                                                                          _type(type) {}





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


const std::vector<Vehicle *> &Street::getOccupants() const {
    return _occupants;
}


const std::vector<Vehicle *> &Street::getEntrants() const {
    return _entrants;
}


Vehicle *Street::getFrontOccupant() const {
    return _frontOccupant;
}
void Street::setFrontOccupant(Vehicle *frontOccupant) {
    _frontOccupant = frontOccupant;
}

Vehicle *Street::getBackOccupant() const {
    return _backOccupant;
}
void Street::setBackOccupant(Vehicle *backOccupant) {
    _backOccupant = backOccupant;
}

