//
// Created by Thomas Gueutal on 17.05.20.
//

#ifndef TRAFFICSIMULATOR_UTIL_H
#define TRAFFICSIMULATOR_UTIL_H

#include "string"
#include <fstream>
#include "sstream"
#include "iostream"
#include "random"

    class Util {
    public:
        static std::string boolToString(bool b);

        static std::string boolToArrowString(bool isTwoWay, bool LR);

        static std::string isTwoWayToString(bool twoWay);

        static std::string boolToEnabledString(bool state);
    };



#endif //TRAFFICSIMULATOR_UTIL_H
