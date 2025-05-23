from .Satellite import Satellite
from .Point import Point, xyzPoint
import sys
from os.path import dirname, abspath
import matplotlib as mpl
import matplotlib.pyplot as plt
from math import floor
import time
import json
import random
import numpy as np
import math

if dirname(dirname(abspath(__file__))) not in sys.path:
    sys.path.append(dirname(dirname(abspath(__file__))))


from utils import (
    calcIndex,
    satLatency,
    rotate,
    ang2xyz,
    smallAngleBetween,
    vector,
)


class Constellation:
    constellationCnt = 0

    def __init__(
        self,
        nOrbit: int,
        nSatPerOrbit: int,
        offset: float,
        cycle: float,
        alt: float,
        inclination: float,
        ISLpattern: int,
        routingStrategy: int,
    ) -> None:
        Constellation.constellationCnt += 1
        self.id = Constellation.constellationCnt

        # =============== PARAMETERS ==============
        self.nOrbit = nOrbit
        self.nSatPerOrbit = nSatPerOrbit
        self.cycle = cycle
        self.alt = alt
        self.inclination = inclination
        self.ISLpattern = ISLpattern
        self.routingStrategy = routingStrategy
        self.offset = offset

        # ============== INIT SATS ==============
        # 1<=curOrbit<=nOrbit
        # 1<=curPosInOrbit<=nSatPerOrbit
        self.sats = []
        for i in range(1, nOrbit + 1):
            for j in range(1, nSatPerOrbit + 1):
                self.sats.append(Satellite(self, self.id, i, j))

        # ============= INIT ROUTING TABLE ==============
        self.resetRoutingTable()

    def __str__(self) -> str:
        return f"constellation #{self.id}"

    def setPhaseShift(self, phaseShift: int) -> None:
        # set the phase shift of a ISL pattern of a cons
        self.phaseShift = phaseShift

    def resetISL(self) -> None:
        # reset the ISL of a cons
        for sat in self.sats:
            sat.resetISL()

    def enableAllISL(self) -> None:
        # enable all ISL in the constellation
        for sat in self.sats:
            sat.enableAllISL()

    def buildISL(self, t: float) -> None:
        # build the constellation's ISL
        print(f"===== Start building ISL for {self} =====")
        if self.ISLpattern == 1:
            buildISLpattern1(self)
        print(f"===== Complete building ISL for {self} =====")

    def resetRoutingTable(self) -> None:
        # reset routing table of a cons
        for sat in self.sats:
            sat.resetRoutingTable()

    def buildRoutingTable(self, t: float) -> None:
        # build routing table for the constellation
        print(f"===== Start building routing table for {self} =====")
        self.resetRoutingTable()
        cnt = 0
        tot = self.nOrbit * self.nSatPerOrbit
        target = tot / 10
        for sat in self.sats:
            sat.buildRoutingTable(t)
            cnt += 1
            if cnt >= target:
                print(f"    {cnt/tot*100}% complete")
                target += tot / 10
        print(f"===== Complete building routing table for {self} =====")

    def getSat(self, targetOrbit: int, targetPosInOrbit: int) -> "Satellite":
        # get sat by (orbit, pos in orbit)
        return self.sats[calcIndex(targetOrbit, targetPosInOrbit, self)]

    def printISL(self) -> None:
        # print ISL connection in cons
        print(f"ISL in {self} is:")
        for i in range(1, self.nOrbit + 1):
            print("------------------")
            for j in range(1, self.nSatPerOrbit + 1):
                self.getSat(i, j).printISL()

    def printISLandPos(self, t: float) -> None:
        # print ISL connection in cons
        print(f"ISL in {self} is:")
        for i in range(1, self.nOrbit + 1):
            print("------------------")
            for j in range(1, self.nSatPerOrbit + 1):
                self.getSat(i, j).printISLandPos(t)

    def plotOnlySats(self, t: float) -> None:
        # plot sats
        fig, ax = prePlot()
        addOnlySatToFig(self, fig, t)
        postPlot(fig)

    def plotSatsAndISL(self, t: float) -> None:
        # plot sats and isl
        fig, ax = prePlot()
        addOnlySatToFig(self, fig, t)
        addOnlyISLtoFig(self, fig, t)
        postPlot(fig)

    def plotSatsColoredWithLatencyAndISL(
        self, t: float, sat: Satellite
    ) -> None:
        # plot sats and isl
        fig, ax = prePlot()
        addOnlySatColoredWithLatencyToFig(self, fig, t, sat)
        addOnlyISLtoFig(self, fig, t)
        postPlot(fig)

    def plotSatsAndISLwithUtilization(self, t: float) -> None:
        # plot sats and isl
        # this function is not to be used
        fig, ax = prePlot()
        addOnlySatToFig(self, fig, t)
        utilization = [
            [
                [0 for k in range(0, self.getSat(i, j).islCnt)]
                for j in range(0, self.nSatPerOrbit)
            ]
            for i in range(0, self.nOrbit)
        ]
        for startSat in self.sats:
            for endSatIndex in range(
                startSat.index + 1, self.nOrbit * self.nSatPerOrbit
            ):
                endSat = self.sats[endSatIndex]
                # todo: finish
        postPlot(fig)

    def randomlyBreakISL(self, islFailureRate: float) -> None:
        # randomly break ISL link
        # 0<=islFailtureRate<=1
        tot = 0
        targetBreakCnt = 0
        for sat in self.sats:
            tot += sat.islCnt
        tot /= 2
        targetBreakCnt = floor(tot * islFailureRate)
        breakCnt = 0
        for sat in self.sats:
            if breakCnt == targetBreakCnt:
                break
            for i in range(0, sat.islCnt):
                if breakCnt == targetBreakCnt:
                    break
                if sat.islAvailability[i]:
                    disableISL(sat, sat.islTarget[i])
                    breakCnt += 1
        random.seed(time.time() * 100)
        for T in range(0, 2):
            for satA in self.sats:
                for i in range(0, satA.islCnt):
                    satB = satA.islTarget[i]
                    statusAB = satA.islAvailability[i]
                    satC = self.sats[
                        floor(random.random() * self.nOrbit * self.nSatPerOrbit)
                    ]
                    while satC == satA or satC == satB or satC.islCnt == 0:
                        satC = self.sats[
                            floor(
                                random.random()
                                * self.nOrbit
                                * self.nSatPerOrbit
                            )
                        ]
                    j = floor(random.random() * satC.islCnt)
                    satD = satC.islTarget[j]
                    statusCD = satC.islAvailability[j]
                    if statusAB:
                        enableISL(satC, satD)
                    else:
                        disableISL(satC, satD)
                    if statusCD:
                        enableISL(satA, satB)
                    else:
                        disableISL(satA, satB)

    def breakHighLatISL(self, islFailureRate: float, t: float) -> None:
        # break high lat ISL first
        tot = 0
        targetBreakCnt = 0
        for sat in self.sats:
            tot += sat.islCnt
        tot /= 2
        targetBreakCnt = floor(tot * islFailureRate)
        for i in range(0, targetBreakCnt):
            maxLat = 0
            for sat in self.sats:
                for i in range(0, sat.islCnt):
                    if sat.islAvailability[i]:
                        maxLat = max(maxLat, abs(sat.pos(t).lat))
                        break
            startIndex = random.randint(0, self.nOrbit * self.nSatPerOrbit - 1)
            breakFlag = False
            for i in [*range(startIndex, self.nOrbit * self.nSatPerOrbit)] + [
                *range(0, startIndex)
            ]:
                sat = self.sats[i]
                if abs(abs(sat.pos(t).lat) - maxLat) < 10:
                    startISL = random.randint(0, sat.islCnt - 1)
                    for j in [*range(startISL, sat.islCnt)] + [
                        *range(0, startISL)
                    ]:
                        if sat.islAvailability[j]:
                            disableISL(sat, sat.islTarget[j])
                            breakFlag = True
                            break
                    if breakFlag:
                        break

    def breakISLforSunLight(self, t: float, theta: float) -> None:
        # break ISL when angle between ISL and sunlight < 3 deg
        # theta is in deg, checkout the frame of reference
        # theta = 0 -> spring/autumn
        # theta = 90 -> summer/winter
        # t: time
        dist = 149600000000
        axialTilt = 23.5
        sunPos = Point(dist, theta + 180, 0).check()
        axis = xyzPoint(1, 0, 0)
        sunPos = rotate(sunPos, axis, axialTilt)
        # print(ang2xyz(sunPos))
        for sat in self.sats:
            for i in range(0, sat.islCnt):
                if sat.islAvailability[i]:
                    v1 = vector(ang2xyz(sunPos), sat.xyzPos(t))
                    v2 = vector(sat.xyzPos(t), sat.islTarget[i].xyzPos(t))
                    if smallAngleBetween(v1, v2) <= 5:
                        disableISL(sat, sat.islTarget[i])

    def breakISLforOperation(self, islFailureRate: float) -> None:
        # break ISL because of operation,
        # i.e. ISL in the same orbit are more likely to suffer from degradation
        tot = 0
        targetBreakCnt = 0
        for sat in self.sats:
            tot += sat.islCnt
        tot /= 2
        targetBreakCnt = floor(tot * islFailureRate)
        batchStart = []
        brokenCnt = 0
        while brokenCnt < targetBreakCnt:
            curBatch = random.randint(1, self.nOrbit)
            while curBatch in batchStart:
                curBatch += 1
                if curBatch == self.nOrbit:
                    curBatch = 1
            curOrbit = curBatch
            startPos = 3
            curPos = startPos
            while brokenCnt < targetBreakCnt:
                sat = self.getSat(curOrbit, curPos)
                for i in range(0, sat.islCnt):
                    if sat.islAvailability[i]:
                        disableISL(sat, sat.islTarget[i])
                        brokenCnt += 1
                        if brokenCnt == targetBreakCnt:
                            break
                curPos += 3
                if curPos > self.nSatPerOrbit:
                    startPos -= 1
                    if startPos == 0:
                        break
                    curPos = startPos
                    curOrbit += 1
                    if curOrbit > self.nOrbit:
                        curOrbit = 1

    def testAllConnectionPairs(self, t: float) -> list:
        # test the connectivity of current constellation
        # before calling this function, ISL and routing table must be built
        # return [successCnt, failCnt, failRate]
        successCnt = 0
        failCnt = 0
        for startSat in self.sats:
            for endSatIndex in range(
                startSat.index + 1, self.nOrbit * self.nSatPerOrbit
            ):
                endSat = self.sats[endSatIndex]
                curSat = startSat
                while True:
                    if not curSat.available:
                        failCnt += 1
                        break
                    # arrive
                    if self.routingStrategy == 1 or self.routingStrategy == 2:
                        if curSat == endSat:
                            successCnt += 1
                            break
                    elif self.routingStrategy == 3:
                        # for now, assume deg<30 is reached
                        timeDiff = 0.0012
                        if satLatency(endSat, curSat, t) < timeDiff:
                            successCnt += 1
                            break
                    # routing
                    if self.routingStrategy == 1:
                        passToSat = curSat.passTo[endSat.index]
                        islIndex = curSat.islTarget.index(passToSat)
                        if not curSat.islAvailability[islIndex]:
                            failCnt += 1
                            break
                        curSat = passToSat
                    # todo: self.routingStrategy==2 not complete, too slow, switch to cpp
                    elif self.routingStrategy == 3:
                        minLatency = satLatency(endSat, curSat, t)
                        minLatencyPassToSat = -1
                        for i in range(0, curSat.islCnt):
                            if curSat.islAvailability[i]:
                                if minLatency > satLatency(
                                    endSat, curSat.islTarget[i], t
                                ):
                                    minLatencyPassToSat = curSat.islTarget[i]
                                    minLatency = satLatency(
                                        endSat, curSat.islTarget[i], t
                                    )
                        if minLatencyPassToSat == -1:
                            failCnt += 1
                            break
                        else:
                            curSat = minLatencyPassToSat
        return [successCnt, failCnt, failCnt / (successCnt + failCnt)]

    def testAllConnectionPairsLatency(self, t: float) -> list:
        # test the average latency of all connection pairs
        # return a list [a,b,c]
        # a is the total latency measured in seconds
        # b is the number of accessible pairs
        # c is the number of unaccessible pairs
        totalLatency = 0
        successPairsCnt = 0
        failPairsCnt = 0
        maxJumps = floor(self.nSatPerOrbit * 2.5)
        maxLatency = 2
        for startSat in self.sats:
            if startSat.curPosInOrbit == 1:
                print(
                    f"    Start testing orbit #{startSat.curOrbit} {(startSat.curOrbit-1)/self.nOrbit*100}% completed"
                )
            for endSatIndex in range(
                startSat.index + 1, self.nOrbit * self.nSatPerOrbit
            ):
                endSat = self.sats[endSatIndex]
                # print(f'{startSat} -> {endSat}')
                curSat = startSat
                latency = 0
                nOfJumps = 0
                while True:
                    # print(curSat)
                    if not curSat.available:
                        failPairsCnt += 1
                        break
                    if curSat == endSat:
                        totalLatency += latency
                        successPairsCnt += 1
                        # print(nOfJumps)
                        break
                    if nOfJumps > maxJumps or latency > maxLatency:
                        failPairsCnt += 1
                        break
                    passToSat = curSat.passTo[endSat.index]
                    islIndex = curSat.islTarget.index(passToSat)
                    if curSat.islAvailability[islIndex]:
                        latency += satLatency(curSat, passToSat, t)
                        nOfJumps += 1
                        curSat = passToSat
                        continue
                    else:
                        if self.routingStrategy == 2:
                            startISLindex = random.randint(0, curSat.islCnt - 1)
                            passFlag = False
                            for i in [*range(startISLindex, curSat.islCnt)] + [
                                *range(0, startISLindex)
                            ]:
                                if curSat.islAvailability[i]:
                                    passToSat = curSat.islTarget[i]
                                    latency += satLatency(curSat, passToSat, t)
                                    nOfJumps += 1
                                    curSat = passToSat
                                    passFlag = True
                                    break
                            if not passFlag:
                                failPairsCnt += 1
                                break
                        else:
                            failPairsCnt += 1
                            break
        return [totalLatency, successPairsCnt, failPairsCnt]

    def plot3D(self, t: float) -> None:
        # Calculate figure parameters for exact pixel dimensions
        target_pixels = 2048
        dpi = 100
        fig_size = target_pixels / dpi  # 20.48 inches

        fig, ax = plt.subplots(
            figsize=(fig_size, fig_size), facecolor="black", dpi=dpi
        )

        # Remove padding around the plot
        plt.subplots_adjust(
            left=0, right=1, bottom=0, top=1, wspace=0, hspace=0
        )

        earth_img = plt.imread("cover/earth.jpg")
        img_radius = 1024  # 2048/2
        plot_radius = 1005  # 2010/2

        ax.imshow(
            earth_img,
            extent=[-img_radius, img_radius, -img_radius, img_radius],
            alpha=0.6,
        )
        ax.set_aspect("equal")
        ax.axis("off")
        ax.set_facecolor("black")

        limits = 110

        visible = []
        for sat in self.sats:
            pos = sat.pos(t)
            lon = (pos.lon - 180) % 360 - 180
            if -limits <= lon <= limits:
                lat_rad = math.radians(pos.lat)
                lon_rad = math.radians(lon)
                x = plot_radius * math.cos(lat_rad) * math.sin(lon_rad)
                y = plot_radius * math.sin(lat_rad)
                visible.append((x, y, sat))

        # Plot ISLs
        for x1, y1, sat1 in visible:
            for i in range(sat1.islCnt):
                target = sat1.islTarget[i]
                target_pos = target.pos(t)
                target_lon = (target_pos.lon - 180) % 360 - 180
                if -limits <= target_lon <= limits:
                    target_coords = next(
                        (
                            (x2, y2)
                            for x2, y2, sat2 in visible
                            if sat2 == target
                        ),
                        None,
                    )
                    if target_coords:
                        x2, y2 = target_coords
                        plt.plot(
                            [x1, x2],
                            [y1, y2],
                            color="gray" if sat1.islAvailability[i] else "red",
                            linewidth=2 if sat1.islAvailability[i] else 6,
                            linestyle="-" if sat1.islAvailability[i] else ":",
                            alpha=0.8,
                        )

        # Plot satellites
        for x, y, sat in visible:
            plt.plot(x, y, "o", markersize=12, color="#FFA500", alpha=0.9)

        plt.xlim(-img_radius, img_radius)
        plt.ylim(-img_radius, img_radius)

        # Save with exact dimensions
        plt.savefig(
            "cover/earth_sat.png", dpi=dpi, bbox_inches="tight", pad_inches=0
        )
        plt.close()


