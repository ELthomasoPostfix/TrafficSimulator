//
// Created by jeff on 5/18/20.
//

#include <algorithm>
#include "SSC.h"

Network* SSC::set_up_ssc(Network *d) {
    Network* ssc_network = set_up_start(d);
    Intersection* start = ssc_network->getNetwork()[0];
    while(true){
        for(Street * street : start->getAllLeavingStreets()){
            ssc_network = make_trans_Intersection(start->getOtherIntersection(street),ssc_network);
        }
        std::vector<Intersection*> test = ssc_network->getNetwork();
        start = set_trans(ssc_network->getNetwork().back(),ssc_network);
        if(test.size() == ssc_network->getNetwork().size()){
            break;
        }
        if(ssc_network->getNetwork().back()->getName().size() == 6){
            break;
        }
    }
    auto* street_temp = new Street(ssc_network->getNetwork().back(),ssc_network->getNetwork().back(),ssc_network->getAllowedStreetTypes()[0]);
    ssc_network->getNetwork().back()->addStreet(street_temp);
    int a = 5;
//    ssc_fa->ssc_set_dead_state();
return ssc_network;}


Intersection *SSC::set_trans(Intersection *i, Network* n) {
    for(streetType sT : n->getAllowedStreetTypes()) {
        std::string name;
        std::vector<Intersection*> prev_intersections;
        for (Intersection *intersection : i->getPrevIntersections()) {
            for(Street* street : intersection->getAllLeavingStreets()){
                if(street->getType() == sT){
                    if(!std::count(prev_intersections.begin(),prev_intersections.end(),street->getOtherIntersection(intersection))){
                        name += street->getOtherIntersection(intersection)->getName();
                        prev_intersections.push_back(street->getOtherIntersection(intersection));
                    }
                }
            }
        }
        bool tester = false;
        Intersection* new_intersection;
        for(Intersection* test : n->getNetwork()){
            if(test->getName() == name){
                tester = true;
                new_intersection = test;
            }
            bool tested = false;
            for (Intersection *loop2 : prev_intersections) {
                for(Intersection* thomas : test->getPrevIntersections()){
                    if(loop2 == thomas){
                        tested = true;
                    }
                }
                if (test->getPrevIntersections().size() != prev_intersections.size()) {
                    tested = true;
                }
            }
            if (!tested) {
                new_intersection = test;
                tester = true;
                break;
            }
        }
        if(!tester) {
            new_intersection = new Intersection(name);
            n->addStreetlessIntersection(new_intersection);
            new_intersection->setPrevIntersections(prev_intersections);
        }
        if(i->getStreets().size() < n->getAllowedStreetTypes().size()){
            auto* street_temp = new Street(i,new_intersection,sT);
            i->addStreet(street_temp);
        }
    }
return i;}



Network* SSC::make_trans_Intersection(Intersection * i,Network * n) {
    for (streetType sT : n->getAllowedStreetTypes()){
        std::string name;
        std::vector<Intersection*> previous_intersections;
        for(Intersection* temp : i->getPrevIntersections()){
            for(Street* street : temp->getAllLeavingStreets()){
                if(sT == street->getType()){
                    if(!std::count(previous_intersections.begin(),previous_intersections.end(),street->getOtherIntersection(temp))){
                        name += temp->getOtherIntersection(street)->getName();
                        previous_intersections.push_back(street->getOtherIntersection(temp));
                    }
                }
            }
        }
        Intersection* intersection;
        bool tester = false;
        if(!name.empty()){
            for(Intersection* test : n->getNetwork()){
                if(test->getName() == name){
                    tester = true;
                    intersection = test;
                }
                bool tested = false;
                for (Intersection *loop2 : previous_intersections) {
                    for(Intersection* thomas : test->getPrevIntersections()){
                        if(loop2 == thomas){
                            tested = true;
                        }
                    }
                    if (test->getPrevIntersections().size() != previous_intersections.size()) {
                        tested = true;
                    }
                }
                if (!tested) {
                    intersection = test;
                    tester = true;
                    break;
                }

            }
            if(!tester){
                intersection = new Intersection(name);
                intersection->setPrevIntersections(previous_intersections);
                n->addStreetlessIntersection(intersection);
            }
            if(i->getStreets().size() < n->getAllowedStreetTypes().size()){
                auto* street_temp = new Street(i,intersection,sT);
                i->addStreet(street_temp);
            }
        }
    }
return n;}

Network* SSC::set_up_start(Network *network) {
    Intersection * current_intersection = network->getNetwork()[0];
    auto * ssc_network = new Network;
    auto * new_Inter = new Intersection(current_intersection->getName());
//    ssc_network->addStreetlessIntersection(new_Inter);
    std::vector<Street*> temp = current_intersection->getAllLeavingStreets();
    std::vector<streetType> streetTypes;
    for(Street * street : current_intersection->getStreets()){
        if(!std::count(streetTypes.begin(),streetTypes.end(),street->getType())){
            streetTypes.push_back(street->getType());
        }
    }
    for(streetType street_type : streetTypes){
        ssc_network->addAllowedStreetType(street_type);
        std::string name;
        std::vector<Intersection*> previous_intersections;
        for (Street* street : temp) {
            if (street->getType() == street_type){
                name += current_intersection->getOtherIntersection(street)->getName();
                previous_intersections.push_back(street->getOtherIntersection(current_intersection));
            }
        }
        if (!name.empty()) {
            auto * new_Int = new Intersection(name);
            new_Int->setPrevIntersections(previous_intersections);

            auto* street_temp = new Street(current_intersection,new_Int,street_type);
            new_Inter->addStreet(street_temp);

            ssc_network->addStreetlessIntersection(new_Inter);
            ssc_network->addStreetlessIntersection(new_Int);
        }
    }

return ssc_network;}





