#include "Satellite.h"
#include <cstring>
#include <iostream>
#include <queue>
#include "Constellation.h"
#include "utils.h"
using namespace std;
void Satellite::resetRoutingTable() {
    if (cons->routingStrategy == 1 || cons->routingStrategy == 3 || cons->routingStrategy == 6) {
        canVis.clear();
        passTo.clear();
        latency.clear();
        for (int i = 0; i < cons->nOrbit * cons->nSatPerOrbit; i++) {
            canVis.push_back(false);
            passTo.push_back(-1);
            latency.push_back(-1);
        }
    }
    if (cons->routingStrategy == 3) {
        passTo3.clear();
        passTo3Calced.clear();
        for (int i = 0; i < cons->nOrbit * cons->nSatPerOrbit; i++) {
            vector<int> tmp;
            passTo3.push_back(tmp);
            passTo3Calced.push_back(false);
        }
    }
}
void Satellite::resetISL() {
    islCnt = 0;
    islAvailability.clear();
    islTarget.clear();
}
void Satellite::enableAllISL() {
    for (int i = 0; i < islCnt; i++) {
        islAvailability[i] = true;
    }
}
void Satellite::addISL(Satellite* target) {
    for (auto sat = islTarget.begin(); sat != islTarget.end(); sat++) {
        if (*sat == target) {
            cout << "Error: " + this->str() + " already had ISL with " + target->str() + ". " << endl;
            return;
        }
    }
    islCnt++;
    islAvailability.push_back(true);
    islTarget.push_back(target);
}
void Satellite::disableISL(Satellite* target) {
    for (int i = 0; i < islCnt; i++) {
        if (islTarget[i] == target) {
            islAvailability[i] = false;
            return;
        }
    }
    cout << "Error: " + this->str() + " has no ISL with " + target->str() + ". " << endl;
}
void Satellite::enableISL(Satellite* target) {
    for (int i = 0; i < islCnt; i++) {
        if (islTarget[i] == target) {
            islAvailability[i] = true;
            return;
        }
    }
    cout << "Error: " + this->str() + " has no ISL with " + target->str() + ". " << endl;
}
Satellite* Satellite::shiftSat(int orbitShift, int phaseShift) {
    int targetOrbit = curOrbit + orbitShift;
    int targetPosInOrbit = curPosInOrbit + phaseShift;
    int nOrbit = cons->nOrbit;
    int nSatPerOrbit = cons->nSatPerOrbit;
    targetOrbit = ((targetOrbit - 1) % nOrbit + nOrbit) % nOrbit + 1;
    targetPosInOrbit = ((targetPosInOrbit - 1) % nSatPerOrbit + nSatPerOrbit) % nSatPerOrbit + 1;
    return cons->getSat(targetOrbit, targetPosInOrbit);
}
void Satellite::buildRoutingTable(double t) {
    resetRoutingTable();
    if (cons->routingStrategy == 1 || cons->routingStrategy == 3) {
        buildRoutingTable1(this, t);
    }
    if (cons->routingStrategy == 6) {
        buildRoutingTable6(this, t);
    }
}

void buildRoutingTable1(Satellite* sat, double t) {
    priority_queue<Node> q;
    sat->resetRoutingTable();
    sat->latency[sat->id] = 0;
    q.push(Node(sat, 0));

    sat->canVis[sat->id] = true;
    sat->passTo[sat->id] = -1;
    Satellite* curSat = sat;
    for (int i = 0; i < sat->islCnt; i++) {
        Satellite* targetSat = curSat->islTarget[i];
        // cout << targetSat->id << " " << targetSat->str() << endl
        //      << curSat->id << " " << curSat->str() << endl;
        if ((sat->latency[targetSat->id] == -1) ||
            sat->latency[curSat->id] + satLatency(curSat, targetSat, t) < sat->latency[targetSat->id]) {
            sat->latency[targetSat->id] = sat->latency[curSat->id] + satLatency(curSat, targetSat, t);
            sat->passTo[targetSat->id] = i;
            q.push(Node(targetSat, sat->latency[targetSat->id]));
        }
    }

    while (!q.empty()) {
        Satellite* curSat = q.top().sat;
        q.pop();
        if (sat->canVis[curSat->id]) {
            continue;
        }
        sat->canVis[curSat->id] = true;
        for (int i = 0; i < curSat->islCnt; i++) {
            Satellite* targetSat = curSat->islTarget[i];
            if ((sat->latency[targetSat->id] == -1) ||
                sat->latency[curSat->id] + satLatency(curSat, targetSat, t) < sat->latency[targetSat->id]) {
                sat->latency[targetSat->id] = sat->latency[curSat->id] + satLatency(curSat, targetSat, t);
                sat->passTo[targetSat->id] = sat->passTo[curSat->id];
                q.push(Node(targetSat, sat->latency[targetSat->id]));
            }
        }
    }
}

