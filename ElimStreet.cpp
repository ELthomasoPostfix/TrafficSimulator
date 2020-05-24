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
    for (const Street* loop : *toCopyElimState->getLoops()) {
        _loops.emplace_back(loop);
    }
}

std::string ElimStreet::getEStreetName() const {
    std::string streetName;
    int loopIndex = 0;
    int loopCount = 0;
    std::string temp;
    const std::vector<const Street*>& loops = *getLoops();

    int intersecCounter = 0;

    // for each of the eliminated intersections
    for (const Intersection* intersection : *getIntersections()) {

        // while the current loop is connected to the current intersection
        while (loopIndex < loops.size() and loops[loopIndex]->getPrevIntersection() == intersection) {

            addLoopString(temp, streetName, loopIndex, loopCount, loops);
        }
        // add the loop name to the full name before potentially losing the brackets
        streetName += temp;
        // close the brackets if there were more than one loop after each other
        // the loops being part of a final ElimStreet, so a loop part of the minimised street
        if (loopCount > 1) {
            streetName += ")*";
        }
        if (loopCount > 0) {
            streetName += ", ";
        }
        // reset vars
        loopCount = 0;
        temp = "";

        streetName += intersection->getName();
        // add a comma after each intersection name except the last one
        if (intersecCounter < getIntersections()->size()-1) {
            streetName += ", ";
        }
        ++intersecCounter;
    }
    return streetName;
}

void ElimStreet::addLoopString(std::string &temp, std::string &streetName, int &loopIndex, int &loopCount,
                               const std::vector<const Street *> &loops) const {
    // brackets for if there are more than one loop after ech other
    if (loopCount > 0) {
        streetName += "(";
    }


    // plus for if there are more than one loop after each other
    if (loopCount > 0) {
        temp += " + ";
    }

    // if the loop has more than one intersection, it needs brackets
    bool needsBrackets = loops[loopIndex]->getIntersections()->size() > 1;
    if (needsBrackets) {
        temp += "(";
    }
    // the intersection names of the loop itself
    temp += loops[loopIndex]->getEStreetName();
    if (needsBrackets) {
        temp += ")";
    }
    // always add a * to a loop name
    temp += "*";

    ++loopCount;
    ++loopIndex;
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
void ElimStreet::addLoops(const std::vector<const Street *> &loops) {
    for (const Street* loop : loops) {
        addLoop(loop);
    }
}




