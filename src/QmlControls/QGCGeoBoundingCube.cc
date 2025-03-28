/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCGeoBoundingCube.h"

QGCGeoBoundingCube::QGCGeoBoundingCube(QObject* parent)
    : QObject(parent)
{
    reset();
}

bool QGCGeoBoundingCube::isValid() const
{
    return (pointNW.isValid() && pointSE.isValid() && (pointNW.latitude() != MaxSouth) && (pointSE.latitude() != MaxNorth) && \
           (pointNW.longitude() != MaxEast) && (pointSE.longitude() != MaxWest) && (pointNW.altitude() < MaxAlt) && (pointSE.altitude() > MinAlt));
}

void QGCGeoBoundingCube::reset()
{
    pointNW = QGeoCoordinate(MaxSouth, MaxEast, MaxAlt);
    pointSE = QGeoCoordinate(MaxNorth, MaxWest, MinAlt);
}

QGeoCoordinate QGCGeoBoundingCube::center() const
{
    if (!isValid()) {
        return QGeoCoordinate();
    }

    const double lat = (((pointNW.latitude() + 90.0) + (pointSE.latitude() + 90.0)) / 2.0) - 90.0;
    const double lon = (((pointNW.longitude() + 180.0) + (pointSE.longitude() + 180.0)) / 2.0) - 180.0;
    const double alt = (pointNW.altitude() + pointSE.altitude()) / 2.0;
    return QGeoCoordinate(lat, lon, alt);
}

QList<QGeoCoordinate> QGCGeoBoundingCube::polygon2D(double clipTo) const
{
    QList<QGeoCoordinate> coords;
    if (!isValid()) {
        return coords;
    }

    // Should we clip it?
    if ((clipTo > 0.0) && (area() > clipTo)) {
        // Clip it to a square of given area centered on current bounding box center.
        const double side = sqrt(clipTo);
        const QGeoCoordinate c = center();
        const double a = qPow((side * 0.5), 2);
        const double h = qSqrt(a + a) * 1000.0;
        const QGeoCoordinate nw = c.atDistanceAndAzimuth(h, 315.0);
        const QGeoCoordinate se = c.atDistanceAndAzimuth(h, 135.0);
        coords.append(QGeoCoordinate(nw.latitude(), nw.longitude(), se.altitude()));
        coords.append(QGeoCoordinate(nw.latitude(), se.longitude(), se.altitude()));
        coords.append(QGeoCoordinate(se.latitude(), se.longitude(), se.altitude()));
        coords.append(QGeoCoordinate(se.latitude(), nw.longitude(), se.altitude()));
        coords.append(QGeoCoordinate(nw.latitude(), nw.longitude(), se.altitude()));
    } else {
        coords.append(QGeoCoordinate(pointNW.latitude(), pointNW.longitude(), pointSE.altitude()));
        coords.append(QGeoCoordinate(pointNW.latitude(), pointSE.longitude(), pointSE.altitude()));
        coords.append(QGeoCoordinate(pointSE.latitude(), pointSE.longitude(), pointSE.altitude()));
        coords.append(QGeoCoordinate(pointSE.latitude(), pointNW.longitude(), pointSE.altitude()));
        coords.append(QGeoCoordinate(pointNW.latitude(), pointNW.longitude(), pointSE.altitude()));
    }

    return coords;
}

bool QGCGeoBoundingCube::operator==(const QList<QGeoCoordinate> &coords) const
{
    QList<QGeoCoordinate> c = polygon2D();
    if (c.size() != coords.size()) {
        return false;
    }

    for (int i = 0; i < c.size(); i++) {
        if (c[i] != coords[i]) {
            return false;
        }
    }

    return true;
}

double QGCGeoBoundingCube::width() const
{
    if (!isValid()) {
        return 0.0;
    }

    const QGeoCoordinate ne = QGeoCoordinate(pointNW.latitude(), pointSE.longitude());
    return pointNW.distanceTo(ne);
}

double QGCGeoBoundingCube::height() const
{
    if (!isValid()) {
        return 0.0;
    }

    const QGeoCoordinate sw = QGeoCoordinate(pointSE.latitude(), pointNW.longitude());
    return pointNW.distanceTo(sw);
}

double QGCGeoBoundingCube::area() const
{
    if (!isValid()) {
        return 0.0;
    }

    // Area in km^2
    return (height() / 1000.0) * (width() / 1000.0);
}

double QGCGeoBoundingCube::radius() const
{
    if (!isValid()) {
        return 0.0;
    }

    return (pointNW.distanceTo(pointSE) / 2.0);
}
