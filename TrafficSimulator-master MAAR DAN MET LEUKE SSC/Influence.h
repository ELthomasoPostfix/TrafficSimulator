//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_INFLUENCE_H
#define TRAFFICSIMULATOR_INFLUENCE_H

#include "Util.h"

class Simulation;

// the influence types represent the types of situations that may occur during transit
// (1) LIMIT : the vehicle needs to limit it's speed when driving trough a street because of some influence (speed limit, traffic jam)
// (2) STOP  : the vehicle must stop in its tracks due to some influence (police car, traffic accident, traffic light)

enum influenceType {LIMIT, STOP, REROUTE};

class Influence {
    influenceType _type;

    // the argument describes extra information about the influence
    // examples:
    // (1) car accident : the int describes the location (progress) on the road where it happened
    // (2) traffic light: the int describes the exact location from where on the STOP signal will take effect
    // (3) speed limit  : the int describes the speed limit in the street, speed limit is applied in the getSpeed() function
    // of a vehicle. The progress of a vehicle is in function addProgress() (not adjustProgress())
    // relies on the getSpeed() function
    int _argument;
public:
    Influence(influenceType type);


    // getters and setters

    influenceType getType() const;

    int getArgument() const;

    void setArgument(int argument);

    int toScore() const;
    };


#endif //TRAFFICSIMULATOR_INFLUENCE_H
