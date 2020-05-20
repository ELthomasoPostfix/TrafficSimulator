//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_SIMULATION_H
#define TRAFFICSIMULATOR_SIMULATION_H

#include "Intersection.h"

class Simulation {

    // only keep a single instance of each type of influence, each use refers to the same object
    std::vector<Influence*> allInfluences;

    int _totalTimeSimulated;

    // TODO add some statistics functions here
    // the amount of distance subsequent vehicles MUST keep between each other
    static const int _minCarDistance = 1;
    // the amount of time it takes for traffic lights to instantly cycle to a new pair of streets
    static const int _trafficLightMaxCount = 5;

    static const int _decisionBufferLength = 10;
    // the distance from a STOP signal at which the STOP signal will take effect on the vehicle
    static const int _effectiveSTOPdistance = _minCarDistance;

    static const int _streetLength = 100;


    int _totalSpawnedVehicles = 0;

public:
    // #### constructor functions ####
    Simulation();

    void instantiateInfluences();
    // ###############################

    const Influence* getInfluence(influenceType IType) const;


    // getters and setters
    const std::vector<Influence*>& getAllInfluences() const;
    bool addInfluence(Influence* newInfluence);

    int getTotalTimeSimulated() const;
    void setTotalTimeSimulated(int totalTimeSimulated);
    void addTotalTimeSimulated(int additionalTime);

    static const int getMinCarDistance();

    static const int getTrafficLightMaxCount();

    static const int getEffectiveSTOPdistance();

    static const int getDecisionBufferLength();

    static const bool vehicleTypeCanEnterStreetType(const vehicleClass& vehicleClass, const streetType& streetType);

    static const int getStreetLength();

    int getTotalSpawnedVehicles() const;
    void incrementTotalSpawnedVehicles();
};


#endif //TRAFFICSIMULATOR_SIMULATION_H
