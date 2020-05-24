//
// Created by Thomas Gueutal on 21.05.20.
//

#include "StateElimination.h"

void StateElimination::eliminate(Network *cityNetwork, Intersection* startIntersection, Intersection* endIntersection) {

    // loop over every Intersection in the minimal Network
    for (Intersection* intersection : cityNetwork->getNetwork()) {
        //      Intersection* intersection = cityNetwork->getNetwork()[1];  // TODO    ### delete this
        // the start- and end intersections may not be eliminated
        if (intersection != startIntersection and intersection != endIntersection) {
            std::vector<Street *> leavingStreets;
            std::vector<Street *> enteringStreets;
            std::vector<const Street *> loopStreets;
            // identify the leaving and entering states
            // TODO:  are twoWay streets treated as two different streets? ==> both in leaving and entering
            separateStreetList(leavingStreets, enteringStreets, loopStreets, intersection->getStreets(), intersection);

            // there are both previous and next intersections, eliminate
            if (!leavingStreets.empty() and !enteringStreets.empty()) {
                doElimination(intersection, enteringStreets, leavingStreets, loopStreets);
            // there are no leaving streets, so there is no next intersection
            // OR  there are no entering streets, so there is no previous intersection
            // OR  both
            } else {
                isolateUnnecessaryIntersections(leavingStreets, enteringStreets, intersection);
            }
        }
    }
    // delete any final ElimStates that doe not go from startIntersection to endIntersection
}


void StateElimination::separateStreetList(std::vector<Street*>& leavingStreets, std::vector<Street*>& enteringStreets,
                                          std::vector<const Street*>& loopStreets, const std::vector<Street*>& allStreets,
                                          const Intersection* currIntersection) {
    if (currIntersection != nullptr) {
        // separate the list of streets into two lists
        //   - the list of all leaving streets
        //   - the list of all entering streets
        for (Street *street : allStreets) {
            if (isLoop(street)) {
                loopStreets.emplace_back(street);

            // not a looping street with the same previous and next intersections
            } else {
                bool isLeaving = currIntersection->isLeavingStreet(street);
                if (isLeaving) {
                    leavingStreets.emplace_back(street);
                }
                if (street->isTwoWay() or !isLeaving) {
                    enteringStreets.emplace_back(street);
                }
            }
        }
    }
}

void StateElimination::isolateUnnecessaryIntersections(std::vector<Street *> &leavingStreets, std::vector<Street *> &enteringStreets,
                                     Intersection* intersection) {
    // the state cannot reach any other states in this network, including the end state,
    // so remove references to it's streets in other intersections
    if (leavingStreets.empty()) {
        for (Street* enteringStreet : enteringStreets) {
            enteringStreet->getOtherIntersection(intersection)->removeStreet(enteringStreet);
        }
    } if (enteringStreets.empty()) {
        // the state cannot be reached in this network, so remove references to it's streets in other
        // intersections
        for (Street* leavingStreet : leavingStreets) {
            leavingStreet->getOtherIntersection(intersection)->removeStreet(leavingStreet);
        }
    }
}

