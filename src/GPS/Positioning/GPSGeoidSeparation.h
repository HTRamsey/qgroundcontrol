#pragma once

#include <QtPositioning/QGeoCoordinate>

/// The height of mean sea level above the WGS84 ellipsoid where a receiver measured it: the difference between the
/// two heights that receiver reports there, as a vehicle's GPS does in GPS_RAW_INT.
struct GPSGeoidSeparation
{
    QGeoCoordinate measuredAt;
    double meters = 0.0;
};