def buildISLpattern1(cons: Constellation) -> None:
    # build ISL pattern 1
    cons.resetISL()
    shift = cons.phaseShift
    for i in range(1, cons.nOrbit + 1):
        for j in range(1, cons.nSatPerOrbit + 1):
            satA = cons.getSat(i, j)

            satB = satA.shiftSat(0, 1)
            addISL(satA, satB)

            satB = satA.shiftSat(
                1, shift + (cons.offset if i == cons.nOrbit else 0)
            )
            addISL(satA, satB)


def addOnlySatColoredWithLatencyToFig(
    cons: Constellation, fig: "Fig", t: float, sat: Satellite
) -> "Fig":
    # add only sat points to fig
    x = []
    y = []
    colors = []
    for i in range(1, cons.nOrbit + 1):
        for j in range(1, cons.nSatPerOrbit + 1):
            pos = cons.getSat(i, j).pos(t)
            x.append(pos.lon - 180)
            y.append(pos.lat)
            colors.append(sat.latency[cons.getSat(i, j).index])
    plt.scatter(x, y, s=5, c=colors)


def addOnlySatToFig(cons: Constellation, fig: "Fig", t: float) -> "Fig":
    # add only sat points to fig
    x = []
    y = []
    for i in range(1, cons.nOrbit + 1):
        x.clear()
        y.clear()
        for j in range(1, cons.nSatPerOrbit + 1):
            pos = cons.getSat(i, j).pos(t)
            x.append(pos.lon - 180)
            y.append(pos.lat)
        plt.scatter(x, y, s=30)
        # plt.scatter(x, y, s=200)


