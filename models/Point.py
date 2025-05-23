import math


def mod360(num: float) -> float:
    # Mod a float by 360 and return a positive num
    return math.fmod(math.fmod(num, 360)+360, 360)


class Point:
    # A point class {altitude, longitude, latitude}
    def __init__(self, alt: float = 0, lon: float = 0, lat: float = 0) -> None:
        self.alt = alt
        self.lat = lat
        self.lon = lon

    def __str__(self) -> str:
        return f'Lon: {round(self.lon-180,4)}, Lat: {round(self.lat,4)}'

    def check(self) -> None:
        # turn a point into proper format
        # proper format: -90<=lat<=90 0<=lon<=360
        self.lat = mod360(self.lat)
        self.lon = mod360(self.lon)
        if (self.lat > 270):
            self.lat -= 360
        elif (self.lat > 90):
            self.lon += 180
            self.lat = 180-self.lat
        self.lon = mod360(self.lon)
        return self


class xyzPoint:
    # A point class {x, y, z} with the unit m

    def __init__(self, x: float = 0, y: float = 0, z: float = 0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def __str__(self) -> str:
        return f'x: {round(self.x/1000,4)}, y: {round(self.y/1000,4)} z: {round(self.z/1000,4)}'

    def unitVec(self) -> 'xyzPoint':
        # return the unit vector of self
        norm = math.sqrt(self.x*self.x+self.y*self.y+self.z*self.z)
        return xyzPoint(self.x/norm, self.y/norm, self.z/norm)

    def norm(self) -> float:
        return math.sqrt(self.x*self.x+self.y*self.y+self.z*self.z)
