//
// Created by elthomaso on 17.05.20.
//

#include "Util.h"

std::string Util::boolToString(const bool b) {

    if (b) {
        return "true";
    } else {
        return "false";
    }
}

std::string Util::isTwoWayToString(bool twoWay) {
    if (twoWay) {
        return "twoWay";
    } else {
        return "oneWay";
    }
}
