from models.Point import Point, xyzPoint
import math
import numpy as np


def sin(deg: float) -> float:
    # deg -> sin(deg)
    return math.sin(math.radians(deg))


def cos(deg: float) -> float:
    # deg -> cos(deg)
    return math.cos(math.radians(deg))


def asin(num: float) -> float:
    # sin -> deg
    return math.degrees(math.asin(num))


def acos(num: float) -> float:
    # cos -> deg
    return math.degrees(math.acos(num))


def atan(num: float) -> float:
    # tan -> deg
    return math.degrees(math.atan(num))


def ang2xyz(p: Point) -> xyzPoint:
    # Point -> xyzPoint
    p.check()
    z = p.alt * sin(p.lat)
    x = p.alt * cos(p.lat) * cos(p.lon)
    y = p.alt * cos(p.lat) * sin(p.lon)
    return xyzPoint(x, y, z)


def xyz2ang(p: xyzPoint) -> Point:
    # xyzPoint -> Point
    alt = math.sqrt(p.x * p.x + p.y * p.y + p.z * p.z)
    lat = asin(p.z / alt)
    lon = atan(p.y / p.x)
    if p.x < 0:
        lon += 180
    return Point(alt, lon, lat).check()


def calcPos(sat: "Satellite", time: float) -> Point:
    # calculate sat's position at time
    cons = sat.cons
    latShift = (
        360 * time / cons.cycle
        + 360 * sat.curPosInOrbit / cons.nSatPerOrbit
        + 360.0 * cons.offset * sat.curOrbit / cons.nOrbit / cons.nSatPerOrbit
    )
    res = Point(cons.alt, 0, latShift)
    lonShift = 360 * sat.curOrbit / cons.nOrbit + 360 * time / 86400
    res = rotate(res, xyzPoint(0, 0, 1), lonShift)
    res = rotate(res, ang2xyz(Point(1, lonShift, 0)), -(90 - cons.inclination))
    return res.check()


def rotate(p: Point, axis: xyzPoint, t: float) -> Point:
    # calculate p rotate around the axis for an angle t(deg). The direction is given by right hand rule
    # t stands for theta
    axis = axis.unitVec()
    a = axis.x
    b = axis.y
    c = axis.z
    rMatrix = [
        [
            cos(t) + a * a * (1 - cos(t)),
            a * b * (1 - cos(t)) - c * sin(t),
            a * c * (1 - cos(t)) + b * sin(t),
        ],
        [
            a * b * (1 - cos(t)) + c * sin(t),
            cos(t) + b * b * (1 - cos(t)),
            b * c * (1 - cos(t)) - a * sin(t),
        ],
        [
            a * c * (1 - cos(t)) - b * sin(t),
            b * c * (1 - cos(t)) + a * sin(t),
            cos(t) + c * c * (1 - cos(t)),
        ],
    ]
    xyzP = ang2xyz(p)
    rotatedP = np.matmul(rMatrix, [xyzP.x, xyzP.y, xyzP.z]).tolist()
    return xyz2ang(xyzPoint(rotatedP[0], rotatedP[1], rotatedP[2])).check()


def smallAngleBetween(v1: xyzPoint, v2: xyzPoint) -> float:
    # return the smaller angle between two vectors
    dotProduct = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
    angle = acos(dotProduct / v1.norm() / v2.norm())
    return min(angle, 180 - angle)


def calcIndex(curOrbit: int, curPosInOrbit: int, cons: "Constellation") -> int:
    # turn (curOrbit, curPosInOrbit) into index in cons.sats
    return (curOrbit - 1) * cons.nSatPerOrbit + curPosInOrbit - 1


def indexDecompose(index: int, cons: "Constellation") -> list:
    # decompose index in cons.sats into [curOrbit, curPosInOrbit]
    curOrbit = math.floor(index / cons.nSatPerOrbit) + 1
    curPosInOrbit = index - (curOrbit - 1) * cons.nSatPerOrbit + 1
    return [curOrbit, curPosInOrbit]


def satLatency(sat1: "Satellite", sat2: "Satellite", t: float) -> float:
    # calculate the latency between two satellite
    p1 = sat1.xyzPos(t)
    p2 = sat2.xyzPos(t)
    dis = math.sqrt(
        (p1.x - p2.x) * (p1.x - p2.x)
        + (p1.y - p2.y) * (p1.y - p2.y)
        + (p1.z - p2.z) * (p1.z - p2.z)
    )
    return dis / 299792458


def vector(p1: xyzPoint, p2: xyzPoint) -> xyzPoint:
    # return a vector pointing from p1 to p2
    return xyzPoint(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z)
