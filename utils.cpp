#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include "Constellation.h"
#include "Point.h"
#include "Satellite.h"
using namespace std;
inline double radians(double deg) {
    return deg / 180 * PI;
}
inline double degrees(double rad) {
    return rad / PI * 180;
}
inline double dsin(double deg) {
    return sin(radians(deg));
}
inline double dcos(double deg) {
    return cos(radians(deg));
}
inline double dtan(double deg) {
    return tan(radians(deg));
}
inline double dasin(double num) {
    return degrees(asin(num));
}
inline double dacos(double num) {
    return degrees(acos(num));
}
inline double datan(double num) {
    return degrees(atan(num));
}
Point calcPos(Satellite* sat, double t) {
    Constellation* cons = sat->cons;
    double latShift = 360.0 * t / cons->cycle + 360.0 * sat->curPosInOrbit / cons->nSatPerOrbit +
                      360.0 * cons->offset * sat->curOrbit / cons->nOrbit / cons->nSatPerOrbit;
    Point res(cons->alt, 0, latShift);
    double lonShift = 360.0 * sat->curOrbit / cons->nOrbit + 360.0 * t / 86400.0;
    res = rotate(res, xyzPoint(0, 0, 1), lonShift);
    res = rotate(res, ang2xyz(Point(1, lonShift, 0)), -(90 - cons->inclination));
    return res.check();
}
xyzPoint ang2xyz(Point p) {
    p.check();
    double z = p.alt * dsin(p.lat);
    double x = p.alt * dcos(p.lat) * dcos(p.lon);
    double y = p.alt * dcos(p.lat) * dsin(p.lon);
    return xyzPoint(x, y, z);
}
Point xyz2ang(xyzPoint p) {
    double alt = p.norm();
    double lat = dasin(p.z / alt);
    double lon = datan(p.y / p.x);
    if (p.x < 0) {
        lon += 180;
    }
    return Point(alt, lon, lat).check();
}
Point rotate(Point p, xyzPoint axis, double t) {
    axis = axis.unitVec();
    double a = axis.x, b = axis.y, c = axis.z;
    double rMatrix[3][3] = {
        {dcos(t) + a * a * (1 - dcos(t)), a * b * (1 - dcos(t)) - c * dsin(t), a * c * (1 - dcos(t)) + b * dsin(t)},
        {a * b * (1 - dcos(t)) + c * dsin(t), dcos(t) + b * b * (1 - dcos(t)), b * c * (1 - dcos(t)) - a * dsin(t)},
        {a * c * (1 - dcos(t)) - b * dsin(t), b * c * (1 - dcos(t)) + a * dsin(t), dcos(t) + c * c * (1 - dcos(t))}};
    xyzPoint xyzP = ang2xyz(p);
    double xyzPMatrix[3] = {xyzP.x, xyzP.y, xyzP.z};
    double rotatedP[3] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            rotatedP[i] += rMatrix[i][j] * xyzPMatrix[j];
        }
    }
    return xyz2ang(xyzPoint(rotatedP[0], rotatedP[1], rotatedP[2]));
}
double smallAngleBetween(const xyzPoint& v1, const xyzPoint& v2) {
    double dotProduct = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    double angle = dacos(dotProduct / v1.norm() / v2.norm());
    return min(angle, 180 - angle);
}
int calcIndex(int orbit, int posInOrbit, Constellation& cons) {
    return (orbit - 1) * cons.nSatPerOrbit + posInOrbit - 1;
}
double pointLatency(Point p1, Point p2) {
    return xyzPointLatency(ang2xyz(p1), ang2xyz(p2));
}
double xyzPointLatency(const xyzPoint& p1, const xyzPoint& p2) {
    double dis = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) + (p1.z - p2.z) * (p1.z - p2.z));
    return dis / 299792458.0;
}
double satLatency(Satellite* sat1, Satellite* sat2, double t) {
    return xyzPointLatency(sat1->xyzPos(t), sat2->xyzPos(t));
}
xyzPoint vec(const xyzPoint& p1, const xyzPoint& p2) {
    return xyzPoint(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
}
vector<double> pos2OrbitAndPos(Point pos, Constellation& cons, double t, xyzPoint lonAxis) {
    pos = rotate(pos, lonAxis, 90 - cons.inclination);
    // cout << "---" << pos.lon << " " << xyz2ang(lonAxis).lon << endl;
    if (abs(xyz2ang(lonAxis).lon - pos.lon) > 10) {
        pos.lat = 180 - pos.lat;
    }
    double curOrbit =
        fmod(fmod(xyz2ang(lonAxis).lon - 360.0 * t / 86400.0, 360.0) + 360.0, 360.0) / 360.0 * cons.nOrbit;
    double curPosInOrbit =
        fmod(fmod(pos.lat - 360.0 * t / cons.cycle - 360.0 * cons.offset * curOrbit / cons.nOrbit / cons.nSatPerOrbit,
                  360.0) +
                 360.0,
             360.0) /
        360.0 * cons.nSatPerOrbit;
    vector<double> res = {curOrbit, curPosInOrbit};
    return res;
}
vector<double> nearestOrbitAndPos(Point pos, Constellation& cons, double t) {
    // [0-1]: 1st orbit and pos; [2-3]: 2nd orbit and pos
    pos.check();
    double sign = pos.lat >= 0 ? 1 : -1;
    if (abs(pos.lat) > cons.inclination) {
        pos.lat = (cons.inclination - 1) * sign;
    }
    double theta = dacos(dtan(pos.lat) / dtan(cons.inclination));
    // cout << "theta: " << theta << endl;
    vector<double> res1 = pos2OrbitAndPos(pos, cons, t, ang2xyz(Point(1, pos.lon + theta - 90, 0).check()));
    vector<double> res2 = pos2OrbitAndPos(pos, cons, t, ang2xyz(Point(1, pos.lon - theta - 90, 0).check()));
    vector<double> res = {res1[0], res1[1], res2[0], res2[1]};
    return res;
}
vector<Satellite*> nearest4Sat(Point pos, Constellation& cons, double t) {
    vector<Satellite*> res;
    vector<double> nearest = nearestOrbitAndPos(pos, cons, t);
    vector<Satellite*> res1 = nearest2Sat(pos, cons, t, nearest[0], nearest[1]);
    res.push_back(res1[0]);
    res.push_back(res1[1]);
    res1 = nearest2Sat(pos, cons, t, nearest[2], nearest[3]);
    res.push_back(res1[0]);
    res.push_back(res1[1]);
    return res;
}
vector<Satellite*> nearest2Sat(Point pos, Constellation& cons, double t, double orbit, double posInOrbit) {
    vector<Satellite*> res;
    int curOrbit = int(orbit + 0.5), curPosInOrbit = int(posInOrbit + 0.5);
    if (curOrbit == 0) {
        curOrbit = cons.nOrbit;
    }
    if (curPosInOrbit == 0) {
        curPosInOrbit = cons.nSatPerOrbit;
    }
    xyzPoint xyzPos = ang2xyz(pos);
    res.push_back(cons.getSat(curOrbit, curPosInOrbit));
    int bestI = -1, bestJ = -1;
    double bestLatency = 999999999;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue;
            Satellite* sat = res[0]->shiftSat(i, j);
            double satLatency = xyzPointLatency(xyzPos, sat->xyzPos(t));
            if (satLatency < bestLatency) {
                bestLatency = satLatency;
                bestI = i;
                bestJ = j;
            }
        }
    }
    res.push_back(res[0]->shiftSat(bestI, bestJ));
    return res;
}
void loadRegion(ifstream& inFile, vector<Region>& regions) {
    string line;
    getline(inFile, line);
    while (!inFile.eof()) {
        getline(inFile, line);
        if (line == "" || line == "\n")
            continue;
        regions.push_back(Region(line));
    }
}
void printRegions(const vector<Region>& regions) {
    cout << "region,speed,urban,rural,population,longitude,latitude,area,radius,netizen_rate,urban_netizen,rural_"
            "netizen,total_netizen"
         << endl;
    for (auto region = regions.begin(); region != regions.end(); region++) {
        (*region).print();
    }
}
int minDistOnRing(int posA, int posB, int ringSize) {
    int res = abs(posA - posB);
    res = min(res, min(abs(posA + ringSize - posB), abs(posB + ringSize - posA)));
    return res;
}
double gridDist(Satellite* sat1, Satellite* sat2, double t) {
    Constellation* cons = sat1->cons;
    int res = abs(sat1->curOrbit - sat2->curOrbit) +
              minDistOnRing(sat1->curPosInOrbit, sat2->curPosInOrbit, cons->nSatPerOrbit);
    res = min(res, abs(sat2->curOrbit + cons->nOrbit - sat1->curOrbit) +
                       minDistOnRing(sat1->curPosInOrbit + cons->offset, sat2->curPosInOrbit, cons->nSatPerOrbit));
    res = min(res, abs(sat1->curOrbit + cons->nOrbit - sat2->curOrbit) +
                       minDistOnRing(sat2->curPosInOrbit + cons->offset, sat1->curPosInOrbit, cons->nSatPerOrbit));
    res += satLatency(sat1, sat2, t);
    return res;
}
