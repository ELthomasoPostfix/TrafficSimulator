//
// Created by jeff on 5/18/20.
//

#include "SSC.h"

FA* SSC::set_up_ssc(FA *d) {
    FA* ssc_fa = set_up_start(d);
    State* start = ssc_fa->all_states[0];
    while(true){
        for(std::pair<char,State*> p : start->getStateTransition()){
            ssc_fa = make_trans_state(p.second,ssc_fa);
        }
        std::vector<State*> test = ssc_fa->all_states;
        start = set_trans(ssc_fa->all_states.back(),ssc_fa);
        if(test.size() == ssc_fa->all_states.size()){
            break;
        }
    }
    ssc_fa->buildFA();
    ssc_fa->setNames();
    ssc_fa->set_end_states();
    ssc_fa->fix();
    ssc_fa->ssc_set_dead_state();
    return ssc_fa;}

State *SSC::set_trans(State *s, FA* d) {
    for(char c : d->getSigma()) {
        std::string name;
        std::vector<State*> prev_states;
        for (State *st : s->getSscPrevStates()) {
            for(std::pair<char,State*> p : st->getStateTransition()){
                if(p.first == c){
                    if(!std::count(prev_states.begin(),prev_states.end(),p.second)){
                        name += p.second->getNaam();
                        prev_states.push_back(p.second);
                    }
                }
            }
        }
        bool tester = false;
        State* new_state;
        for(State* test : d->all_states){
            if(test->getNaam() == name){
                tester = true;
                new_state = test;
            }
            bool tested = false;
            for (State *loop2 : prev_states) {
                if (!std::count(test->getSscPrevStates().begin(), test->getSscPrevStates().end(), loop2) or test->getSscPrevStates().size() != prev_states.size()) {
                    tested = true;
                }
            }
            if (!tested) {
                new_state = test;
                tester = true;
                break;
            }

        }
        if(!tester) {
            new_state = new State(false, false, name);
            d->addState(new_state);
            for(State* test : prev_states){
                new_state->setSscPrevStates(test);
            }
        }
        if(s->getStateTransition().size() < 2){
            s->setStateTransition(new_state, c);
        }
    }
return s;}


FA* SSC::make_trans_state(State *s, FA *d) {
    for (char c : d->getSigma()){
        std::string name;
        std::vector<State *> prev_states;
        for(State* temp : s->getSscPrevStates()){
            for(std::pair<char,State*> p : temp->getStateTransition()){
                if(c == p.first){
                    if(!std::count(prev_states.begin(),prev_states.end(),p.second)){
                        name += p.second->getNaam();
                        prev_states.push_back(p.second);
                    }
                }
            }
        }
        State* new_trans;
        bool tester = false;
        if(!name.empty()){
            std::vector<std::string> all_names;
            for(State* loop : d->all_states){
                all_names.push_back(loop->getNaam());
            }
            for(State* test : d->all_states){
                if(test->getNaam() == name){
                    tester = true;
                    new_trans = test;
                }
                bool tested = false;
                for (State *loop2 : prev_states) {
                    if (!std::count(test->getSscPrevStates().begin(), test->getSscPrevStates().end(), loop2) or test->getSscPrevStates().size() != prev_states.size()) {
                        tested = true;
                    }
                }
                if (!tested) {
                    new_trans = test;
                    tester = true;
                    break;
                }

            }
            if(!tester){
                new_trans = new State(false,false,name);
                for (State *sta : prev_states) {
                    new_trans->setSscPrevStates(sta);
                }
                d->addState(new_trans);
            }
            if(s->getStateTransition().size() < 2){
                s->setStateTransition(new_trans, c);
            }
        }
    }
return d;}

bool SSC::check(const std::vector<std::string>& v, const std::vector<State *>& s, const std::string& name, FA* d) {
    if(!std::count(v.begin(),v.end(),name)){
        return false;
    }
    for(State* state : d->all_states){
        bool tested = false;
        for(State* loop1 : state->getSscPrevStates()){
            for(State* loop2 : s){
                if(loop1->getNaam() != loop2->getNaam()){
                    tested = true;
                }
            }
        }
        if(!tested){
            return false;
        }
    }
return true;}

FA* SSC::set_up_start(FA *d) {
    State* current = d->getQ0();
    bool first_state = true;
    FA* ssc_fa = new FA;
    auto *new_state = new State(first_state, false, current->getNaam());
    ssc_fa->addState(new_state);
    std::vector<std::pair<char, State *>> temp = current->getStateTransition();
    for (char c : d->getSigma()) {
        ssc_fa->setSigma(c);
        std::string name;
        std::vector<State *> prev_states;
        for (std::pair<char, State *> p : temp) {
            if (c == p.first) {
                name += p.second->getNaam();
                prev_states.push_back(p.second);
            }
        }
        if (!name.empty()) {
            auto *new_trans = new State(false, false, name);
            for (State *s : prev_states) {
                new_trans->setSscPrevStates(s);
            }
            ssc_fa->addState(new_trans);
            new_state->setStateTransition(new_trans, c);
        }
    }
return ssc_fa;}



