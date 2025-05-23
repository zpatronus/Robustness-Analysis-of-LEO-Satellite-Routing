#ifndef F3A7CE9B_26C4_48AF_A074_FE6B3A0E78CB
#define F3A7CE9B_26C4_48AF_A074_FE6B3A0E78CB
#include <string>
#include "Point.h"
using namespace std;
class Region {
   public:
    string name;
    double speed;
    double urbanPop;
    double ruralPop;
    double totalPop;
    Point pos;
    double landArea;
    double radius;
    double netizenRate;
    double urbanNetizen;
    double ruralNetizen;
    double totalNetizen;
    void copyFrom(const Region& p);
    Region();
    Region(const Region& p);
    Region& operator=(const Region& p);
    Region(string s);
    void print() const;
    Point randomPosNear() const;
};

#endif /* F3A7CE9B_26C4_48AF_A074_FE6B3A0E78CB */
