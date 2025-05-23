#include "Constellation.h"
#include <omp.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include "Satellite.h"
#include "utils.h"
using namespace std;
Constellation::Constellation(int nOrbit,
                             int nSatPerOrbit,
                             double offset,
                             double cycle,
                             double alt,
                             double inclination,
                             int ISLpattern,
                             int routingStrategy)
    : nOrbit(nOrbit),
      nSatPerOrbit(nSatPerOrbit),
      cycle(cycle),
      alt(alt),
      inclination(inclination),
      ISLpattern(ISLpattern),
      routingStrategy(routingStrategy),
      offset(offset),
      phaseShift(0) {
    int id = -1;
    for (int i = 1; i <= nOrbit; i++) {
        for (int j = 1; j <= nSatPerOrbit; j++) {
            id++;
            Satellite* satpt = new Satellite(this, id, i, j);
            sats.push_back(satpt);
        }
    }
    resetRoutingTable();
}
void Constellation::resetRoutingTable() {
    for (int i = 0; i < nOrbit * nSatPerOrbit; i++) {
        sats[i]->resetRoutingTable();
    }
}
string Constellation::str() {
    return "constellation";
}
void Constellation::resetISL() {
    for (int i = 0; i < nOrbit * nSatPerOrbit; i++) {
        sats[i]->resetISL();
    }
}
void Constellation::enableAllISL() {
    for (int i = 0; i < nOrbit * nSatPerOrbit; i++) {
        sats[i]->enableAllISL();
    }
}
void Constellation::buildISL(double t) {
    puts("===== Start building ISL for constellation =====");
    switch (ISLpattern) {
        case 1: {
            buildISLpattern1(*this);
            break;
        }
        default:
            break;
    }
    puts("===== Complete building ISL for constellation =====");
}
Satellite* Constellation::getSat(int orbit, int posInOrbit) {
    return sats[calcIndex(orbit, posInOrbit, *this)];
}
void buildISLpattern1(Constellation& cons) {
    cons.resetISL();
    int shift = cons.phaseShift;
    for (int i = 1; i <= cons.nOrbit; i++) {
        for (int j = 1; j <= cons.nSatPerOrbit; j++) {
            Satellite* satA = cons.getSat(i, j);
            Satellite* satB = satA->shiftSat(0, 1);
            addISL(satA, satB);
            satB = satA->shiftSat(1, shift + ((i == cons.nOrbit) ? cons.offset : 0));
            addISL(satA, satB);
        }
    }
}
void addISL(Satellite* satA, Satellite* satB) {
    satA->addISL(satB);
    satB->addISL(satA);
}
void Constellation::buildRoutingTable(double t) {
    // puts("===== Start building routing table for constellation =====");
    // resetRoutingTable();
    if (routingStrategy == 1 || routingStrategy == 3 || routingStrategy == 5 || routingStrategy == 6) {
        int cnt = 0;
        int tot = nOrbit * nSatPerOrbit;
        double target = tot / 5.0;
#pragma omp parallel for num_threads(16) schedule(dynamic, 30)
        for (auto sat = sats.begin(); sat != sats.end(); sat++) {
            (*sat)->buildRoutingTable(t);
            cnt++;
            if (cnt >= target) {
                // cout << "    " << 1.0 * cnt / tot * 100 << "% complete" << endl;
                target += tot / 5.0;
            }
        }
    }
    // puts("===== Complete building routing table for constellation =====");
}
void Constellation::printISL() {
    puts("ISL in constellation is:");
    for (int i = 1; i <= nOrbit; i++) {
        puts("------------------");
        for (int j = 1; j <= nSatPerOrbit; j++) {
            getSat(i, j)->printISL();
        }
    }
}
void Constellation::printISLandPos(double t) {
    puts("ISL in constellation is:");
    for (int i = 1; i <= nOrbit; i++) {
        puts("------------------");
        for (int j = 1; j <= nSatPerOrbit; j++) {
            getSat(i, j)->printISLandPos(t);
        }
    }
}
void Constellation::randomlyBreakISL(double islFailureRate) {
    int tot = 0;
    int targetBreakCnt = 0;
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        tot += (*sat)->islCnt;
    }
    tot /= 2;
    targetBreakCnt = int(tot * islFailureRate + 0.5);
    int breakCnt = 0;
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        if (targetBreakCnt == breakCnt) {
            break;
        }
        for (int i = 0; i < (*sat)->islCnt; i++) {
            if (breakCnt == targetBreakCnt) {
                break;
            }
            if ((*sat)->islAvailability[i]) {
                disableISL((*sat), (*sat)->islTarget[i]);
                breakCnt++;
            }
        }
    }
    srand(time(NULL) * 100);
    for (int T = 0; T < 2; T++) {
        for (auto sat = sats.begin(); sat != sats.end(); sat++) {
            Satellite* satA = *sat;
            for (int i = 0; i < (*sat)->islCnt; i++) {
                Satellite* satB = satA->islTarget[i];
                bool statusAB = satA->islAvailability[i];
                Satellite* satC = sats[getRandomId()];
                while (satC == satA || satC == satB || satC->islCnt == 0) {
                    satC = sats[getRandomId()];
                }
                int j = rand() % satC->islCnt;
                Satellite* satD = satC->islTarget[j];
                bool statusCD = satC->islAvailability[j];
                if (statusAB != statusCD) {
                    if (statusAB) {
                        enableISL(satC, satD);
                        disableISL(satA, satB);
                    } else {
                        disableISL(satC, satD);
                        enableISL(satA, satB);
                    }
                }
            }
        }
    }
}
void disableISL(Satellite* satA, Satellite* satB) {
    satA->disableISL(satB);
    satB->disableISL(satA);
}
void enableISL(Satellite* satA, Satellite* satB) {
    satA->enableISL(satB);
    satB->enableISL(satA);
}
int Constellation::getRandomId() {
    int res = 0;
    for (int i = 0; i < 5; i++) {
        res += rand();
    }
    res = (res % (nOrbit * nSatPerOrbit) + (nOrbit * nSatPerOrbit)) % ((nOrbit * nSatPerOrbit));
    return res;
}
void Constellation::breakHighLatISL(double islFailureRate, double t) {
    srand(time(NULL) * 100);
    int tot = 0;
    int targetBreakCnt = 0;
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        tot += (*sat)->islCnt;
    }
    tot /= 2;
    targetBreakCnt = int(tot * islFailureRate + 0.5);
    double bias = 0;
    for (int T = 1; T <= targetBreakCnt; T++) {
        double maxLat = this->inclination * (0.9 - 1.0 * T / tot) - bias;
        int curId = getRandomId();
        bool isBreak = false;
        for (int S = 0; S < nOrbit * nSatPerOrbit; S++) {
            Satellite* sat = sats[curId];
            if (abs(sat->pos(t).lat) > maxLat) {
                int curISL = rand() % sat->islCnt;
                for (int R = 0; R < sat->islCnt; R++) {
                    if (sat->islAvailability[curISL]) {
                        disableISL(sat, sat->islTarget[curISL]);
                        isBreak = true;
                        break;
                    }
                    curISL++;
                    if (curISL == sat->islCnt) {
                        curISL = 0;
                    }
                }
            }
            if (isBreak) {
                break;
            }
            curId++;
            if (curId == nOrbit * nSatPerOrbit) {
                curId = 0;
            }
        }
        if (!isBreak) {
            bias += 5;
        }
    }
}
void Constellation::breakISLforSunLight(double t, double theta) {
    double dist = 149600000000.0;
    double axialTilt = 23.5;
    Point sunPos = Point(dist, theta + 180, 0).check();
    xyzPoint axis(1, 0, 0);
    sunPos = rotate(sunPos, axis, axialTilt);
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        for (int i = 0; i < (*sat)->islCnt; i++) {
            if ((*sat)->islAvailability[i]) {
                xyzPoint v1 = vec(ang2xyz(sunPos), (*sat)->xyzPos(t));
                xyzPoint v2 = vec((*sat)->xyzPos(t), (*sat)->islTarget[i]->xyzPos(t));
                if (smallAngleBetween(v1, v2) <= 5) {
                    disableISL((*sat), (*sat)->islTarget[i]);
                }
            }
        }
    }
}
void Constellation::breakISLinBatch(double islFailureRate) {
    srand(time(NULL) * 100);
    int tot = 0;
    int targetBreakCnt = 0;
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        tot += (*sat)->islCnt;
    }
    tot /= 2;
    targetBreakCnt = int(tot * islFailureRate + 0.5);
    bool batchStart[1000] = {0};
    int brokenCnt = 0;
    while (brokenCnt < targetBreakCnt) {
        int curBatch = rand() % nOrbit + 1;
        while (batchStart[curBatch]) {
            curBatch++;
            if (curBatch == nOrbit) {
                curBatch = 1;
            }
        }
        int curOrbit = curBatch;
        int startPos = 3;
        int curPos = startPos;
        while (brokenCnt < targetBreakCnt) {
            Satellite* sat = getSat(curOrbit, curPos);
            for (int i = 0; i < sat->islCnt; i++) {
                if (sat->islAvailability[i]) {
                    disableISL(sat, sat->islTarget[i]);
                    brokenCnt++;
                    if (brokenCnt == targetBreakCnt) {
                        break;
                    }
                }
            }
            curPos += 3;
            if (curPos > nSatPerOrbit) {
                startPos--;
                if (startPos == 0) {
                    break;
                }
                curPos = startPos;
                curOrbit++;
                if (curOrbit > nOrbit) {
                    curOrbit = 1;
                }
            }
        }
    }
}
Result::Result() : success(0), fail(0), totLatency(0) {}
Result::Result(double success, double fail, double totLatency) : success(success), fail(fail), totLatency(totLatency) {}
void Result::copyFrom(const Result& p) {
    success = p.success;
    fail = p.fail;
    totLatency = p.totLatency;
}
Result::Result(const Result& p) {
    copyFrom(p);
}
Result& Result::operator=(const Result& p) {
    copyFrom(p);
    return *this;
}
double Result::failRate() const {
    if (success + fail == 0) {
        return 0;
    }
    return 1.0 * fail / (fail + success);
}
double testSatConnection(Constellation& cons, Satellite* startSat, Satellite* endSat, double t) {
    Satellite* curSat = startSat;
    double latency = 1e-9;
    while (true) {
        if (curSat->isAvailable == false || latency > 1) {
            return -1;
        }
        if (curSat == endSat) {
            return latency;
        }
        int islId = curSat->pass(endSat, t);
        if (islId == -1 || curSat->islAvailability[islId] == false) {
            return -1;
        }
        if (cons.routingStrategy == 2) {
            latency += satLatency(curSat, curSat->islTarget[islId], t);
        }
        curSat = curSat->islTarget[islId];
    }
}
Result testAllConnectionPairsParallel(Satellite* startSat, double t) {
    Constellation& cons = *(startSat->cons);
    double success = 0, fail = 0;
    double totLatency = 0;
    for (int j = startSat->id + 1; j < cons.nOrbit * cons.nSatPerOrbit; j++) {
        // Satellite* endSat = cons.sats[j];
        double latency = testSatConnection(*(startSat->cons), startSat, cons.sats[j], t);
        if (latency < 0) {
            fail++;
        } else {
            success++;
            totLatency += latency;
        }
        // while (true) {
        //     if (curSat->isAvailable == false || latency > 1) {
        //         fail++;
        //         break;
        //     }
        //     if (curSat == endSat) {
        //         // TODO: this should be changed according to different routing strategy
        //         success++;
        //         totLatency += latency;
        //         break;
        //     }
        //     int islId = curSat->pass(endSat, t);
        //     if (curSat->islAvailability[islId] == false) {
        //         fail++;
        //         break;
        //     }
        //     if (cons.routingStrategy == 2) {
        //         latency += satLatency(curSat, curSat->islTarget[islId], t);
        //     }
        //     curSat = curSat->islTarget[islId];
        // }
    }
    return Result(success, fail, totLatency);
}
Result Constellation::testAllConnectionPairs(double t) {
    double success = 0, fail = 0;
    double totLatency = 0;
#pragma omp parallel for num_threads(16) schedule(dynamic, 50)
    for (auto sat = sats.begin(); sat != sats.end(); sat++) {
        Result res = testAllConnectionPairsParallel((*sat), t);
        success += res.success;
        fail += res.fail;
        totLatency += res.totLatency;
    }
    return Result(success, fail, totLatency);
}
bool reachTargetRegion(xyzPoint& regionPos, vector<Satellite*>& endSats, Satellite* curSat, double t) {
    if (3.7e-3 > xyzPointLatency(regionPos, curSat->xyzPos(t))) {
        return true;
    }
    for (auto eSat = endSats.begin(); eSat != endSats.end(); eSat++) {
        if (*eSat == curSat) {
            return true;
        }
    }
    return false;
}
double flood(Satellite* curSat,
             Satellite* endSat,
             vector<Satellite*>& endSats,
             xyzPoint xyzEndPos,
             bool* vis,
             double t) {
    if (vis[curSat->id])
        return -1;
    if (reachTargetRegion(xyzEndPos, endSats, curSat, t))
        return 1e-9;
    vis[curSat->id] = true;
    for (int i = 0; i < curSat->islCnt; i++) {
        if (curSat->islAvailability[i]) {
            if (flood(curSat->islTarget[i], endSat, endSats, xyzEndPos, vis, t) > 0) {
                return 1e-9;
            }
        }
    }
    return -1;
}
double testRegionConnection(Constellation& cons,
                            Satellite* startSat,
                            Point& endPos,
                            double t,
                            bool calcLatency,
                            int overRideRoutingStrategy) {
    // -1: fail
    // -3: meaningless result
    // >0: success, meaning latency
    int actualRoutingStrategyUsed = overRideRoutingStrategy == 0 ? cons.routingStrategy : overRideRoutingStrategy;
    if (abs(endPos.lat) > cons.inclination) {
        endPos.lat = endPos.lat > 0 ? cons.inclination : -cons.inclination;
    }
    vector<Satellite*> endSats = nearest4Sat(endPos, cons, t);
    Satellite* endSat = endSats[0];
    Satellite* curSat = startSat;
    double latency = 1e-11;
    xyzPoint xyzEndPos = ang2xyz(endPos);
    bool distributedTurnsToGrid = false;
    for (auto eSat = endSats.begin(); eSat != endSats.end(); eSat++) {
        if (gridDist(curSat, *eSat, t) < gridDist(curSat, endSat, t)) {
            endSat = *eSat;
        }
    }
    if (actualRoutingStrategyUsed == 5) {
        if (startSat == endSat) {
            latency = -3;
        } else {
            bool* vis = new bool[cons.nOrbit * cons.nSatPerOrbit];
            for (int i = 0; i < cons.nOrbit * cons.nSatPerOrbit; i++)
                vis[i] = false;
            latency = flood(startSat, endSat, endSats, xyzEndPos, vis, t);
            delete[] vis;
        }
        return latency;
    }
    while (true) {
        if (curSat->isAvailable == false || latency > 1) {
            latency = -1;
            break;
        }
        if (reachTargetRegion(xyzEndPos, endSats, curSat, t)) {
            if (curSat == startSat) {
                latency = -3;
            }
            break;
        }
        int islId;
        if (actualRoutingStrategyUsed == 2) {
            if (!distributedTurnsToGrid) {
                islId = curSat->pass(endPos, t);
                if (islId == -1) {
                    distributedTurnsToGrid = true;
                    for (auto eSat = endSats.begin(); eSat != endSats.end(); eSat++) {
                        if (gridDist(curSat, *eSat, t) < gridDist(curSat, endSat, t)) {
                            endSat = *eSat;
                        }
                    }
                }
            }
            if (distributedTurnsToGrid) {
                islId = curSat->passByGrid(endSat, t);
            }
        } else if (actualRoutingStrategyUsed == 1 || actualRoutingStrategyUsed == 6) {
            islId = curSat->pass(endSat, t);
        } else if (actualRoutingStrategyUsed == 3) {
            islId = curSat->pass(endSat, t);
        } else if (actualRoutingStrategyUsed == 4) {
            islId = curSat->passByGrid(endSat, t);
        }
        if (islId == -1 || curSat->islAvailability[islId] == false) {
            latency = -1;
            break;
        }
        // optional feature
        if (calcLatency) {
            latency += satLatency(curSat, curSat->islTarget[islId], t);
        }
        curSat = curSat->islTarget[islId];
    }
    if (actualRoutingStrategyUsed == 3) {
        double path2Latency = 1e-11;
        bool isReached = false;
        if (!startSat->passTo3Calced[endSat->id]) {
            startSat->buildRoutingTable3(endSat, t);
        }
        Satellite* curSat = startSat;
        for (auto islId = startSat->passTo3[endSat->id].begin(); islId != startSat->passTo3[endSat->id].end();
             islId++) {
            if (curSat->isAvailable == false || path2Latency > 1) {
                path2Latency = -1;
                break;
            }
            bool isReachedd = false;
            if (reachTargetRegion(xyzEndPos, endSats, curSat, t)) {
                isReached = true;
                if (curSat == startSat) {
                    path2Latency = -3;
                }
                break;
            }
            if (*islId == -1 || curSat->islAvailability[*islId] == false) {
                path2Latency = -1;
                break;
            }
            if (calcLatency) {
                path2Latency += satLatency(curSat, curSat->islTarget[*islId], t);
            }
            curSat = curSat->islTarget[*islId];
        }
        if (curSat->isAvailable == false || path2Latency > 1) {
            path2Latency = -1;
        }
        if (reachTargetRegion(xyzEndPos, endSats, curSat, t)) {
            isReached = true;
            if (curSat == startSat) {
                path2Latency = -3;
            }
        }
        if (isReached) {
            latency = latency < 0 ? path2Latency : min(latency, path2Latency);
        }
    }
    return latency;
}
Result testByRegionsParellel(const Region& rStart, const Region& rEnd, Constellation& cons, double t) {
    double success = 0, fail = 0, totLatency = 0;
    // cout << pStart.str() << " " << pEnd.str() << endl;
    vector<Satellite*> startSats = nearest4Sat(rStart.randomPosNear(), cons, t);
    Point endPos = rEnd.randomPosNear();
    for (auto startSat = startSats.begin(); startSat != startSats.end(); startSat++) {
        double latency = testRegionConnection(cons, *startSat, endPos, t, false, 0);
        // cout << latency << " ";
        if (latency < 0) {
            if (latency > -2) {
                fail += rStart.totalNetizen * rEnd.totalNetizen * rStart.speed * rEnd.speed;
            }
        } else {
            success += rStart.totalNetizen * rEnd.totalNetizen * rStart.speed * rEnd.speed;
            totLatency += latency;
        }
    }
    return Result(success, fail, totLatency);
}
Result Constellation::testByRegions(double t, vector<Region>& regions) {
    double success = 0, fail = 0;
    double totLatency = 0;
    srand(time(NULL) * 100);
    int size = regions.size();
    int N = size * size;
#pragma omp parallel for num_threads(16) schedule(dynamic, 50)
    for (int i = 0; i < N; i++) {
        Result res = testByRegionsParellel(regions[i / size], regions[i % size], *this, t);
        success += res.success;
        fail += res.fail;
        totLatency += res.totLatency;
    }
    return Result(success, fail, totLatency);
}
