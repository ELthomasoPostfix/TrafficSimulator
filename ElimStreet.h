//
// Created by Thomas Gueutal on 22.05.20.
//

#ifndef TRAFFICSIMULATOR_ELIMSTREET_H
#define TRAFFICSIMULATOR_ELIMSTREET_H

#include "Street.h"
#include "Intersection.h"


class ElimStreet: public Street {

    bool isElimStreet = true;

    std::vector<const Street*> _streets;
    std::vector<const Intersection*> _intersections;

    std::vector<const Street*> _loops;

public:
    ElimStreet(Intersection *prev, Intersection *next, streetType type);

    ~ElimStreet() override;

    bool isEStreet() override;

    void copyPath(Street* toCopyElimState);

    std::string getEStreetName() const override;
    void addLoopString(std::string& streetName, int loopIndex) const;

    void print();


    // getters and setters

    const std::vector<const Street *>* getStreets() const override;
    void addStreet(const Street * streets) override;

    const std::vector<const Intersection *>* getIntersections() const override;
    void addIntersection(const Intersection* intersection) override;

    const std::vector<const Street *>* getLoops() const override;
    void setLoops(std::vector<const Street *> &loops);
    void addLoop(const Street* loop);
};


#endif //TRAFFICSIMULATOR_ELIMSTREET_H
