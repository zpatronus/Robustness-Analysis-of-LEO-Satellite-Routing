#ifndef C9035189_B465_4715_83C2_BECCD2423A84
#define C9035189_B465_4715_83C2_BECCD2423A84
#include <cmath>
#include <string>
using namespace std;
class Point {
   private:
    void copyFrom(const Point& p) {
        alt = p.alt;
        lat = p.lat;
        lon = p.lon;
    }
    double mod360(double x) { return fmod(fmod(x, 360) + 360, 360); }

   public:
    double alt, lon, lat;
    Point() : alt(0), lon(0), lat(0) {}
    Point(double alt, double lon, double lat) : alt(alt), lon(lon), lat(lat) {}

    Point(const Point& p) { copyFrom(p); }
    Point& operator=(const Point& p) {
        copyFrom(p);
        return *this;
    }
    string str() { return "Lon: " + to_string(lon - 180) + ", Lat: " + to_string(lat); }
    Point check() {
        lat = mod360(lat);
        lon = mod360(lon);
        if (lat > 270) {
            lat -= 360;
        } else if (lat > 90) {
            lon += 180;
            lat = 180 - lat;
        }
        lon = mod360(lon);
        return *this;
    }
};

class xyzPoint {
   private:
    void copyFrom(const xyzPoint& p) {
        x = p.x;
        y = p.y;
        z = p.z;
    }

   public:
    double x, y, z;
    xyzPoint() : x(0), y(0), z(0) {}
    xyzPoint(double x, double y, double z) : x(x), y(y), z(z) {}
    xyzPoint(const xyzPoint& p) { copyFrom(p); }
    xyzPoint& operator=(const xyzPoint& p) {
        copyFrom(p);
        return *this;
    }
    string str() { return "x: " + to_string(x) + ", y: " + to_string(y) + ", z: " + to_string(z); }
    double norm() const { return sqrt(x * x + y * y + z * z); }
    xyzPoint unitVec() {
        double nm = norm();
        return xyzPoint(x / nm, y / nm, z / nm);
    }
};

#endif /* C9035189_B465_4715_83C2_BECCD2423A84 */
