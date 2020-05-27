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

    Intersection* _ownPointer;

    const Influence* _trafficLights;
    std::vector<std::pair<Street*, Street*>> _trafficLightPairs;
    std::pair<Street*, Street*> _currentPair;
    int _trafficLightCounter;

    std::vector<Street*> _streets;

    bool _multipurposeMarker;

    // Useful for the SSC, a list containing all the previous intersection of this current one.
    std::vector<Intersection*> _previous_intersections;
public:
    Intersection(std::string  name);

    ~Intersection();

    // TODO could add a return to measure how often (time units) the street contains cars
    void emitInfluences(std::ofstream& trafficLightStream);

    void emitTrafficLightSignal(std::ofstream& trafficLightStream);

    void requestSignal(Vehicle* requestingVehicle) const;
    void requestEntrantSignal(Vehicle* requestingVehicle) const;


    void calculateTrafficScore();

    void onWrite(std::ofstream& outputFile, const std::string& indent);
    Intersection* getCommonIntersection(const std::pair<Street*, Street*>& TLpair);

    // helper functions to calculate individual scores

    int hasTLtoScore(bool hasTL) const;
    int vehicleAmountToScore(int amount) const;
    void addInfluenceScores(int* score, const std::vector<const Influence*>& influences) const;


    // emit a stop signal at the front occupants of the currentTrafficLightPair  ( get using getCurrentPair() )
    void stopCurrentFrontOccupants() const;
    void stopSingleFrontOccupant(int enteredLaneIndex, Vehicle* frontOccupant) const;
    void unStopCurrentFrontOccupants() const;
    void unstopSingleFrontOccupant(int enteringLaneIndex, Vehicle* frontOccupant) const;

    // if an intersection has only one traffic light pair, make it alternate between being off and on
    void invertFrontSTOPStates();
    void turnTLgreen();
    void turnTLred(const std::pair<Street*,Street*>& currentPair);
    // must be used on the intersection that you are leaving with the street you want to leave through
    int laneIndexWhenLeaving(const Street* street) const;
    // must be used on the intersection that you are entering with the street you are entering through
    int laneIndexWhenEntering(const Street* street) const;


    // some message functions

    void addCycleMessageFront(std::ofstream& trafficLightStream) const;
    void addCycleMessageBack(std::ofstream& trafficLightStream) const;
    void addTLCincrementMessage(std::ofstream& trafficLightStream) const;
    void addSingleTLCincrementMessage(std::ofstream& trafficLightStream) const;
    void addSingleTLCycleMessageFront(std::ofstream &trafficLightStream) const;
    void addSingleTLCycleMessageBack(std::ofstream &trafficLightStream) const;

    // getters and setters

    Intersection* getOtherIntersection(const Street* street) const;
    Vehicle* getIncomingFrontVehicle(const Street* street) const;

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
    Street* findStreet(const Intersection* intersection1, const Intersection* intersection2, const streetType& streetType,
                       bool twoWay) const;
    bool removeStreet(Street* toRemove);
    bool removeStreet(const Street* toRemove);
    void clearStreets();

    void setTrafficLights(const Influence* trafficLights);
    const Influence* getTrafficLightInfluence() const;
    bool getHasTrafficLights() const;
    void removeTrafficLights();

    Intersection *getOwnPointer() const;

    int getTrafficLightCounter() const;
    void setTrafficLightCounter(int trafficLightCounter);

    const std::vector<std::pair<Street *, Street *>> &getTrafficLightPairs() const;
    void addTrafficLightPair(std::pair<Street*, Street*> newPair);
    std::vector<Street*> getUnpairedStreets() const;
    bool streetInTrafficLightPairs(const Street* street) const;
    bool haveIntersection(const std::pair<Street*, Street*>& pair1, const std::pair<Street*, Street*>& pair2);

    const std::pair<Street *, Street *> &getCurrentPair() const;
    void setCurrentPair(const std::pair<Street *, Street *> &currentPair);
    void cycleTrafficLightsPair();

    bool isMultipurposeMarker() const;
    void setMultipurposeMarker(bool multipurposeMark);
    void removeAllMArkings();

    void setPrevIntersections (const std::vector<Intersection*>&);
    std::vector<Intersection*> getPrevIntersections ();
};


#endif //TRAFFICSIMULATOR_INTERSECTION_H