void buildRoutingTable6(Satellite* sat, double t) {
    priority_queue<Node> q;
    sat->resetRoutingTable();
    sat->latency[sat->id] = 0;
    q.push(Node(sat, 0));

    sat->canVis[sat->id] = true;
    sat->passTo[sat->id] = -1;
    Satellite* curSat = sat;
    for (int i = 0; i < sat->islCnt; i++) {
        Satellite* targetSat = curSat->islTarget[i];
        // cout << targetSat->id << " " << targetSat->str() << endl
        //      << curSat->id << " " << curSat->str() << endl;
        if ((sat->latency[targetSat->id] == -1) || sat->latency[curSat->id] + 0.001 < sat->latency[targetSat->id]) {
            sat->latency[targetSat->id] = sat->latency[curSat->id] + 0.001;
            sat->passTo[targetSat->id] = i;
            q.push(Node(targetSat, sat->latency[targetSat->id]));
        }
    }

    while (!q.empty()) {
        Satellite* curSat = q.top().sat;
        q.pop();
        if (sat->canVis[curSat->id]) {
            continue;
        }
        sat->canVis[curSat->id] = true;
        for (int i = 0; i < curSat->islCnt; i++) {
            Satellite* targetSat = curSat->islTarget[i];
            if ((sat->latency[targetSat->id] == -1) || sat->latency[curSat->id] + 0.001 < sat->latency[targetSat->id]) {
                sat->latency[targetSat->id] = sat->latency[curSat->id] + 0.001;
                sat->passTo[targetSat->id] = sat->passTo[curSat->id];
                q.push(Node(targetSat, sat->latency[targetSat->id]));
            }
        }
    }
}

void Satellite::buildRoutingTable3(Satellite* target, double t) {
    buildRoutingTable3Help(this, target, t);
}

void buildRoutingTable3Help(Satellite* sat, Satellite* target, double t) {
    if (sat->passTo3Calced[target->id]) {
        return;
    }
    sat->passTo3Calced[target->id] = true;

    Constellation* cons = sat->cons;
    bool used[1700][6] = {0};
    Satellite* curSat = sat;
    while (curSat != target && target != sat) {
        used[curSat->id][curSat->passTo[target->id]] = true;
        curSat = curSat->islTarget[curSat->passTo[target->id]];
    }
    priority_queue<Node3> q;
    double latency[1700] = {0};
    bool vis[1700] = {0};
    for (int i = 0; i < cons->nOrbit * cons->nSatPerOrbit; i++) {
        latency[i] = -1;
    }
    latency[sat->id] = 0;
    vector<int> tmp;
    q.push(Node3(sat, 0, tmp));
    while (!q.empty()) {
        Node3 curNode = q.top();
        q.pop();
        Satellite* curSat = curNode.sat;
        if (vis[curSat->id]) {
            continue;
        }
        vis[curSat->id] = true;
        if (curSat == target) {
            sat->passTo3[target->id] = curNode.pathRecord;
            break;
        }
        for (int i = 0; i < curSat->islCnt; i++) {
            if (used[curSat->id][i]) {
                continue;
            }
            Satellite* targetSat = curSat->islTarget[i];
            if ((latency[targetSat->id] == -1) ||
                latency[curSat->id] + satLatency(curSat, targetSat, t) < latency[targetSat->id]) {
                latency[targetSat->id] = latency[curSat->id] + satLatency(curSat, targetSat, t);
                vector<int> newPathRecord = curNode.pathRecord;
                newPathRecord.push_back(i);
                q.push(Node3(targetSat, latency[targetSat->id], newPathRecord));
            }
        }
    }
}