void StateElimination::doElimination(Intersection *intersection, std::vector<Street *> &enteringStreets,
                                     std::vector<Street *> &leavingStreets, std::vector<const Street*>& loopStreets) {

    for (Street* enteringStreet : enteringStreets) {
        Intersection* prevIntersection = enteringStreet->getOtherIntersection(intersection);
        for (Street* leavingStreet : leavingStreets) {
            Intersection *nextIntersection = leavingStreet->getOtherIntersection(intersection);
            // if a leaving street is a loop from and to the to eliminate intersections s, then you don't make a new
            // ElimStreet using the entering street and the leaving street
            // the same applies for entering streets, as a loop on s is always both an entering and a leaving street
            // ==> you may not make new streets that end in the to eliminate intersection
            if (!isLoop(leavingStreet) and !isLoop(enteringStreet)) {
                // create new ElimStreets, add the new streets and intersection to it and connect it to the other
                // intersections (prev and or next)
                connect(leavingStreet, enteringStreet, prevIntersection, nextIntersection, intersection, loopStreets);
            }
            // the last entering street that needs to use the leaving streets detach each leaving street
            // from its previous and next intersections' streets list after using the leaving street
            if (enteringStreet == enteringStreets.back()) {
                disconnect(intersection, nextIntersection, leavingStreet);
            }
        }
        // a two way street would be in both the entering and leaving lists, so don't disconnect it here. The disconnecting
        // of a two way street will happen in the leavingStreet loop
        if (!enteringStreet->isTwoWay()) {
            // after a single entering street has been used on all leaving streets, detach the street from
            // its previous and next intersections' streets list
            disconnect(intersection, prevIntersection, enteringStreet);
        }
    }
    for (Street* leavingStreet : leavingStreets) {
        if (leavingStreet->isEStreet()) {
            intersection->removeStreet(leavingStreet);
            delete leavingStreet;
        }
    }
    // when eliminating states new ElimStreets are made each time.
    // delete all previously existing, smaller ElimStreets for the enteringStreet
    for (Street* enteringStreet : enteringStreets) {
        if (enteringStreet->isEStreet()) {
            intersection->removeStreet(enteringStreet);
            delete enteringStreet;
        }
    }
    // remove all loops from the intersections' streets list
    for (const Street* loop : loopStreets) {
        intersection->removeStreet(loop);
    }
}

void StateElimination::disconnect(Intersection *intersection, Intersection *otherIntersection, Street *toRemoveStreet) {

        // remove the leaving street from the intersection's next's streets
        otherIntersection->removeStreet(toRemoveStreet);
        intersection->removeStreet(toRemoveStreet);
}

void StateElimination::connect(Street *leavingStreet, Street *enteringStreet, Intersection *prevIntersection,
                               Intersection *nextIntersection, Intersection *intersection, std::vector<const Street*>& loopStreets) {

    // the previous street is already an eliminationStreet
    if (enteringStreet->isEStreet() and !leavingStreet->isEStreet()) {

        connectOneElimState(enteringStreet, leavingStreet, intersection, nextIntersection, false, loopStreets);

        // the next street is an eliminationStreet
    } else if (leavingStreet->isEStreet() and !enteringStreet->isEStreet()) {

        connectOneElimState(leavingStreet, enteringStreet, intersection, prevIntersection, true, loopStreets);

        // both the entering and leaving streets are an ElimStreet, so reuse one
    } else if (enteringStreet->isEStreet() and leavingStreet->isEStreet()) {

        connectTwoElimState(leavingStreet, enteringStreet, intersection, loopStreets);
        // neither the previous nor the next street is an eliminationStreet
    } else {

        connectZeroElimState(leavingStreet, enteringStreet, intersection, prevIntersection, nextIntersection, loopStreets);
    }
}


void StateElimination::connectZeroElimState(Street *leavingStreet, Street *enteringStreet, Intersection *intersection,
                                            Intersection *prevIntersection, Intersection* nextIntersection,
                                            std::vector<const Street*>& loopStreets) {

    ElimStreet* newElimStreet = new ElimStreet(prevIntersection, nextIntersection, A);
    // both streets need to be added, as well as the to eliminate intersection
    newElimStreet->addStreet(enteringStreet);
    newElimStreet->addIntersection(intersection);
    newElimStreet->addStreet(leavingStreet);

    // add possible loops to the newElimStreet
    newElimStreet->setLoops(loopStreets);

    // connect the newElimState to the prev and next intersections
    newElimStreet->setPrevIntersection(prevIntersection);
    newElimStreet->setNextIntersection(nextIntersection);
    prevIntersection->addStreet(newElimStreet);
    // prevent adding a single street twice to the same intersection.
    // This happens when e.g.    1 -> 3 -> 1   where 1 and 3 are intersections, 3 is the to eliminate intersection
    // ==> the streets form a loop, so next and prev are both intersection 1
    if (prevIntersection != nextIntersection) {
        nextIntersection->addStreet(newElimStreet);
    }
    std::cout << "s: " << intersection->getName() << "   ";
    newElimStreet->print();
}

