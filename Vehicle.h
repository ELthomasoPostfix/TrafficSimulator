//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_VEHICLE_H
#define TRAFFICSIMULATOR_VEHICLE_H

#include "vector"

#include "Influence.h"

class Intersection;

class Street;

// (1) personal vehicles encompass normal family cars, sports cars and all other relatively small, privately owned vehicles.
// these vehicles normally have NO DIRECT IMPACT on the traffic in the form of active influences/active special situations
// except for maybe car accidents
// (2) transport vehicles will have more restrictions placed on them by the simulations, such as more defined or
// set paths they need to follow, etc.
// (3) the special class vehicles will be able to initiate special situations on a more regular basis than normal or transport
// vehicles. Special vehicles include: ambulances, police cars, fire trucks, etc ...

enum vehicleClass {personal=0, transport=1, special=2};

class Vehicle {
    // way to identify vehicle
    vehicleClass _class;
    const std::string _licensePlate;

    // all influence related members
    std::vector<const Influence*> _incomingInfluences;
    bool _isLimited;
    bool _isStopped;

    double _speed;

    // members describing the current location
    Intersection* _startIntersection;
    Intersection* _endIntersection;
    Street* _currentStreet;
    bool _underway;
    double _progress;  // only when a vehicle is a set amount of distance (Simulation::getStreetLength - Simulation::getDecisionBuffer complete) through the street, can they decide on a new route

    std::pair<std::vector<Intersection*>, std::vector<Street*>> _path;

    Intersection* _prevIntersection;
    Intersection* _nextIntersection;
    Street* _nextStreet;
    std::vector<const Influence*> _entrantInfluences;

    // when a vehicle enters a street, they will essentially enter a queue to get out at the other side
    // (1) this queue will allow easy simulation of a traffic jam, as the progress across the street and whether or not
    // a vehicle is STOP-ped can directly be retrieved from the next pointer
    // (2) it allows will allow the implementation of a distance rule between cars
    // (3) it allows a new car to easily enter the queue when it enters the street, simulating real traffic
    // (4) if street lights stay green for e.g. 20 time units, and cars take 1 time units to enter a new street,
    // then the queue can realistically shrink by letting the front car enter a new street.
    // all cars can decide which route to take inside a street and before a traffic light
    Vehicle* _nextVehicle;
    Vehicle* _prevVehicle;
public:
    Vehicle(vehicleClass vClass, std::string licensePlate);

    virtual ~Vehicle();

    Street* chooseRandomStreet();

    void drive(std::ofstream& ofstream);
    void alterPath();
    void leaveEntrantsList() const;
    void addSTOPMessage(std::ofstream& ofstream, double effectiveSTOPLocation) const;
    void adjustProgress(std::ofstream& ofstream);
    void addProgressMessage(std::ofstream& ofstream, double progress) const;

    // influences will always be emitted to the immediate surroundings, but should only be used by a special vehicle
    virtual void emitInfluence() const;
    virtual void undoSiren() const;

    // special vehicle needs to be able to add an outgoing STOP influence
    virtual void addOutgoingInfluence(const Influence* outgoingInfluence);

    void receiveInfluence(const Influence* incomingInfluence);

    void enterStreet(std::ofstream& ofstream);

    void accident();

    void onWrite(std::ofstream& ofstream) const;


    // boolean functions

    bool mayDrive(double effectiveSTOPLocation, double argument) const;

    bool vehicleCanLeaveIntersection(const std::vector<Street*>& leavingStreets) const;

    bool isFront() const;


    // getters and setters

    vehicleClass getClass() const;
    void setClass(vehicleClass vClass);
    std::string classToName() const;
    static vehicleClass nameToClass(const std::string& name);
    static vehicleClass intToClass(int vClass);

    const std::vector<const Influence *> &getIncomingInfluences() const;
    const Influence* getIncomingInfluence(const influenceType& influenceType, double argument) const;
    double getArgument(influenceType influenceType) const;
    double getLowestRelevantArgument(influenceType influenceType) const;
    bool addIncomingInfluence(const Influence * incomingInfluence);
    void removeIncomingInfluence(const Influence* toRemoveInfluence);
    void clearIncomingInfluences();

    const std::vector<const Influence *> &getEntrantInfluences() const;
    bool addEntrantInfluence(const Influence* entrantInfluence);
    void clearEntrantInfluences();

    bool isLimited() const;
    void setIsLimited(bool isLimited);

    bool isStopped() const;
    void setIsStopped(bool isStopped);

    Intersection *getStartIntersection() const;
    void setStartIntersection(Intersection *startIntersection);

    Intersection *getEndIntersection() const;
    void setEndIntersection(Intersection *endIntersection);

    Street *getCurrentStreet() const;
    void setCurrentStreet(Street *currentStreet);

    bool isUnderway() const;
    void setUnderway(bool underway);

    double getProgress() const;
    void setProgress(double progress);
    void addProgress(double additionalProgress);

    Vehicle *getNextVehicle() const;
    void setNextVehicle(Vehicle *nextVehicle);

    Vehicle *getPrevVehicle() const;
    void setPrevVehicle(Vehicle *prevVehicle);

    Intersection *getPrevIntersection() const;
    void setPrevIntersection(Intersection *prevIntersection);

    Intersection *getNextIntersection() const;
    void setNextIntersection(Intersection *nextIntersection);

    Street *getNextStreet() const;
    void setNextStreet(Street *nextStreet);

    double getSpeed() const;
    void setSpeed(double speed);
    double getMaxDriveDistance() const;

    const std::string &getLicensePlate() const;
};


#endif //TRAFFICSIMULATOR_VEHICLE_H
