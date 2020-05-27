//
// Created by Thomas Gueutal on 10.05.20.
//

#ifndef TRAFFICSIMULATOR_SPECIALVEHICLE_H
#define TRAFFICSIMULATOR_SPECIALVEHICLE_H

#include "Vehicle.h"

// necessary : retrieve a Influence*
class Simulation;

class SpecialVehicle : public Vehicle {

    // all influences caused by vehicles
    // if a normal vehicle has a traffic accident, then even they won't be able to emit Influences
    std::vector<const Influence*> _outgoingInfluences;

    bool _sirenOn;
    bool _sirenSent;

public:
    SpecialVehicle(const std::string& licensePlate);

    ~SpecialVehicle() override;

    void InstantiateInfluence(double argument);

    void emitInfluence() const override;
    void undoSiren() const override;

    void addOutgoingInfluence(const Influence* outgoingInfluence) override;

    const Influence* getSiren() const;

    bool getSirenOn() const override;
    void setSirenOn(bool sirenState) override;
    bool getSirenSent() const override;
    void setSirenSent(bool sirenSentState) override;
};


#endif //TRAFFICSIMULATOR_SPECIALVEHICLE_H
