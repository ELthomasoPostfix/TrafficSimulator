//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Intersection.h"

#include <utility>

Intersection::Intersection(std::string  name): _name(std::move(name)) {
    _trafficScore = 0;
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


