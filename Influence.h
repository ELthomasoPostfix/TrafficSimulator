//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_INFLUENCE_H
#define TRAFFICSIMULATOR_INFLUENCE_H

#include "string"
#include <fstream>
#include "sstream"
#include "iostream"

// the influence types represent the types of situations that may occur during transit
// (1) LIMIT : the vehicle needs to limit it's speed when driving trough a street because of some influence (speed limit, traffic jam)
// (2) STOP  : the vehicle must stop in its tracks due to some influence (police car, traffic accident, traffic light)

enum influenceType {LIMIT, STOP, REROUTE};

class Influence {
    influenceType _type;

public:
    Influence(influenceType type);


    // getters and setters

    influenceType getType() const;
};


#endif //TRAFFICSIMULATOR_INFLUENCE_H
