#ifndef E117084B_96EE_462C_9B10_D020C5F8EAC4
#define E117084B_96EE_462C_9B10_D020C5F8EAC4
#include <cmath>
#include "Constellation.h"
#include "Point.h"
#include "Region.h"
#include "Satellite.h"
#define PI (3.1415926535)
class Satellite;
class Constellation;
class Region;
inline double radians(double deg);
inline double degrees(double rad);
inline double dsin(double deg);
inline double dcos(double deg);
inline double dtan(double deg);
inline double dasin(double num);
inline double dacos(double num);
inline double datan(double num);
xyzPoint ang2xyz(Point p);
Point xyz2ang(xyzPoint p);
Point calcPos(Satellite* sat, double t);
Point rotate(Point p, xyzPoint axis, double t);
double smallAngleBetween(const xyzPoint& v1, const xyzPoint& v2);
int calcIndex(int orbit, int posInOrbit, Constellation& cons);
double pointLatency(Point p1, Point p2);
double xyzPointLatency(const xyzPoint& p1, const xyzPoint& p2);
double satLatency(Satellite* sat1, Satellite* sat2, double t);
xyzPoint vec(const xyzPoint& p1, const xyzPoint& p2);
vector<double> pos2OrbitAndPos(Point pos, Constellation& cons, double t, xyzPoint lonAxis);
vector<double> nearestOrbitAndPos(Point pos, Constellation& cons, double t);
vector<Satellite*> nearest4Sat(Point pos, Constellation& cons, double t);
vector<Satellite*> nearest2Sat(Point pos, Constellation& cons, double t, double orbit, double posInOrbit);
void loadRegion(ifstream& inFile, vector<Region>& regions);
void printRegions(const vector<Region>& regions);
int minDistOnRing(int posA, int posB, int ringSize);
double gridDist(Satellite* sat1, Satellite* sat2, double t);
#endif /* E117084B_96EE_462C_9B10_D020C5F8EAC4 */
