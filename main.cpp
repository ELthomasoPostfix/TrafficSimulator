
#include "jsonParser.h"
#include "Network.h"


int main() {

    jsonParser parser;
    Network* city2 = parser.processJSON("../test_files/example2.json");
    std::string vehiclesOUTPUT = "jsonDriveTestVehicles.txt";
    std::string trafficLightsOUTPUT = "jsonDriveTestTrafficLights.txt";
    std::string vehicleChainOUTPUT = "vehicleChains.txt";

    city2->doMainLoop(100, vehiclesOUTPUT, trafficLightsOUTPUT, vehicleChainOUTPUT);

    return 0;
}
