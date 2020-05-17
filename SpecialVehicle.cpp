//
// Created by Thomas Gueutal on 10.05.20.
//

#include "SpecialVehicle.h"
#include "Street.h"
#include "Intersection.h"

SpecialVehicle::SpecialVehicle(const std::string& licensePlate) : Vehicle(special, licensePlate) {
    InstantiateInfluence();
}

void SpecialVehicle::InstantiateInfluence() {

}

void SpecialVehicle::emitInfluence() {
    // get the lane the vehicle is currently in
    const int enteredLane = getPrevIntersection()->laneIndexWhenLeaving(getCurrentStreet());
    if (enteredLane != -1) {
        std::vector<Vehicle *> currentLane = getCurrentStreet()->getLanes()[enteredLane];
        // send out the STOP signal to the entire lane in the street
        for (Vehicle *laneOccupant : currentLane) {
            laneOccupant->receiveInfluence(_outgoingInfluences[0]);
        }
    }
}

void SpecialVehicle::addOutgoingInfluence(const Influence* outgoingInfluence) {
    _outgoingInfluences.emplace_back(outgoingInfluence);
}
