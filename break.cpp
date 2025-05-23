#include <fstream>
#include <iostream>
#include <string>
#include "Constellation.h"
#include "Satellite.h"
using namespace std;
int main(int, char**) {
    cout << "subfolder id: ";
    int subfolderid;
    cin >> subfolderid;
    cout
        << "Routing strategy: \n1: shortest path\n2: distributed\n3: multipath\n4: grid\n5: base line\n6: least hops\n";
    int routingStrategy;
    cin >> routingStrategy;
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

    ofstream setUpFile("../results/results" + to_string(subfolderid) + "/setup.txt");
    setUpFile << "nOrbit " << nOrbit << "\nnSatPerOrbit " << nSatPerOrbit << "\nphaseShift " << offset << "\ncycle "
              << cycle << "\nalt " << alt << "\ninclination " << inclination << "\nroutingStrategy " << routingStrategy
              << "\nbreakWay " << breakWayName[breakWayId] << endl;
    ifstream inFile("../all_in_one.csv");
    vector<Region> regions;
    loadRegion(inFile, regions);

    // printRegions(regions);

    int t = start;
    while (t < end) {
        cons.buildRoutingTable(t);
        ofstream out("../results/results" + to_string(subfolderid) + "/break" + breakWayName[breakWayId] +
                     to_string(t / 60) + ".csv");
        if (breakWayId < 3) {
            out << "ratio,success,failure,failRate,totLatency" << endl;
            double rate = 0;
            while (rate <= 100) {
                cons.enableAllISL();
                if (breakWayName[breakWayId] == "Random") {
                    cons.randomlyBreakISL(1.0 * rate / 100);
                } else if (breakWayName[breakWayId] == "Batch") {
                    cons.breakISLinBatch(1.0 * rate / 100);
                } else if (breakWayName[breakWayId] == "HighLat") {
                    cons.breakHighLatISL(1.0 * rate / 100, t);
                }
                // Result res = cons.testAllConnectionPairs(t);
                Result res = cons.testByRegions(t, regions);
                out << rate << "," << res.success << "," << res.fail << "," << res.failRate() << "," << res.totLatency
                    << endl;
                cout << rate << "% in " << 100.0 * (t - start) / (end - start) << "% complete" << endl;
                if (rate < 1) {
                    rate += 0.05;
                } else if (rate < 10) {
                    rate += 1;
                } else if (rate < 20) {
                    rate += 2;
                } else {
                    rate += 5;
                }
            }
        } else {
            out << "theta,success,failure,failRate,totLatency" << endl;
            for (double theta = 0; theta < 180; theta++) {
                cons.enableAllISL();
                cons.breakISLforSunLight(t, theta);
                Result res = cons.testByRegions(t, regions);
                out << theta << "," << res.success << "," << res.fail << "," << res.failRate() << "," << res.totLatency
                    << endl;
                cout << theta << " deg in " << 100.0 * (t - start) / (end - start) << "% complete" << endl;
            }
        }
        t += 60;
    }
    return 0;
}
