//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_TRANSPORTVEHICLE_H
#define TRAFFICSIMULATOR_TRANSPORTVEHICLE_H

#include "Vehicle.h"

class TransportVehicle : public Vehicle {

    std::vector<Intersection*> _preDeterminedRoute;

public:
    TransportVehicle(std::string licensePlate);

    void emitInfluence() const override;
};


#endif //TRAFFICSIMULATOR_TRANSPORTVEHICLE_H
