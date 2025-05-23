from .Point import Point, xyzPoint
import sys
from os.path import dirname, abspath
import heapq

if dirname(dirname(abspath(__file__))) not in sys.path:
    sys.path.append(dirname(dirname(abspath(__file__))))

from utils import calcPos, ang2xyz, calcIndex, satLatency


class Satellite:
    satelliteCnt = 0

    def __init__(self,  cons: 'Constellation', consId: int, curOrbit: int, curPosInOrbit: int) -> None:
        Satellite.satelliteCnt += 1
        self.id = Satellite.satelliteCnt
        self.cons = cons
        self.consId = consId
        self.available = True
        self.index = calcIndex(curOrbit, curPosInOrbit, cons)
        # print(consId)

        # ============= INITIAL POSITION ============
        self.curOrbit = curOrbit
        self.curPosInOrbit = curPosInOrbit

        # ============= INITIAL ISL ==============
        self.islCnt = 0
        self.islAvailability = []
        self.islTarget = []

        # ============= INITIAL ROUTING TABLE ================
        self.canVis = []
        # if self.canVis[i]=false, self.passTo[i] is undefined
        # i: index of sat in cons
        # passTo stores satellite object
        self.passTo = []
        self.latency = []

    def __str__(self) -> str:
        return f'satellite #{self.id} at ({self.curOrbit},{self.curPosInOrbit}) ({"on" if self.available else "off"})'

    def pos(self, t: float) -> Point:
        # return the alt lon lat position of a sat
        return calcPos(self, t).check()

    def xyzPos(self, t: float) -> xyzPoint:
        # return the xyz position of a sat
        return ang2xyz(calcPos(self, t).check())

    def addISL(self, target: 'Satellite') -> None:
        # add ISL with target
        try:
            self.islTarget.index(target)
        except:
            self.islCnt += 1
            self.islAvailability.append(True)
            self.islTarget.append(target)
        else:
            print(f'Error: {self} already had ISL with {target}. ')

    def resetISL(self) -> None:
        # reset ISL of a sat
        self.islCnt = 0
        self.islAvailability.clear()
        self.islTarget.clear()

    def removeISL(self, target: 'Satellite') -> None:
        # remove ISL with target
        if (self.islCnt == 0):
            print(
                f'Error: {self} has 0 ISL. Cannot remove {target} from ISL connection.')
            return
        try:
            index = self.islTarget.index(target)
            self.islTarget.remove(target)
            self.islAvailability.pop(index)
            self.islCnt -= 1
        except:
            print(f'Error: {self} has no ISL with {target}.')

    def disableISL(self, target: 'Satellite') -> None:
        # disable ISL with target
        if (self.islCnt == 0):
            print(
                f'Error: {self} has 0 ISL. Cannot disable {target} from ISL connection.')
            return
        try:
            index = self.islTarget.index(target)
            self.islAvailability[index] = False
        except:
            print(f'Error: {self} has no ISL with {target}.')

    def enableISL(self, target: 'Satellite') -> None:
        # enable ISL with target
        if (self.islCnt == 0):
            print(
                f'Error: {self} has 0 ISL. Cannot enable {target} from ISL connection.')
            return
        try:
            index = self.islTarget.index(target)
            self.islAvailability[index] = True
        except:
            print(f'Error: {self} has no ISL with {target}.')

    def enableAllISL(self) -> None:
        # enable all ISL in the satellite
        for i in range(0, self.islCnt):
            self.islAvailability[i] = True

    def shiftSat(self, orbitShift: int, phaseShift: int) -> 'Satellite':
        # calculate the satellite shifted by (orbitShift, phaseShift)
        # e.g. (1,1).shiftSat(-1,1)=(24,2), if the constellation has 24 orbits
        targetOrbit = self.curOrbit+orbitShift
        targetPosInOrbit = self.curPosInOrbit+phaseShift
        nOrbit = self.cons.nOrbit
        nSatPerOrbit = self.cons.nSatPerOrbit
        targetOrbit = ((targetOrbit-1) % nOrbit+nOrbit) % nOrbit+1
        targetPosInOrbit = ((targetPosInOrbit-1) %
                            nSatPerOrbit+nSatPerOrbit) % nSatPerOrbit+1
        return self.cons.getSat(targetOrbit, targetPosInOrbit)

    def printISL(self) -> None:
        # print the ISLs
        print(f'{self} is connected with:')
        for i in range(0, self.islCnt):
            print(
                f'    {"(on) " if self.islAvailability[i] else "(off)"}  {self.islTarget[i]}')

    def printISLandPos(self, t: float) -> None:
        # print ISLs and pos
        print(f'{self} is at {self.pos(t)}')
        self.printISL()

    def resetRoutingTable(self) -> None:
        # reset routing table of a sat
        self.canVis = [False for i in range(
            0, self.cons.nOrbit*self.cons.nSatPerOrbit+1)]
        self.passTo = [i for i in range(
            0, self.cons.nOrbit*self.cons.nSatPerOrbit+1)]
        self.latency = [-1 for i in range(0,
                                          self.cons.nOrbit*self.cons.nSatPerOrbit+1)]

    def buildRoutingTable(self, t) -> None:
        # build routing table for the satellite
        self.resetRoutingTable()
        cons = self.cons
        if (cons.routingStrategy == 1 or cons.routingStrategy == 2):
            buildRoutingTable1(self, t)

    def printRoutingTable(self) -> None:
        # print the routing table
        print(f"{self}'s routing table: ")
        for i in range(0, self.cons.nOrbit*self.cons.nSatPerOrbit):
            print(
                f'    To {self.cons.sats[i]} {"(can vis): pass to " if self.canVis[i] else "(not vis)"}{self.passTo[i] if self.canVis[i] else ""} {self.latency[i] if self.canVis[i] else ""}')