void StateElimination::connectOneElimState(Street *elimStreet, Street *toConnectStreet, Intersection *intersection,
                                           Intersection *otherIntersection, bool setPrev, std::vector<const Street*>& loopStreets) {

    // even if the entering street is an ElimStreet, it can still generate more ElimStates
    // if intersection has multiple leaving states
    ElimStreet* newElimStreet = new ElimStreet(elimStreet->getPrevIntersection(),
                                               elimStreet->getNextIntersection(), A);


    // enforce addition in correct order
    // if next street is the ElimStreet, first add the previous street to newElimStreet, then copy the path of the EimStreet
    //  p  --non-ElimStreet-->  s  --ElimStreet-->  q
    if (setPrev) {
        // add the new components to the ElimStreet
        newElimStreet->addStreet(toConnectStreet);

        // loops attached to intersection (s)
        // add these loops to newElim's loop list
        newElimStreet->setLoops(loopStreets);

        newElimStreet->addIntersection(intersection);

        // copy the existing path
        newElimStreet->copyPath(elimStreet);
    // if prev street is the ElimStreet, first copy the path of the ElimStreet, then add the next street
    //  p  --ElimStreet-->  s  --non-ElimStreet-->  q
    } else {
        // copy the existing path
        newElimStreet->copyPath(elimStreet);
        // then do the loops before the intersection
        newElimStreet->addLoops(loopStreets);

        newElimStreet->addIntersection(intersection);
        // add the new components to the ElimStreet
        newElimStreet->addStreet(toConnectStreet);
    }
    // connect the ElimStreet to the next of the to eliminate intersection
    // == connect the p and q states
    if (setPrev) {
        newElimStreet->setPrevIntersection(otherIntersection);
    } else {
        newElimStreet->setNextIntersection(otherIntersection);
    }
    // connect the previous and next intersections to the newElimStreet

    Intersection* elimOther = elimStreet->getOtherIntersection(intersection);
    // toConnectOther is the intersection opposite of the ElimStreet's to eliminate intersection
    elimOther->addStreet(newElimStreet);
    // prevent adding a single street twice to the same intersection.
    // This happens when e.g.    1 -> 3 -> 1   where 1 and 3 are intersections, 3 is the to eliminate intersection
    // ==> the streets form a loop, so next and prev are both intersection 1
    if (elimOther != otherIntersection) {
        // otherIntersection is the intersection opposite of non-ElimStreet's to eliminate intersection
        otherIntersection->addStreet(newElimStreet);
    }

    std::cout << "s: " << intersection->getName() << "   ";
    newElimStreet->print();
}

void StateElimination::connectTwoElimState(Street *leavingStreet, Street *enteringStreet, Intersection *intersection,
                                           std::vector<const Street*>& loopStreets) {

    ElimStreet* newElimStreet = new ElimStreet(enteringStreet->getPrevIntersection(),
                                               enteringStreet->getNextIntersection(), A);

    // copy existing path
    newElimStreet->copyPath(enteringStreet);
    // add new components
    newElimStreet->addIntersection(intersection);
    // copy existing path
    newElimStreet->copyPath(leavingStreet);

    // add possible loops to the newElimStreet
    newElimStreet->setLoops(loopStreets);


    // connect the newElimState to the previous and next Intersections
    Intersection* prevIntersection = enteringStreet->getPrevIntersection();
    Intersection* nextIntersection = leavingStreet->getNextIntersection();
    newElimStreet->setPrevIntersection(prevIntersection);
    newElimStreet->setNextIntersection(nextIntersection);

    // connect the previous and next intersections to the newElimState
    prevIntersection->addStreet(newElimStreet);
    // prevent adding a single street twice to the same intersection.
    // This happens when e.g.    1 -> 3 -> 1   where 1 and 3 are intersections, 3 is the to eliminate intersection
    // ==> the streets form a loop, so next and prev are both intersection 1
    if (prevIntersection != nextIntersection) {
        nextIntersection->addStreet(newElimStreet);
    }
    std::cout << "s: " << intersection->getName() << "   ";
    newElimStreet->print();
}




// bool functions

bool StateElimination::isLoop(Street *street) const {
    return street->getPrevIntersection() == street->getNextIntersection();
}
