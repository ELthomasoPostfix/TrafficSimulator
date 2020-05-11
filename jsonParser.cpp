//
// Created by elthomaso on 11.05.20.
//

#include "jsonParser.h"


Network* jsonParser::processJSON(const std::string& fileName) {
    std::fstream inputFile(fileName);
    inputFile.open(fileName);
    nlohmann::json json = nullptr;

    // deserialize the json file
    inputFile >> json;

    // initialise the network obj
    Network* cityobj = new Network();

    for (std::string symbolStr: json["alphabet"]) {
        cityobj->addAllowedStreetType(Street::nameToType(symbolStr[0]));
    }

    // add states to the network
    jsonToStates(json, cityobj);

    // add all transitions to the states of the network
    jsonToTransitions(json, cityobj);

    return cityobj;
}

void jsonParser::jsonToStates(nlohmann::json& json, Network* cityNetwork) {

    for (auto& intersection : json["intersections"]) {

        Intersection* newIntersection = new Intersection(intersection["name"]);

        // add traffic lights if necessary
        if (intersection["trafficlights"]) {
            newIntersection->setTrafficLights(cityNetwork->getSimulation()->getInfluence(STOP));
        }
    }
}

void jsonParser::jsonToTransitions(nlohmann::json& json, Network* cityNetwork) {
}/*
    std::string FAtype = json["type"];
    for (auto& transition: json["transitions"]) {
        // retrieve member values from the json object
        std::string from = transition["from"];
        std::string to = transition["to"];
        std::string input_str = transition["input"];
        char input_symbol = input_str[0];

        // find the state the transition leaves from and the target state of the transition in the list of all states
        State* originState = findState(Q, from);
        State* targetState = findState(Q, to);

        std::string epsilon;
        if (FAtype == "ENFA") {
            epsilon = json["eps"];
        }
        // add the transition to the origin state
        // only if the transitions symbol is in the alphabet or an epsilon
        if (originState != nullptr and targetState != nullptr) {
            if (((FAtype == "DFA" or FAtype == "NFA" or FAtype == "ENFA") and charInAlphabet(input_symbol, alphabet)) or
                (FAtype == "ENFA" and input_symbol == epsilon[0])){
                originState->add_transition(targetState, input_symbol,FAtype);
            } else {
                std::string errStr = "The input symbol '";
                errStr.push_back(input_symbol);
                errStr += "' was not found in the alphabet {";
                for (const char& c: alphabet) {
                    errStr.push_back(c);
                    if (c != alphabet[alphabet.size()-1]) {
                        errStr += ", ";
                    }
                }
                errStr += "} , so the transition from ";
                errStr += from;
                errStr += " to ";
                errStr += to;
                errStr += " will not be added to state ";
                errStr += from;
                std::cerr << errStr << std::endl;
            }
        } else {
            std::string missingState = from + "(from)";
            if (originState != nullptr) missingState = to + "(to)";
            else if (targetState == nullptr) missingState += "/" + to + "(to)";
            std::cerr << "The specified state(s) " << missingState << " was/were not found within the FA!\nThe transition from " << from
                      << " to " << to << " for the input symbol " << input_symbol << " will not be added." << std::endl;
        }
    }
}
*/
bool jsonParser::charInAlphabet(const char& c, const std::vector<char>& alphabet) {
    for (const char & ac: alphabet) {
        if (ac == c) return true;
    }
    return false;
}
