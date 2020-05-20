//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_STREET_H
#define TRAFFICSIMULATOR_STREET_H

#include "TransportVehicle.h"
#include "SpecialVehicle.h"


class Intersection;

// A : every type of normal vehicle and special vehicle will be able to take this route
// B : a route with a specialised bus lane, meaning buses will be more likely to take these routes if they make a detour
// T :Tram line on this street, this is the only street trams are allowed to use
enum streetType {A=0, B=1, T=2};


class Street {
    // variables used by Intersections
    Intersection* _nextIntersection;
    Intersection* _prevIntersection;
    bool _isTwoWay;

    streetType _type;

    // can contain speed limits or accidents or even a police car siren
    std::vector<const Influence*> _influences;
    bool _hasSpeedLimit;
    bool _hasStopSignal;

    // the list of all vehicles currently INSIDE the street
    std::vector<std::vector<Vehicle*>> _lanes;

    // the list of all vehicles deciding whether or not to enter that particular street
    // TODO the current Street of the vehicle should be empty when they drive through an intersection/State ???
    std::vector<std::vector<Vehicle*>> _entrants;

    // used to properly let Vehicles queue in streets without overlapping with each other
    // a two way street will have two lanes. That means there are also Z front vehicles and two back vehicles
    Vehicle* _frontOccupant[2];
    Vehicle* _backOccupant[2];

public:

    Street(Intersection* prev, Intersection* next, streetType type);

    bool fillEmptyLanes();
    bool fillEmptyEntrantLanes();

    void requestInfluences(Vehicle* requestingVehicle) const;
    void requestEntrantInfluences(Vehicle* requestingVehicle) const;

    Intersection* getOtherIntersection(const Intersection* intersection) const;


    // utils type functions

    std::string getTwoWayString() const;




    // getters and setters

    Intersection *getNextIntersection() const;
    void setNextIntersection(Intersection *nextIntersection);

    Intersection *getPrevIntersection() const;
    void setPrevIntersection(Intersection *prevIntersection);

    streetType getType() const;
    std::string typeToName() const;
    static streetType nameToType(const char& name);

    const std::vector<std::vector<Vehicle *>> &getLanes() const;
    bool removeFromLane(Vehicle* toRemoveVehicle, int lane);

    const std::vector<std::vector<Vehicle *>> &getEntrants() const;
    void addEntrant(int indexWhenLeaving, Vehicle* entrant);
    bool removeEntrant(int indexWhenLeaving, const Vehicle* entrant);


    Vehicle *getFrontOccupant(int index) const;
    void setFrontOccupant(Vehicle *frontOccupant, int lane);
    void setFrontNull(int lane);

    Vehicle *getBackOccupant(int index) const;
    void setBackOccupant(Vehicle *backOccupant, int lane);
    void setBackNull(int lane);

    bool isTwoWay() const;
    void setIsTwoWay(bool isTwoWay);

    const std::vector<const Influence *> &getInfluences() const;
    bool addInfluence(const Influence* newInfluence);
    bool removeInfluence(influenceType influenceType);

    bool hasSpeedLimit() const;
    void setHasSpeedLimit(bool hasSpeedLimit);

    bool hasStopSignal() const;
    void setHasStopSignal(bool hasStopSignal);
};


#endif //TRAFFICSIMULATOR_STREET_H
