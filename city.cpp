#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Constellation.h"
#include "Satellite.h"
using namespace std;
int main(int, char**) {
    cout << "subfolder id: ";
    int subfolderid;
    cin >> subfolderid;
    int routingStrategy = 3;
    string breakWayName[4] = {"Random", "HighLat", "Batch", "SunLight"};
    for (int i = 0; i < 4; i++) {
        cout << i << ": " << breakWayName[i] << endl;
    }
    cout << "Break way: ";
    int breakWayId = 0;
    cin >> breakWayId;

    int nOrbit = 72;
    int nSatPerOrbit = 22;
    int offset = -11;
    double cycle = 5731;
    double alt = 550;
    alt = (alt + 6378.1) * 1000;
    double inclination = 53;
    int ISLpattern = 1;
    // int routingStrategy = 1;
    Constellation cons(nOrbit, nSatPerOrbit, offset, cycle, alt, inclination, ISLpattern, routingStrategy);
    cons.phaseShift = 0;

    // cout << cons.sats[0]->xyzPos(0).str() << " " << cons.sats[10]->xyzPos(0).str() << " " <<
    // xyzPointLatency(cons.sats[0]->xyzPos(0), cons.sats[10]->xyzPos(0)) << endl;

    int start, end;
    cout << "Input start minute: ";
    cin >> start;
    start *= 60;
    cout << "Input end minute (exclusive): ";
    cin >> end;
    end *= 60;

    cons.buildISL(0);
    // cons.printISL();

    ofstream setUpFile("results/results" + to_string(subfolderid) + "/setup.txt");
    setUpFile << "nOrbit " << nOrbit << "\nnSatPerOrbit " << nSatPerOrbit << "\nphaseShift " << offset << "\ncycle "
              << cycle << "\nalt " << alt << "\ninclination " << inclination << "\nbreakWay "
              << breakWayName[breakWayId] << endl;
    double rate = 0, radius = 6378100;
    Point Beijing(radius, 116 + 180, 40), NewYork(radius, -74 + 180, 41), Paris(radius, 2 + 180, 49),
        Sydney(radius, 151 + 180, -34), CapeTown(radius, 18 + 180, -34);
    Point pairs[3][2];
    pairs[0][0] = NewYork;
    pairs[0][1] = Beijing;
    pairs[1][0] = Paris;
    pairs[1][1] = CapeTown;
    pairs[2][0] = Paris;
    pairs[2][1] = Sydney;
    string pairsName[3] = {"NewYork-Beijing", "Paris-CapeTown", "Paris-Sydney"};

    // You can simulate multiple ISL failure rate in one run.
    vector<double> rateV;
    rateV.push_back(5);
    // rateV.push_back(1);
    // rateV.push_back(0.1);
    // rateV.push_back(10);
    // rateV.push_back(0);

    for (auto ra = rateV.begin(); ra != rateV.end(); ra++) {
        double rate = *ra;
        ofstream out("results/results" + to_string(subfolderid) + "/city" + "_" + to_string(rate) + ".csv");
        out << "time,pairs_name,routingStrategy,success,fail,totLatency" << endl;
        cons.enableAllISL();
        if (breakWayName[breakWayId] == "Random") {
            cons.randomlyBreakISL(1.0 * rate / 100);
        } else if (breakWayName[breakWayId] == "Batch") {
            cons.breakISLinBatch(1.0 * rate / 100);
        }
        for (int t = start; t < end; t += 5) {
            if (breakWayName[breakWayId] == "HighLat") {
                cons.enableAllISL();
                cons.breakHighLatISL(1.0 * rate / 100, t);
            } else if (breakWayName[breakWayId] == "SunLight") {
                cons.enableAllISL();
                cons.breakISLforSunLight(t, 1);
            }
            cons.buildRoutingTable(t);
            // #pragma omp parallel for
            for (int pairsId = 0; pairsId < 3; pairsId++) {
                vector<Satellite*> startSats = nearest4Sat(pairs[pairsId][0], cons, t);
                // #pragma omp parallel for
                for (int routingStrategy = 1; routingStrategy <= 4; routingStrategy++) {
                    int fail = 0, success = 0;
                    double totLatency = 0;
                    // #pragma omp parallel for
                    for (auto startSat = startSats.begin(); startSat != startSats.end(); startSat++) {
                        double latency =
                            testRegionConnection(cons, *startSat, pairs[pairsId][1], t, true, routingStrategy);
                        if (latency < 0) {
                            if (latency > -2) {
                                fail++;
                            }
                        } else {
                            success += 1;
                            totLatency += latency;
                        }
                    }
                    out << to_string(t) + "," + pairsName[pairsId] + "," + to_string(routingStrategy) + "," +
                               to_string(success) + "," + to_string(fail) + "," + to_string(totLatency)
                        << endl;
                }
            }
            if (t % 10 == 0) {
                cout << 100.0 * (t - start) / (end - start) << "% rate=" << rate << " completed" << endl;
            }
        }
    }

    return 0;
}