Point Satellite::pos(double t) {
    return calcPos(this, t).check();
}
xyzPoint Satellite::xyzPos(double t) {
    return ang2xyz(pos(t).check());
}
void Node::copyFrom(const Node& target) {
    sat = target.sat;
    dis = target.dis;
}
Node::Node(const Node& target) {
    copyFrom(target);
}
Node& Node::operator=(const Node& target) {
    copyFrom(target);
    return *this;
}
bool operator<(const Node& a, const Node& b) {
    return a.dis > b.dis;
}
void Node3::copyFrom(const Node3& target) {
    sat = target.sat;
    dis = target.dis;
    pathRecord = target.pathRecord;
}
Node3::Node3(const Node3& target) {
    copyFrom(target);
}
Node3& Node3::operator=(const Node3& target) {
    copyFrom(target);
    return *this;
}
bool operator<(const Node3& a, const Node3& b) {
    return a.dis > b.dis;
}
void Satellite::printISL() {
    cout << str() << " is connected with:" << endl;
    for (int i = 0; i < islCnt; i++) {
        cout << "    " << (islAvailability[i] ? "(on) " : "(off)") << "  " << islTarget[i]->str() << endl;
    }
}
void Satellite::printISLandPos(double t) {
    printPos(t);
    printISL();
}
void Satellite::printPos(double t) {
    cout << str() << " is at " << pos(t).str() << endl;
}
void Satellite::printRoutingTable(double t) {
    cout << str() << "'s routing table:" << endl;
    if (cons->routingStrategy == 1) {
        for (int i = 0; i < cons->nOrbit * cons->nSatPerOrbit; i++) {
            cout << "  To " << cons->sats[i]->str() << (canVis[i] ? " (can vis): pass to " : " (not vis)") << " "
                 << (canVis[i] ? islTarget[pass(cons->sats[i], t)]->str() : "") << endl;
        }
    }
    if (cons->routingStrategy == 3) {
        for (int i = 0; i < cons->nOrbit * cons->nSatPerOrbit; i++) {
            cout << "  To " << cons->sats[i]->str() << ": " << endl;
            Satellite* curSat = this;
            cout << "    ";
            while (curSat->id != i) {
                cout << curSat->passTo[i] << " ";
                curSat = curSat->islTarget[curSat->passTo[i]];
            }
            cout << endl;
            if (passTo3Calced[i]) {
                cout << "    ";
                curSat = this;
                for (auto islId = passTo3[i].begin(); islId != passTo3[i].end(); islId++) {
                    cout << *islId << " ";
                    curSat = curSat->islTarget[*islId];
                }
                cout << endl;
            }
        }
    }
}
int Satellite::pass(Satellite* target, double t) {
    return passTo[target->id];
}
int Satellite::pass(Point target, double t) {
    xyzPoint targetXyz = ang2xyz(target);
    double bestLatency = xyzPointLatency(targetXyz, xyzPos(t));
    int bestISL = -1;
    for (int i = 0; i < islCnt; i++) {
        if (!islAvailability[i]) {
            continue;
        }
        double iLatency = xyzPointLatency(targetXyz, islTarget[i]->xyzPos(t));
        if (iLatency < bestLatency) {
            bestLatency = iLatency;
            bestISL = i;
        }
    }
    return bestISL;
}
int Satellite::passByGrid(Satellite* target, double t) {
    double bestDist = gridDist(this, target, t);
    int bestISL = -1;
    for (int i = 0; i < islCnt; i++) {
        if (!islAvailability[i]) {
            continue;
        }
        double dist = gridDist(islTarget[i], target, t);
        if (dist < bestDist) {
            bestDist = dist;
            bestISL = i;
        }
    }
    return bestISL;
}
