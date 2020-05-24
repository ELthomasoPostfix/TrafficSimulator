//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Influence.h"

Influence::Influence(influenceType type) : _type(type) {
    _argument = 0;
}



// getters and setters

influenceType Influence::getType() const {
    return _type;
}

int Influence::getArgument() const {
    return _argument;
}

void Influence::setArgument(int argument) {
    _argument = argument;
}

int Influence::toScore() const {
    const influenceType& type = getType();
    const double& arg = getArgument();
    // TODO can make more dynamic by adding a list that holds values to consider for a specific type
    //  so the user can add more custom values
    if (type == STOP) {
        // the STOP signal is of a special vehicle that stops an entire street
        if (arg == -2) {
            return 20;
        // traffic lights, ...
        } else {
            return 5;
        }
    // influence depends on speed limit
    } else if (type == LIMIT) {
        return arg*0.1;
    // the return value of ths essentially decides whether or not vehicles want to avoid construction of streets
    // if one of the streets on their path is blocked, it can avoid that route prematurely
    } else if (type == REROUTE) {
        return 0;
    }
}

std::string Influence::typeToName() const {
    switch (getType()) {
        case LIMIT:
            return "LIMIT";
        case STOP:
            return "STOP";
        case REROUTE:
            return "REROUTE";
        default:
            return "";
    }
}

void Influence::onWrite(std::ofstream &outputFile, const std::string &indent) const {
    std::string indents = indent + indent + indent + indent;
    outputFile << indents << indent << "type:  " << typeToName() << "\n"
               << indents << indent << "arg:   " << getArgument() << "\n";
}
