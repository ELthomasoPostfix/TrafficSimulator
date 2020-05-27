//
// Created by jeff on 5/18/20.
//

#ifndef NEW_T_A_SSC_H
#define NEW_T_A_SSC_H
#include "Network.h"

class SSC {
public:
    static Network* set_up_ssc(Network* d);
    static Network* make_trans_Intersection (Intersection* s,Network* d);
    static Network* set_up_start(Network* d);
    static Intersection* set_trans(Intersection*s, Network* d);
    static void fix(Network * d);
};


#endif //NEW_T_A_SSC_H
