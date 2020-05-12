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
