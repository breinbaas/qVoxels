#ifndef HELPERS_H
#define HELPERS_H

struct WGS84Coord {
    double latitude;
    double longitude;
};

WGS84Coord convertRDToWGS84(double x, double y);

#endif // HELPERS_H
