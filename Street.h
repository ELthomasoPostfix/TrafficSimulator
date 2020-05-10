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
enum streetType {A, B, T};

class Street {
    Intersection* _nextIntersection;
    Intersection* _prevIntersection;

    streetType _type;

    // the list of all vehicles currently INSIDE the street
    std::vector<Vehicle*> _occupants;

    // the list of all vehicles deciding whether or not to enter that particular street
    // TODO the current Street of the vehicle should be empty when they drive through an intersection/State ???
    std::vector<Vehicle*> _entrants;

    // used to properly let Vehicles queue in streets without overlapping with each other
    Vehicle* _frontOccupant;
    Vehicle* _backOccupant;

public:
    Street(Intersection* prev, Intersection* next, streetType type);


    // getters and setters

    Intersection *getNextIntersection() const;
    void setNextIntersection(Intersection *nextIntersection);

    Intersection *getPrevIntersection() const;
    void setPrevIntersection(Intersection *prevIntersection);

    streetType getType() const;
    std::string typeToName() const;

    const std::vector<Vehicle *> &getOccupants() const;

    const std::vector<Vehicle *> &getEntrants() const;


    Vehicle *getFrontOccupant() const;
    void setFrontOccupant(Vehicle *frontOccupant);

    Vehicle *getBackOccupant() const;
    void setBackOccupant(Vehicle *backOccupant);
};


#endif //TRAFFICSIMULATOR_STREET_H
