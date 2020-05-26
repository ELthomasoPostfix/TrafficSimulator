//
// Created by Thomas Gueutal on 21.05.20.
//


#ifndef TRAFFICSIMULATOR_STATEELIMINATION_H
#define TRAFFICSIMULATOR_STATEELIMINATION_H

#include "Network.h"
#include "ElimStreet.h"

class StateElimination {

public:
    void eliminate(Network *cityNetwork, Intersection* startIntersection, Intersection* endIntersection);

private:
    void separateStreetList(std::vector<Street*>& leavingStreets, std::vector<Street*>& enteringStreets,
                            std::vector<const Street*>& loopStreets, const std::vector<Street*>& allStreets,
                            const Intersection* currIntersection);

    void isolateUnnecessaryIntersections(std::vector<Street*>& leavingStreets, std::vector<Street*>& enteringStreets,
                       Intersection* intersection, Network* cityNetwork);

    void doElimination(Intersection* intersection, std::vector<Street*>& enteringStreets, std::vector<Street*>& leavingStreets,
                       std::vector<const Street*>& loopStreets);

    void disconnect(Intersection* intersection, Intersection* otherIntersection, Street* toRemoveStreet);

    void connect(Street* leavingStreet, Street* enteringStreet, Intersection* prevIntersection, Intersection* nextIntersection,
                 Intersection* intersection, std::vector<const Street*>& loopStreets);

    void connectZeroElimState(Street* leavingStreet, Street* enteringStreet, Intersection* intersection,
                                Intersection* otherIntersection, Intersection* nextIntersection, std::vector<const Street*>& loopStreets);

    void connectOneElimState(Street *elimStreet, Street *toConnectStreet, Intersection* intersection,
                             Intersection* otherIntersection, bool setPrev, std::vector<const Street*>& loopStreets);

    void connectTwoElimState(Street* leavingStreet, Street* enteringStreet, Intersection* intersection,
                             std::vector<const Street*>& loopStreets);


    // bool functions

    bool isLoop(Street* street) const;
};


#endif //TRAFFICSIMULATOR_STATEELIMINATION_H
