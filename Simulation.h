//
// Created by elthomaso on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_SIMULATION_H
#define TRAFFICSIMULATOR_SIMULATION_H

#include "Intersection.h"

class Simulation {

    // only keep a single instance of each type of influence, each use refers to the same object
    std::vector<Influence*> allInfluences;

    // TODO add some statistics functions here


public:
    // #### constructor functions ####
    Simulation();

    void instantiateInfluences();
    // ###############################

    const Influence* getInfluence(influenceType IType) const;


    // getters and setters
    const std::vector<Influence*>& getAllInfluences() const;
    bool addInfluence(Influence* newInfluence);
};


#endif //TRAFFICSIMULATOR_SIMULATION_H
