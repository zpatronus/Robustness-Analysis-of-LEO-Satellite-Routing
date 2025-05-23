#ifndef A8955C2A_15FE_4100_BDE8_73C586E2CE22
#define A8955C2A_15FE_4100_BDE8_73C586E2CE22
#include <string>
#include <vector>
#include "Satellite.h"
#include "utils.h"
using namespace std;
class Satellite;
class Region;
class Result {
   public:
    double success;
    double fail;
    double totLatency;
    double failRate() const;
    Result();
    Result(double success, double fail, double totLatency);
    void copyFrom(const Result& p);
    Result(const Result& p);
    Result& operator=(const Result& p);
};
class Constellation {
   public:
    int nOrbit;
    int nSatPerOrbit;
    double cycle;
    double alt;
    double inclination;
    int ISLpattern;
    int routingStrategy;
    double offset;
    vector<Satellite*> sats;
    int phaseShift;
    Constellation()
        : nOrbit(-1),
          nSatPerOrbit(-1),
          cycle(0),
          alt(0),
          inclination(0),
          ISLpattern(-1),
          routingStrategy(-1),
          offset(0),
          phaseShift(0) {}
    Constellation(int nOrbit,
                  int nSatPerOrbit,
                  double offset,
                  double cycle,
                  double alt,
                  double inclination,
                  int ISLpattern,
                  int routingStrategy);
    void resetRoutingTable();
    string str();
    void resetISL();
    void enableAllISL();
    void buildISL(double t);
    Satellite* getSat(int orbit, int posInOrbit);
    void buildRoutingTable(double t);
    void printISL();
    void printISLandPos(double t);
    void randomlyBreakISL(double islFailureRate);
    /**
     * @brief Get random ID from 0-nOrbit*nSatPerOrbit
     * remember to srand first
     *
     * @return int
     */
    int getRandomId();
    void breakHighLatISL(double islFailureRate, double t);
    void breakISLforSunLight(double t, double theta);
    void breakISLinBatch(double islFailureRate);
    Result testAllConnectionPairs(double t);
    Result testByRegions(double t, vector<Region>& regions);
};
void buildISLpattern1(Constellation& cons);
void addISL(Satellite* satA, Satellite* satB);
void disableISL(Satellite* satA, Satellite* satB);
void enableISL(Satellite* satA, Satellite* satB);
double testRegionConnection(Constellation& cons,
                            Satellite* startSat,
                            Point& endPos,
                            double t,
                            bool calcLatency,
                            int overRideRoutingStrategy);
double flood(Satellite* curSat,
             Satellite* endSat,
             vector<Satellite*>& endSats,
             xyzPoint xyzEndPos,
             bool* vis,
             double t);
#endif /* A8955C2A_15FE_4100_BDE8_73C586E2CE22 */
