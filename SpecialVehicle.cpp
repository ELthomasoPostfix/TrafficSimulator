//
// Created by Thomas Gueutal on 10.05.20.
//

#include "SpecialVehicle.h"

SpecialVehicle::SpecialVehicle(const std::string licensePlate) : Vehicle(special, licensePlate) {
    InstantiateInfluence();
}

void SpecialVehicle::InstantiateInfluence() {

}

void SpecialVehicle::emitInfluence() {

}
