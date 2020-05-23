
#include "jsonParser.h"
#include "StateElimination.h"


int main() {

    std::string fileNm = "STEL4";

    jsonParser parser;
    Network* city2 = parser.processJSON("../test_files/" + fileNm + ".json");
    std::string vehiclesOUTPUT = "jsonDriveTestVehicles.txt";
    std::string trafficLightsOUTPUT = "jsonDriveTestTrafficLights.txt";
    std::string vehicleChainOUTPUT = "vehicleChains.txt";
    /*
    city2->doMainLoop(300, vehiclesOUTPUT, trafficLightsOUTPUT, vehicleChainOUTPUT);

    Simulation::printSimVariables();
    */
    std::ofstream STELstream;
    STELstream.open(fileNm + ".dot");
    city2->toDot(STELstream);

    city2->toPNG(fileNm + ".dot");
    STELstream.close();


    StateElimination stateElimination;
    const std::vector<Intersection*>& network = city2->getNetwork();
    stateElimination.eliminate(city2, network[0], network[network.size()-1]);

    STELstream.open(fileNm + ".dot");
    //city2->toDotElim(STELstream);
    STELstream.close();

    return 0;
}
