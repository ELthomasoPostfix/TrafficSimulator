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
    stop->setArgument(Simulation::getStreetLength() + getEffectiveSTOPdistance());
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
    return minCarDistance;
}
void Simulation::setMinCarDistance(int distance) {
    minCarDistance = distance;
    Simulation::setEffectiveSTOPdistance(distance);
}

const int Simulation::getTrafficLightMaxCount() {
    return trafficLightMaxCount;
}
void Simulation::setTrafficLightMaxCount(int count) {
    trafficLightMaxCount = count;
}

const int Simulation::getEffectiveSTOPdistance() {
    return effectiveSTOPdistance;
}
void Simulation::setEffectiveSTOPdistance(int distance) {
    effectiveSTOPdistance = distance;
}

const int Simulation::getDecisionBufferLength() {
    return decisionBufferLength;
}
void Simulation::setDecisionBufferLength(int length) {
    decisionBufferLength = length;
}

const bool Simulation::vehicleTypeCanEnterStreetType(const vehicleClass &vehicleClass, const streetType &streetType) {
    // if type compatibility between vehicles and streets plays a role, allow proper
    // examination of types
    if (Simulation::getTypeCompatibilityState()) {
        if (streetType == A or streetType == B) {
            return true;
        } else if (streetType == T and (vehicleClass == transport)) {
            return true;
        }
        return false;
    // vehicle and street compatibility play no role
    } else {
        return true;
    }
}


void Simulation::setTypeCompatibilityState(bool state) {
    typeCompatibility = state;
}
const bool Simulation::getTypeCompatibilityState() {
    return typeCompatibility;
}


const int Simulation::getStreetLength() {
    return streetLength;
}
void Simulation::setStreetLength(int length) {
    streetLength = length;
}

const int Simulation::getVehicleSpawnRate() {
    return vehicleSpawnRate;
}
void Simulation::setVehicleSpawnRate(int spawnRate) {
    vehicleSpawnRate = spawnRate;
}



void Simulation::printSimVariables() {
    std::cout << "\n------------ Sim Variables ------------" << std::endl
              << "minCarDistance:        " << Simulation::getMinCarDistance() <<  std::endl
              << "trafficLightMaxCount:  " << Simulation::getTrafficLightMaxCount() << std::endl
              << "effectiveSTOPdistance: " << Simulation::getEffectiveSTOPdistance() << std::endl
              << "decisionBufferLength:  " << Simulation::getDecisionBufferLength() << std::endl
              << "streetLength:          " << Simulation::getStreetLength() << std::endl
              << "type compatibility:    " << Util::boolToEnabledString(Simulation::getTypeCompatibilityState()) << std::endl
              << "vehicleSpawnRate:      " << Simulation::getVehicleSpawnRate() << std::endl
              << "---------------------------------------\n";
}



int Simulation::getTotalSpawnedVehicles() const {
    return _totalSpawnedVehicles;
}
void Simulation::incrementTotalSpawnedVehicles() {
    _totalSpawnedVehicles++;
}

int Simulation::getSpawnTimer() const {
    return _spawnTimer;
}
void Simulation::setSpawnTimer(int spawnTimer) {
    if (spawnTimer >= 0) {
        _spawnTimer = spawnTimer;
    }
}
void Simulation::incrementSpawnTimer() {
    ++_spawnTimer;
}

std::string Simulation::getNewLicensePlate() {
    std::stringstream ss;
    ss << getTotalSpawnedVehicles();
    ss << rand() % 9 + 1;
    ss << rand() % (getTotalSpawnedVehicles() + 1);
    incrementTotalSpawnedVehicles();
    return ss.str();
}

Vehicle *Simulation::createVehicleObj(const vehicleClass& vClass) {
    std::string licensePlate = getNewLicensePlate();
    if (vClass == personal) {
        return new Vehicle(personal, licensePlate);
    } else if (vClass == transport) {
        return new TransportVehicle(licensePlate);
    } else if (vClass == special) {
        return new SpecialVehicle(licensePlate);
    } else {
        return nullptr;
    }
}





