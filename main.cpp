
#include "jsonParser.h"
#include "Network.h"


int main() {

    jsonParser parser;
    Network* city2 = parser.processJSON("../test_files/singleVehicle.json");
    std::string vehiclesOUTPUT = "jsonDriveTestVehicles.txt";
    std::string trafficLightsOUTPUT = "jsonDriveTestTrafficLights.txt";
    std::string vehicleChainOUTPUT = "vehicleChains.txt";

    city2->doMainLoop(300, vehiclesOUTPUT, trafficLightsOUTPUT, vehicleChainOUTPUT);

    Simulation::printSimVariables();

    return 0;
}
