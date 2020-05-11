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
    _frontOccupant[index] = frontOccupant;
}

Vehicle *Street::getBackOccupant(int index) const {
    return _backOccupant[index];
}
void Street::setBackOccupant(Vehicle *backOccupant, int index) {
    _backOccupant[index] = backOccupant;
}

bool Street::isTwoWay() const {
    return _isTwoWay;
}
void Street::setIsTwoWay(bool isTwoWay) {
    _isTwoWay = isTwoWay;
}

