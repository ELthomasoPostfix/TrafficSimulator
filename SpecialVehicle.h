//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_SPECIALVEHICLE_H
#define TRAFFICSIMULATOR_SPECIALVEHICLE_H

#include "Vehicle.h"

// necessary : retrieve a Influence*
class Simulation;

class SpecialVehicle : public Vehicle {

    // all influences caused by vehicles
    // if a normal vehicle has a traffic accident, then even they won't be able to emit Influences
    std::vector<Influence*> _outgoingInfluences;

public:
    SpecialVehicle();

    void InstantiateInfluence();

    void emitInfluence() override;

};


#endif //TRAFFICSIMULATOR_SPECIALVEHICLE_H