def addOnlyISLtoFig(cons: Constellation, fig: "Fig", t: float) -> "Fig":
    # add only isl to fig
    x = []
    y = []
    for sat in cons.sats:
        for i in range(0, sat.islCnt):
            targetSat = sat.islTarget[i]
            x = [sat.pos(t).lon - 180, targetSat.pos(t).lon - 180]
            y = [sat.pos(t).lat, targetSat.pos(t).lat]
            if abs(x[0] - x[1]) < 200:
                plt.plot(
                    x,
                    y,
                    color="gray" if sat.islAvailability[i] else "red",
                    # linewidth=1 if sat.islAvailability[i] else 5,
                    linewidth=0.2 if sat.islAvailability[i] else 5,
                    linestyle="-" if sat.islAvailability[i] else ":",
                )


def prePlot() -> list:
    # conduct before plotting
    mpl.use("TkAgg")
    # fig, ax = plt.subplots(figsize=(14, 5))
    fig, ax = plt.subplots(figsize=(8, 5))
    plt.tight_layout(rect=[0.053, 0.086, 0.99, 0.99])
    # ax.set_xticks(range(-180, 181, 30))
    # ax.set_yticks(range(-90, 91, 20))
    ax.set_xticks(range(-180, 181, 20))
    ax.set_yticks(range(-90, 91, 10))
    # ax.set_xticks(range(-180, 181, 10))
    # ax.set_yticks(range(-90, 91, 5))
    ax.set_xlim([-180, 180])
    ax.set_ylim([-55, 55])
    ax.set_xlabel("Longitude [deg]", fontsize=23)
    ax.set_ylabel("Latitude [deg]", fontsize=23)
    plt.xticks(fontsize=23)
    plt.yticks(fontsize=23)
    ax.grid(alpha=0.05)
    return [fig, ax]


def postPlot(fig: "Fig") -> None:
    # conduct after fig is complete
    plt.show()
    plt.waitforbuttonpress()


def disableISL(satA: "Satellite", satB: "Satellite") -> None:
    # disable isl between satA and satB
    satA.disableISL(satB)
    satB.disableISL(satA)


def enableISL(satA: "Satellite", satB: "Satellite") -> None:
    # enable isl between satA and satB
    satA.enableISL(satB)
    satB.enableISL(satA)


def addISL(satA: "Satellite", satB: "Satellite") -> None:
    # add isl between satA and satB
    satA.addISL(satB)
    satB.addISL(satA)


def removeISL(satA: "Satellite", satB: "Satellite") -> None:
    # remove isl between satA and satB
    satA.removeISL(satB)
    satB.removeISL(satA)
