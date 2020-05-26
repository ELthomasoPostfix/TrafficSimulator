//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_SIMULATION_H
#define TRAFFICSIMULATOR_SIMULATION_H

#include "Intersection.h"

// the amount of distance subsequent vehicles MUST keep between each other
extern int minCarDistance;
// the amount of time it takes for traffic lights to instantly cycle to a new pair of streets
extern int trafficLightMaxCount;

extern int decisionBufferLength;
// the distance from a STOP signal at which the STOP signal will take effect on the vehicle
extern int effectiveSTOPdistance;

extern int streetLength;
// whether or not vehicles being a certain type affects them entering a certain street type
extern bool typeCompatibility;

extern int vehicleSpawnRate;


class Simulation {

    // only keep a single instance of each type of influence, each use refers to the same object
    std::vector<Influence*> allInfluences;

    int _totalTimeSimulated;

    // TODO add some statistics functions here

    int _spawnTimer = 0;

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
    static void setMinCarDistance(int distance);

    static const int getTrafficLightMaxCount();
    static void setTrafficLightMaxCount(int count);

    static const int getEffectiveSTOPdistance();
    static void setEffectiveSTOPdistance(int distance);

    static const int getDecisionBufferLength();
    static void setDecisionBufferLength(int length);

    static const bool vehicleTypeCanEnterStreetType(const vehicleClass& vehicleClass, const streetType& streetType);

    static void setTypeCompatibilityState(bool state);
    static const bool getTypeCompatibilityState();

    static const int getStreetLength();
    static void setStreetLength(int length);

    static const int getVehicleSpawnRate();
    static void setVehicleSpawnRate(int spawnRate);

    static void printSimVariables();

    int getTotalSpawnedVehicles() const;
    void incrementTotalSpawnedVehicles();

    int getSpawnTimer() const;
    void setSpawnTimer(int spawnTimer);
    void incrementSpawnTimer();

    std::string getNewLicensePlate();
    Vehicle* createVehicleObj(const vehicleClass& vehicleClass);
};


#endif //TRAFFICSIMULATOR_SIMULATION_H
