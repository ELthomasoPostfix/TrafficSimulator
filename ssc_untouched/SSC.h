//
// Created by jeff on 5/18/20.
//

#ifndef NEW_T_A_SSC_H
#define NEW_T_A_SSC_H
#include "FA.h"

class SSC {
public:
    static FA* set_up_ssc(FA* d);
    static FA* make_trans_state (State* s,FA* d);
    static FA* set_up_start(FA* d);
    static State* set_trans(State*s, FA* d);
    static bool check(const std::vector<std::string>& v , const std::vector<State*>& s,const std::string& name,FA* d );
};


#endif //NEW_T_A_SSC_H
