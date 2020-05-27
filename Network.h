//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_NETWORK_H
#define TRAFFICSIMULATOR_NETWORK_H

#include "Simulation.h"

class Network {
    // the actual network consisting of intersections and streets, etc...
    std::vector<Intersection*> _network;
    std::vector<Vehicle*> _vehicles;
    std::vector<Intersection*> _influencingIntersections;

    std::vector<streetType> _allowedStreetTypes;

public:

    // the simulation class is meant to serve as a statistical analysis and data storage class
    Simulation* _simulation;

public:

    Network();

    ~Network();

    void doMainLoop(int duration, std::string& ofName, std::string& ofName2, std::string& ofName3);

    void toDot(std::ofstream& outputFile) const;
    void toDotElim(std::ofstream& outputFile) const;
    void toPNG(const std::string& dotFileName) const;

    void onWrite(std::ofstream& networkOUTPUT);



    void findAndSetPath(Vehicle* recipient, Intersection* startIntersection, Intersection* endIntersection);
    std::vector<std::vector<const Street *>> findAllPaths(Intersection* startIntersection, Intersection* endIntersection);



    Network* getSubNetwork(Intersection* startIntersection, Intersection* endIntersection, float extraStepMoodifier);
    bool isIn(const Intersection* intersection, const std::vector<Intersection*>& intersections);

    // this function should only be used in the context of state elimination
    // it converts all streets of a state eliminated network,
    // which all start at the start state and end at the end state, and then returns a list of paths
    // in the form of a list of lists of streets. one list of streets is a single path
    std::vector<std::vector<const Street*>> elimStreetsToPaths(Intersection* startIntersection);

    void testPathCorrectnessManuel(const std::vector<std::vector<const Intersection *>> &intersectionPath,
                                   const std::vector<std::vector<const Street *>> &streetPath, Intersection* startIntersection);


    void removeAllMultipurposeMarkers() const;

    void tryRandomVehicleSpawn(std::ofstream& vehicleDriveStream);
    Vehicle* createVehicle();
    void selectPath(const std::vector<std::vector<const Street*>>& paths, const Intersection* startIntersection,
                    Vehicle* spawnedVehicle);

    Intersection* getRandomIntersection() const;

private:
    void writeAllVehicleChains(std::ofstream& vehicleChainStream) const;
    void findFrontVehicles(std::vector<const Vehicle*>& frontVehicles, std::vector<bool>& chainWasWritten) const;
    void writeChain(std::ofstream& vehicleChainStream, std::vector<const Vehicle*>& frontVehicles,
                    std::vector<bool>& chainWasWritten, const Street* currentStreet, unsigned int startIndex) const;
    void writeChainString(std::ofstream& vehicleChainStream, const Vehicle*& currChainVehicle) const;


    // vehicle spawning

    void assignSubStartAndEnd(Intersection *&subStart, Intersection *&subEnd, Network *subNetwork,
                              Intersection* startIntersection, Intersection* endIntersection) const;
public:

    // TODO add algorithms here

    // TODO add road expansion functions here


    // getters and setters

    const std::vector<Intersection *> &getNetwork() const;
    bool addIntersection(Intersection* newIntersection);
    bool addStreetlessIntersection(Intersection* newStreetlessIntersection);
    Intersection* findIntersection(const std::string& name) const;
    void removeIntersection(const Intersection* toRemoveIntersec);

    Simulation *getSimulation() const;

    const std::vector<streetType> &getAllowedStreetTypes() const;
    bool typeIsAllowed(streetType type) const;
    void addAllowedStreetType(streetType newAllowedStreetType);

    const std::vector<Vehicle *> &getVehicles() const;
    void addVehicle(Vehicle * vehicle);
    void removeVehicle(const Vehicle* toRemoveVehicle);

    const std::vector<Intersection *> &getInfluencingIntersections() const;
    void addInfluencingIntersection(Intersection * influencingIntersection);
};


#endif //TRAFFICSIMULATOR_NETWORK_H
