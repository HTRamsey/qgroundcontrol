#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QMetaType>
#include <QtPositioning/QGeoCoordinate>

#include <cmath>

/// A trajectory point with timestamp for OGC Moving Features export
struct TimestampedTrajectoryPoint
{
    QGeoCoordinate coordinate;
    QDateTime timestamp;

    // Optional extended properties
    double heading = std::nan("");
    double speed = std::nan("");
    double altitudeAGL = std::nan("");

    bool isValid() const { return coordinate.isValid() && timestamp.isValid(); }
};

Q_DECLARE_METATYPE(TimestampedTrajectoryPoint)