class HeapQNode:
    # the node class of heap queue in Dijkstra's algo
    def __init__(self, sat, latency):
        self.sat = sat
        self.latency = latency

    def __lt__(self, other: 'HeapQNode'):
        return self.latency < other.latency

    def __eq__(self, other: 'HeapQNode'):
        return self.latency == other.latency

    def __repr__(self) -> str:
        return f'({self.sat}: {self.latency})'


def buildRoutingTable1(sat: 'Satellite', t: float) -> None:
    # build routing table with strategy 1
    # print(f'building routing table for {sat}')
    hq = []
    cons = sat.cons
    heapq.heapify(hq)
    heapq.heappush(hq, HeapQNode(sat, 0))
    sat.latency = [-1 for i in range(0, cons.nOrbit*cons.nSatPerOrbit+1)]
    sat.latency[sat.index] = 0

    curSat = heapq.heappop(hq).sat
    sat.canVis[curSat.index] = True
    sat.passTo[curSat.index] = curSat
    for i in range(0, curSat.islCnt):
        targetSat = curSat.islTarget[i]
        islAvailability = curSat.islAvailability[i]
        if ((sat.latency[targetSat.index] == -1) or sat.latency[curSat.index]+satLatency(curSat, targetSat, t) < sat.latency[targetSat.index]):
            sat.latency[targetSat.index] = sat.latency[curSat.index] + \
                satLatency(curSat, targetSat, t)
            sat.passTo[targetSat.index] = targetSat
            heapq.heappush(hq, HeapQNode(
                targetSat, sat.latency[targetSat.index]))
            # print(f'    {targetSat} {sat.latency[targetSat.index]}')

    while (len(hq) > 0):
        # print(f'{curSat}: {sat.latency[curSat.index]}')
        # print(hq)
        curSat = heapq.heappop(hq).sat
        if (sat.canVis[curSat.index]):
            continue
        sat.canVis[curSat.index] = True
        for i in range(0, curSat.islCnt):
            targetSat = curSat.islTarget[i]
            islAvailability = curSat.islAvailability[i]
            if ((sat.latency[targetSat.index] == -1) or sat.latency[curSat.index]+satLatency(curSat, targetSat, t) < sat.latency[targetSat.index]):
                sat.latency[targetSat.index] = sat.latency[curSat.index] + \
                    satLatency(curSat, targetSat, t)
                sat.passTo[targetSat.index] = sat.passTo[curSat.index]
                heapq.heappush(hq, HeapQNode(
                    targetSat, sat.latency[targetSat.index]))
                # print(f'    {targetSat} {sat.latency[targetSat.index]}')
