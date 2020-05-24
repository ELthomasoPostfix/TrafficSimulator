//
// Created by Thomas Gueutal on 17.05.20.
//

#include "Util.h"

std::string Util::boolToString(const bool b) {

    if (b) {
        return "true";
    } else {
        return "false";
    }
}

std::string Util::boolToArrowString(const bool isTwoWay, const bool LR) {
    std::string arrow = "<--";
    if (isTwoWay) arrow = "<-->";
    else if (LR) arrow = "-->";
    return arrow;
}

std::string Util::isTwoWayToString(bool twoWay) {
    if (twoWay) {
        return "twoWay";
    } else {
        return "oneWay";
    }
}