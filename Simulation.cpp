//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Simulation.h"

Simulation::Simulation() {
    // we only keep a single LIMIT, STOP and REROUTE object in the entire simulation/network
    instantiateInfluences();
    _totalTimeSimulated = 0;
}

void Simulation::instantiateInfluences() {

    Influence* stop    = new Influence(STOP);
    stop->setArgument(100 + getEffectiveSTOPdistance());
    Influence* reroute = new Influence(REROUTE);
    allInfluences.emplace_back(stop);
    allInfluences.emplace_back(reroute);
}

const Influence *Simulation::getInfluence(influenceType IType) const {
    for (Influence* influence : allInfluences) {
        if (influence->getType() == IType) return influence;
    }
    return nullptr;
}




// getters and setters

const std::vector<Influence *> &Simulation::getAllInfluences() const {
    return allInfluences;
}
bool Simulation::addInfluence(Influence *newInfluence) {
    for (Influence* influence : getAllInfluences()) {
        // the influence already exists in the simulation, don't add it
        if (influence->getType() == newInfluence->getType()) {
            return false;
        }
    }
    // the influence does not yet exist in the simulation, add it
    allInfluences.emplace_back(newInfluence);
    return true;
}

int Simulation::getTotalTimeSimulated() const {
    return _totalTimeSimulated;
}
void Simulation::setTotalTimeSimulated(int totalTimeSimulated) {
    _totalTimeSimulated = totalTimeSimulated;
}
void Simulation::addTotalTimeSimulated(int additionalTime) {
    _totalTimeSimulated += additionalTime;
}

const int Simulation::getMinCarDistance() {
    return _minCarDistance;
}

const int Simulation::getTrafficLightMaxCount() {
    return _trafficLightMaxCount;
}

const int Simulation::getEffectiveSTOPdistance() {
    return _effectiveSTOPdistance;
}

const int Simulation::getDecisionBufferLength() {
    return _decisionBufferLength;
}

const bool Simulation::vehicleTypeCanEnterStreetType(const vehicleClass &vehicleClass, const streetType &streetType) {
    if (vehicleClass == personal and streetType == B) { // TODO delete !!!
        return true;
    }
    /*
     if (streetType == A or streetType == B) {
        return true;
    } else if (streetType == T and (vehicleClass == transport)) {
        return true;
    }
     */
    return false;
}


int Simulation::getTotalSpawnedVehicles() const {
    return _totalSpawnedVehicles;
}
void Simulation::incrementTotalSpawnedVehicles() {
    _totalSpawnedVehicles++;
}


