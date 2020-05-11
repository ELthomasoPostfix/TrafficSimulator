//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Network.h"

Network::Network() {
    _simulation = new Simulation();
}


void Network::toDot(std::ofstream &ofstream) const {

    if (ofstream.is_open()) {
        ofstream << "digraph {\nrankdir=LR\n";

        std::vector<Intersection *> allIntersections = getNetwork();

        for (Intersection *intersection : allIntersections) {
            ofstream << "\"" << intersection->getName() << "\"" << "\n";
            for (Street *street : intersection->getStreets()) {
                if (street->getPrevIntersection() == intersection) {
                    std::string both;
                    if (street->isTwoWay()) both = " dir=\"both\"";
                    ofstream << "\"" << intersection->getName() << "\"" << "->" << "\""
                             << street->getNextIntersection()->getName() << "\""
                             << " [label=\"" << street->typeToName() << "\"" << both << "]" << "\n";
                }
            }
        }
        ofstream << "}";
    } else {
        std::cerr << "You cannot write to a closed ofstream." << std::endl;
    }
}
void Network::toPNG(const std::string &fileName) const {
    std::string commandStr = "dot -Tpng -O " + fileName;
    const char *command = commandStr.c_str();
    system(command);
}



// getters and setters

const std::vector<Intersection *> &Network::getNetwork() const {
    return _network;
}
bool Network::addIntersection(Intersection* newIntersection) {
    if (newIntersection != nullptr and !newIntersection->getStreets().empty()) {
        for (Street* street : newIntersection->getStreets()) {
            if (street->getNextIntersection() == nullptr or street->getPrevIntersection() == nullptr) return false;
        }
        _network.emplace_back(newIntersection);
        return true;
    } else {
        return false;
    }
}

Simulation *Network::getSimulation() const {
    return _simulation;
}

const std::vector<streetType> &Network::getAllowedStreetTypes() const {
    return _allowedStreetTypes;
}
bool Network::typeIsAllowed(const streetType type) const {
    for (const streetType allowedType: getAllowedStreetTypes()) {
        if (allowedType == type) return true;
    }
    return false;
}
void Network::addAllowedStreetType(const streetType newAllowedStreetType) {
    if (!typeIsAllowed(newAllowedStreetType)) {
        _allowedStreetTypes.emplace_back(newAllowedStreetType);
    }
}
