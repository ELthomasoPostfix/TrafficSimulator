//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_NETWORK_H
#define TRAFFICSIMULATOR_NETWORK_H

#include "Simulation.h"

class Network {
    // the actual network consisting of intersections and streets, etc...
    std::vector<Intersection*> _network;

    std::vector<streetType> _allowedStreetTypes;

    // the simulation class is meant to serve as a statistical analysis and data storage class
    Simulation* _simulation;

public:

    Network();

    void toDot(std::ofstream& fstream) const;
    void toPNG(const std::string& fileName) const;

    // TODO add algorithms here

    // TODO add road expansion functions here


    // getters and setters

    const std::vector<Intersection *> &getNetwork() const;
    bool addIntersection(Intersection* newIntersection);

    Simulation *getSimulation() const;

    const std::vector<streetType> &getAllowedStreetTypes() const;
    bool typeIsAllowed(streetType type) const;
    void addAllowedStreetType(streetType newAllowedStreetType);
};


#endif //TRAFFICSIMULATOR_NETWORK_H
