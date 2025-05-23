#include "Region.h"
#include <iostream>
#include <sstream>
#include <string>
#include "utils.h"
using namespace std;
void Region::copyFrom(const Region& p) {
    name = p.name;
    speed = p.speed;
    urbanPop = p.urbanPop;
    ruralPop = p.ruralPop;
    totalPop = p.totalPop;
    pos = p.pos;
    landArea = p.landArea;
    radius = p.radius;
    netizenRate = p.netizenRate;
    urbanNetizen = p.urbanNetizen;
    ruralNetizen = p.ruralNetizen;
    totalNetizen = p.totalNetizen;
}
Region::Region()
    : name(""),
      speed(0),
      urbanPop(0),
      ruralPop(0),
      totalPop(0),
      pos(Point(0, 0, 0)),
      landArea(0),
      radius(0),
      netizenRate(0),
      urbanNetizen(0),
      ruralNetizen(0),
      totalNetizen(0) {}
Region::Region(const Region& p) {
    copyFrom(p);
}
Region& Region::operator=(const Region& p) {
    copyFrom(p);
    return *this;
}
Region::Region(string s) {
    istringstream line(s);
    double lon, lat;
    line >> name >> speed >> urbanPop >> ruralPop >> totalPop >> lon >> lat >> landArea >> netizenRate >>
        urbanNetizen >> ruralNetizen >> totalNetizen;
    pos = Point(6371000, lon + 180, lat);
    urbanPop /= 1000;
    ruralPop /= 1000;
    totalPop /= 1000;
    urbanNetizen /= 1000;
    ruralNetizen /= 1000;
    totalNetizen /= 1000;
    radius = sqrt(landArea / 3.1415926) * 1000 * 1.1;
}
void Region::print() const {
    cout << name << "," << speed << "," << urbanPop << "," << ruralPop << "," << totalPop << "," << pos.lon << ","
         << pos.lat << "," << landArea << "," << radius << "," << netizenRate << "," << urbanNetizen << ","
         << ruralNetizen << "," << totalNetizen << endl;
}
Point Region::randomPosNear() const {
    double rotateAngle = 360 * 1.0 * rand() / RAND_MAX;
    xyzPoint shiftPos = ang2xyz(pos);
    Point shift(1.0 * radius * (rand() + 1) / (RAND_MAX + 1), pos.lon + 90, 0);
    // cout << shift.alt << endl;
    shift.check();
    shift = rotate(shift, shiftPos, rotateAngle);
    xyzPoint shiftXyz = ang2xyz(shift);
    shiftPos.x += shiftXyz.x;
    shiftPos.y += shiftXyz.y;
    shiftPos.z += shiftXyz.z;
    Point res = xyz2ang(shiftPos);
    res.alt = 6378.1 * 1000;
    return res;
}
