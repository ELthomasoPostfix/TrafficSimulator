//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_INTERSECTION_H
#define TRAFFICSIMULATOR_INTERSECTION_H

#include "Street.h"

// The intersection functions as a cellular automaton state
class Intersection {

    std::string _name;
    int _trafficScore;

    const Influence* _trafficLights;
    std::vector<std::pair<Street*, Street*>> _trafficLightPairs;
    std::pair<Street*, Street*> _currentPair;
    int _trafficLightCounter;

    std::vector<Street*> _streets;
public:
    Intersection(std::string  name);

    // TODO could add a return to measure how often (time units) the street contains cars
    void emitInfluences();

    void emitTrafficLightSignal();

    // emit a stop signal at the front occupants of the currentTrafficLightPai  ( get using getCurrentPair() )r
    void stopFrontOccupants() const;

    void unStopFrontOccupants() const;

    Intersection* getOtherIntersection(const Street* street) const;
    Vehicle* getIncomingFrontVehicle(const Street* street) const;
    // must be used on the intersection that you are leaving with the street you want to leave through
    int laneIndexWhenLeaving(const Street* street) const;
    // must be used on the intersection that you are entering with the street you are entering through
    int laneIndexWhenEntering(const Street* street) const;

    bool isEnteringStreet(const Street* street) const;
    std::vector<Street*> getAllEnteringStreets() const;
    bool isLeavingStreet(const Street* street) const;
    std::vector<Street*> getAllLeavingStreets() const;






    // getters and setters

    const std::string& getName() const;


    int getTrafficScore() const;
    void setTrafficScore(int trafficScore);
    void adjustTrafficScore(int additional);    // additional may be either positive or negative


    const std::vector<Street *> &getStreets() const;
    bool addStreet(Street* newStreet);

    void setTrafficLights(const Influence* trafficLights);
    const Influence* getTrafficLightInfluence() const;
    bool getHasTrafficLights() const;

    int getTrafficLightCounter() const;
    void setTrafficLightCounter(int trafficLightCounter);

    const std::vector<std::pair<Street *, Street *>> &getTrafficLightPairs() const;
    void addTrafficLightPair(std::pair<Street*, Street*> newPair);
    bool haveIntersection(const std::pair<Street*, Street*>& pair1, const std::pair<Street*, Street*>& pair2);

    const std::pair<Street *, Street *> &getCurrentPair() const;
    void setCurrentPair(const std::pair<Street *, Street *> &currentPair);
    void cycleTrafficLightsPair();
};


#endif //TRAFFICSIMULATOR_INTERSECTION_H
