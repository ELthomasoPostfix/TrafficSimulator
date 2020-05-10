//
// Created by elthomaso on 10.05.20.
//

#include "Simulation.h"

Simulation::Simulation() {
    // we only keep a single LIMIT, STOP and REROUTE object in the entire simulation/network
    instantiateInfluences();
}

void Simulation::instantiateInfluences() {

    Influence* stop    = new Influence(STOP);
    Influence* limit   = new Influence(LIMIT);
    Influence* reroute = new Influence(REROUTE);
    allInfluences.emplace_back(stop);
    allInfluences.emplace_back(limit);
    allInfluences.emplace_back(reroute);
}

const Influence *Simulation::getInfluence(influenceType IType) const {
    for (Influence* influence : allInfluences) {

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
