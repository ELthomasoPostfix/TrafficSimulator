//
// Created by Thomas Gueutal on 10.05.20.
//

#include "Influence.h"

Influence::Influence(influenceType type) : _type(type) {

}



// getters and setters

influenceType Influence::getType() const {
    return _type;
}
