
#include "jsonParser.h"
#include "Network.h"


int main() {
    Network city1;

    Intersection* i1 = new Intersection("1");
    Intersection* i2 = new Intersection("2");
    Intersection* i3 = new Intersection("3");

    Street* s1 = new Street(i1, i2, A);
    s1->setIsTwoWay(true);
    Street* s2 = new Street(i1, i2, T);
    s2->setIsTwoWay(true);
    Street* s3 = new Street(i2, i3, B);
    s3->setIsTwoWay(true);
    Street* s4 = new Street(i2, i3, A);
    s4->setIsTwoWay(true);
    Street* s5 = new Street(i3, i1, A);
    s5->setIsTwoWay(true);

    Vehicle* v1 = new Vehicle(personal);
    Vehicle* v2 = new Vehicle(personal);

    v1->setCurrentStreet(s1);
    v2->setCurrentStreet(s4);
    v1->setUnderway(true);
    v2->setUnderway(true);
    v1->setProgress(90);
    v2->setProgress(89);
    v1->setPrevIntersection(i1);
    v1->setNextIntersection(i2);
    v2->setPrevIntersection(i3);
    v2->setNextIntersection(i2);


    i1->addStreet(s1);
    i1->addStreet(s2);
    i1->addStreet(s5);

    i2->addStreet(s1);
    i2->addStreet(s2);
    i2->addStreet(s3);
    i2->addStreet(s4);

    i3->addStreet(s3);
    i3->addStreet(s4);
    i3->addStreet(s5);

    city1.addIntersection(i1);
    city1.addIntersection(i2);
    city1.addIntersection(i3);

    // v1 and v2 are now both in front of the traffic lights
    s1->setFrontOccupant(v1, 0);
    s4->setFrontOccupant(v2, 1);

    // set up the traffic light pairs
    std::pair<Street*, Street*> trafficLightPair1(s1,s4);
    std::pair<Street*, Street*> trafficLightPair2(s2,s3);
    i2->setTrafficLights(city1.getSimulation()->getInfluence(STOP));
    i2->addTrafficLightPair(trafficLightPair1);
    i2->addTrafficLightPair(trafficLightPair2);
    i2->setCurrentPair(trafficLightPair1);  // the pair that starts off at red


    std::string fileName = "firstTest.dot";
    std::ofstream ofstream;
    ofstream.open(fileName);

    city1.toDot(ofstream);
    ofstream.close();

    city1.toPNG(fileName);


    int counter = 0;
    std::ofstream ofstream1;
    ofstream1.open("traffic_lights_test.txt");

    // TODO  a test to demonstrate how the traffic lights will work.
    //  The emission of signals is used in this implementation. Every time the traffic light counter of an
    //  intersection reaches 5, the intersection will send out a STOP signal to the front (member of Street)
    //  cars of the two Streets part of the currentTrafficLightPair. CurrTLPair is the pair of Streets that
    //  currently have green traffic lights, so the cars part of those streets can leave the streets and enter
    //  their chosen new ones.
    //  The file with the output if "traffic_lights_test.txt"

    while (counter != 25) {
        ofstream1 << "\ntotal: " << counter << "   trafficLights: " << i2->getTrafficLightCounter();

        v1->onWrite(ofstream1);
        v2->onWrite(ofstream1);

        i2->emitInfluences();   // TODO  BUG !! traffic lights only emit a STOP signal when changing current pair

        ++counter;
    }
    ofstream1.close();


    return 0;
}
