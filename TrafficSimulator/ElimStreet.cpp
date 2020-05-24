//
// Created by Thomas Gueutal on 22.05.20.
//

#include "ElimStreet.h"

ElimStreet::ElimStreet(Intersection *prev, Intersection *next, streetType type) : Street(prev, next, type) {
    // any ElimStreet is treated as a one way road from start to finish, though its streets member
    // may contains twoWay streets
    setIsTwoWay(false);
}

ElimStreet::~ElimStreet() {
}



bool ElimStreet::isEStreet() {
    return isElimStreet;
}

void ElimStreet::copyPath(Street *toCopyElimState) {
    // copy the streets and intersections of the other ElimStreet into this ElimStreet
    for (const Street* street : *toCopyElimState->getStreets()) {
        _streets.emplace_back(street);
    }
    for (const Intersection* intersection : *toCopyElimState->getIntersections()) {
        _intersections.emplace_back(intersection);
    }
}

std::string ElimStreet::getEStreetName() const {
    std::string streetName;
    int loopIndex = 0;
    const std::vector<const Street*>& loops = *getLoops();

    for (const Intersection* intersection : *getIntersections()) {
/*
        const Street* loop = loops[loopIndex];
        // if the loop is connected to intersection
        if (loop->getPrevIntersection() == intersection) {
            if (loop->getIntersections()->size() > 1) {
                streetName += "(";
            }

            //addLoopString(streetName, loopIndex);

        }
*/
        streetName += intersection->getName();
        if (intersection != getIntersections()->back()) {
            streetName += ", ";
        }
    }
    return streetName;
}
void ElimStreet::addLoopString(std::string &streetName, const int loopIndex) const {

    const std::vector<const Street*>& loops = *getLoops();
    const Street* loop = loops[loopIndex];
    const std::vector<const Street*>& deepLoops = *loop->getLoops();

    std::string temp = loop->getEStreetName();
    const bool needsBrackets = temp.size() > 1;

    if (needsBrackets and loop == loops.front()) {
        streetName += "(";
    }
    streetName += temp;
    if (loop != loops.back()) {
        streetName += " + ";
    } else if (needsBrackets) {
        streetName += ")";
    }
    streetName += "*";
    streetName += ", ";
}

void ElimStreet::print() {
    std::cout << "streetName:  " << getEStreetName() << "   with prev " << getPrevIntersection()->getName()
              << " and next " << getNextIntersection()->getName() << std::endl;
}



// getters and setters

const std::vector<const Street *>* ElimStreet::getStreets() const {
    return &_streets;
}
void ElimStreet::addStreet(const Street* street) {
    if (street != nullptr) {
        _streets.emplace_back(street);
    }
}

const std::vector<const Intersection *>* ElimStreet::getIntersections() const {
    return &_intersections;
}
void ElimStreet::addIntersection(const Intersection* intersection) {
    if (intersection != nullptr) {
        _intersections.emplace_back(intersection);
    }
}

const std::vector<const Street *>* ElimStreet::getLoops() const {
    return &_loops;
}
void ElimStreet::setLoops(std::vector<const Street *> &loops) {
    _loops = loops;
}
void ElimStreet::addLoop(const Street *loop) {
    _loops.emplace_back(loop);
}





