
#include "jsonParser.h"
#include "Network.h"


int main() {

    jsonParser parser;
    Network* city2 = parser.processJSON("../example2.json");
    std::string ofName1 = "jsonDriveTestVehicles.txt";
    std::string ofName2 = "jsonDriveTestTrafficLights.txt";
    city2->doMainLoop(100, ofName1, ofName2);

    return 0;
}
