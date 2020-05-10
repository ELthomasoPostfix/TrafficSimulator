
#include "Network.h"

int main() {
    Network city1;

    Intersection* i1 = new Intersection("1");
    Intersection* i2 = new Intersection("2");
    Intersection* i3 = new Intersection("3");

    Street* s1 = new Street(i1, i2, A);
    Street* s2 = new Street(i1, i2, T);
    Street* s3 = new Street(i2, i3, B);
    Street* s4 = new Street(i2, i3, A);
    Street* s5 = new Street(i3, i1, A);

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

    std::string fileName = "firstTest.dot";
    std::ofstream ofstream;
    ofstream.open(fileName);

    city1.toDot(ofstream);
    ofstream.close();

    city1.toPNG(fileName);
    return 0;
}
