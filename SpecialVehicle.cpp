//
// Created by Thomas Gueutal on 10.05.20.
//

#include "SpecialVehicle.h"
#include "Street.h"
#include "Intersection.h"

SpecialVehicle::SpecialVehicle(const std::string& licensePlate) : Vehicle(special, licensePlate) {
    InstantiateInfluence(-2);
}

SpecialVehicle::~SpecialVehicle() {
    for (const Influence* outInfluence : _outgoingInfluences) {
        delete outInfluence;
    }
    _outgoingInfluences.clear();
}


void SpecialVehicle::InstantiateInfluence(const double argument) {
    Influence* siren = new Influence(STOP);
    siren->setArgument(argument);
    addOutgoingInfluence(siren);
}

void SpecialVehicle::emitInfluence() const {
    // get the lane the vehicle is currently in
    const int enteredLane = getPrevIntersection()->laneIndexWhenLeaving(getCurrentStreet());
    // the lane is valid/the vehicle could have entered it's current lane through the previous
    // intersection
    if (enteredLane != -1) {
        std::vector<Vehicle *> currentLane = getCurrentStreet()->getLanes()[enteredLane];
        // send out the STOP signal to the entire lane in the street
        for (Vehicle *laneOccupant : currentLane) {
            if (laneOccupant != this) {
                laneOccupant->receiveInfluence(_outgoingInfluences[0]);
            }
        }
        // add the outgoing influence to the street for any new entering vehicles
        getCurrentStreet()->addInfluence(_outgoingInfluences[0]);
    }
}
void SpecialVehicle::undoSiren() const {
    Street* currStreet = getCurrentStreet();

    // remove the siren STOP signal/influene from the streets itself
    currStreet->removeInfluence(getSiren());

    // the lane through which the vehicle will enter next intersection
    int laneWhenEntering = getNextIntersection()->laneIndexWhenEntering(currStreet);
    // remove the siren STOP signal/influence from the occupants of the street (of the correct lane)
    for (Vehicle* vehicle : currStreet->getLanes()[laneWhenEntering]) {
        vehicle->removeIncomingInfluence(getSiren());
    }
}

void SpecialVehicle::addOutgoingInfluence(const Influence* outgoingInfluence) {
    _outgoingInfluences.emplace_back(outgoingInfluence);
}

const Influence* SpecialVehicle::getSiren() const {
    for (const Influence* outgoingInfluence : _outgoingInfluences) {
        if (outgoingInfluence->getArgument() == -2 and outgoingInfluence->getType() == STOP) {
            return outgoingInfluence;
        }
    }
    return nullptr;
}


