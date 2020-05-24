
#include "jsonParser.h"
#include "StateElimination.h"


int main() {

    std::string fileNm = "STEL";

    jsonParser parser;
    Network* city2 = parser.processJSON("../test_files/" + fileNm + ".json");
    std::string vehiclesOUTPUT = "jsonDriveTestVehicles.txt";
    std::string trafficLightsOUTPUT = "jsonDriveTestTrafficLights.txt";
    std::string vehicleChainOUTPUT = "vehicleChains.txt";
    std::string networkOUTPUT = "networkOUTPUT.txt";
    /*
    city2->doMainLoop(300, vehiclesOUTPUT, trafficLightsOUTPUT, vehicleChainOUTPUT);

    Simulation::printSimVariables();
    */
    std::ofstream STELstream;
    STELstream.open(fileNm + ".dot");
    city2->toDot(STELstream);

    city2->toPNG(fileNm + ".dot");
    STELstream.close();


    const std::vector<Intersection*>& network = city2->getNetwork();

    // ########## sub network ###########
    Network* city3 = city2->getSubNetwork(network[0], network[3], 1);
    std::string subNetworkOUTPUT = "subNet";

    STELstream.open(subNetworkOUTPUT + ".dot");
    city3->toDot(STELstream);
    STELstream.close();
    // ##################################


    StateElimination stateElimination;
    stateElimination.eliminate(city2, network[0], network[network.size()-1]);
    std::cout << "start: " << network[0]->getName() << "   end: " << network[network.size()-1]->getName() << std::endl;

    STELstream.open(fileNm + ".dot");
    city2->toDotElim(STELstream);
    STELstream.close();


    STELstream.open(networkOUTPUT);
    city2->onWrite(STELstream);
    STELstream.close();

    return 0;
}
