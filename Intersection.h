//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_INTERSECTION_H
#define TRAFFICSIMULATOR_INTERSECTION_H

#include "Street.h"

// The intersection functions as a cellular automaton state
class Intersection {

    std::string _name;

    int _trafficScore;

    std::vector<Street*> _streets;
public:
    Intersection(std::string  name);


    // getters and setters
    const std::string& getName() const;


    int getTrafficScore() const;
    void setTrafficScore(int trafficScore);
    void adjustTrafficScore(int additional);    // additional may be either positive or negative


    const std::vector<Street *> &getStreets() const;
    bool addStreet(Street* newStreet);

};


#endif //TRAFFICSIMULATOR_INTERSECTION_H
