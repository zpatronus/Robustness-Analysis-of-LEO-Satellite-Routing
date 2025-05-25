#ifndef A1C70C3F_C4D7_4363_8E01_DA232F61F44D
#define A1C70C3F_C4D7_4363_8E01_DA232F61F44D
#include <string>
#include <vector>
#include "Constellation.h"
#include "Point.h"
#include "utils.h"
using namespace std;
class Constellation;
class Satellite {
   private:
    void copyFrom(const Satellite& sat) {
        id = sat.id;
        cons = sat.cons;
        isAvailable = sat.isAvailable;
        curOrbit = sat.curOrbit;
        curPosInOrbit = sat.curPosInOrbit;
        islCnt = sat.islCnt;
        isAvailable = sat.isAvailable;
        islTarget = sat.islTarget;
        canVis = sat.canVis;
        passTo = sat.passTo;
        latency = sat.latency;
        passTo3 = sat.passTo3;
        passTo3Calced = sat.passTo3Calced;
    }

   public:
    int id;
    Constellation* cons;
    bool isAvailable;
    int curOrbit;
    int curPosInOrbit;
    int islCnt;
    vector<bool> islAvailability;
    vector<Satellite*> islTarget;
    vector<bool> canVis;
    /**
     * @brief passTo[i] indicate isl[passTo[i]] should be used when passing to sats[i]
     *
     */
    vector<int> passTo;
    vector<double> latency;
    vector<vector<int>> passTo3;
    vector<bool> passTo3Calced;
    Satellite() : id(-1), cons(nullptr), isAvailable(false), curOrbit(-1), curPosInOrbit(-1), islCnt(-1) {}
    Satellite(Constellation* cons, const int id, const int curOrbit, const int curPosInOrbit)
        : id(id), cons(cons), isAvailable(true), curOrbit(curOrbit), curPosInOrbit(curPosInOrbit), islCnt(0) {}
    Satellite(const Satellite& sat) { copyFrom(sat); }
    Satellite& operator=(const Satellite& sat) {
        copyFrom(sat);
        return *this;
    }
    string str() {
        return "satellite #" + to_string(id + 1) + " at (" + to_string(curOrbit) + "," + to_string(curPosInOrbit) +
               ") (" + (isAvailable ? "on" : "off") + ")";
    }
    void resetRoutingTable();
    void resetISL();
    void enableAllISL();
    void addISL(Satellite* target);
    void disableISL(Satellite* target);
    void enableISL(Satellite* target);
    Satellite* shiftSat(int orbitShift, int phaseShift);
    void buildRoutingTable(double t);
    void buildRoutingTable3(Satellite* target, double t);
    Point pos(double t);
    xyzPoint xyzPos(double t);
    void printISL();
    void printPos(double t);
    void printISLandPos(double t);
    void printRoutingTable(double t);
    /**
     * @brief pass() indicate isl[pass()] should be used when passing to certain satellite
     *
     * @param target
     * @param t
     * @return int
     */
    int pass(Satellite* target, double t);
    int pass(Point target, double t);
    int passByGrid(Satellite* target, double t);
    // Point pos(const double t) {
    //     // TODO: this part is not finished.
    //     return calcPos(this, t).check();
    // }
};
void buildRoutingTable1(Satellite* sat, double t);
void buildRoutingTable6(Satellite* sat, double t);
void buildRoutingTable3Help(Satellite* sat, Satellite* target, double t);
class Node {
   public:
    Satellite* sat;
    double dis;
    Node() : sat(nullptr), dis(-1) {}
    Node(Satellite* sat, double dis) : sat(sat), dis(dis) {}
    void copyFrom(const Node& target);
    Node(const Node& target);
    Node& operator=(const Node& target);
};
bool operator<(const Node& a, const Node& b);
class Node3 {
   public:
    Satellite* sat;
    double dis;
    vector<int> pathRecord;
    Node3() : sat(nullptr), dis(-1) {}
    Node3(Satellite* sat, double dis, vector<int> pathRecord) : sat(sat), dis(dis), pathRecord(pathRecord) {}
    void copyFrom(const Node3& target);
    Node3(const Node3& target);
    Node3& operator=(const Node3& target);
};
bool operator<(const Node3& a, const Node3& b);
#endif /* A1C70C3F_C4D7_4363_8E01_DA232F61F44D */
