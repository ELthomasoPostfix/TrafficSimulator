
#include "jsonParser.h"
#include "StateElimination.h"
#include "SSC.h"

Network* makeSubNetwork(std::ofstream& STELstream, Network* originalNetwork, float extraStepModifier);

void doStateElim(std::ofstream& STELstream, Network* originalNetwork, const std::string& networkOUTPUT,
                 const std::string& fileNm);

int main() {

    // TODO @@@@ disabling type compatibility
    // type compatibility between vehicles and streets now plays NO role whatsoever
    Simulation::setTypeCompatibilityState(false);
    std::cerr << "Turning off type compatibility from main function." << std::endl;
    // TODO @@@@

    std::string fileNm = "STELlarger";

    std::ofstream STELstream;



    jsonParser parser;
    Network* city2 = parser.processJSON("../test_files/" + fileNm + ".json");
    std::string vehiclesOUTPUT = "jsonDriveTestVehicles.txt";
    std::string trafficLightsOUTPUT = "jsonDriveTestTrafficLights.txt";
    std::string vehicleChainOUTPUT = "vehicleChains.txt";
    std::string networkOUTPUT = "networkOUTPUT.txt";

    city2->doMainLoop(300, vehiclesOUTPUT, trafficLightsOUTPUT, vehicleChainOUTPUT);

    city2->getSimulation()->setSpawnTimer(5);

    Network* city3 = SSC::set_up_ssc(city2);

    std::ofstream SSCStream;
    SSCStream.open("SSC.dot");
    city3->toDot(SSCStream);
    SSCStream.close();

    return 0;
}


Network* makeSubNetwork(std::ofstream& STELstream, Network* originalNetwork, const float extraStepModifier) {
    const std::vector<Intersection*>& network = originalNetwork->getNetwork();

    // ########## sub network ###########
    Network* city3 = originalNetwork->getSubNetwork(network[0], network[network.size()-1], extraStepModifier);
    originalNetwork->removeAllMultipurposeMarkers();

    std::string subNetworkOUTPUT = "subNet";

    STELstream.open(subNetworkOUTPUT + ".dot");
    city3->toDot(STELstream);
    STELstream.close();
    // ##################################
    return city3;
}

void doStateElim(std::ofstream& STELstream, Network* originalNetwork, const std::string& networkOUTPUT,
        const std::string& fileNm) {
    const std::vector<Intersection*>& network = originalNetwork->getNetwork();

    StateElimination stateElimination;
    stateElimination.eliminate(originalNetwork, network[0], network[network.size()-1]);
    std::cout << "start: " << network[0]->getName() << "   end: " << network[network.size()-1]->getName() << std::endl;

    STELstream.open(fileNm + ".dot");
    originalNetwork->toDotElim(STELstream);
    STELstream.close();


    STELstream.open(networkOUTPUT);
    originalNetwork->onWrite(STELstream);
    STELstream.close();

}
