/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtPositioning/QGeoCoordinate>

// A bounding "cube" for small surface areas (doesn't take in consideration earth's curvature)
// Coordinate system makes NW Up Left Bottom (0,0,0) and SE Bottom Right Top (y,x,z)
class QGCGeoBoundingCube : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate pointNW MEMBER pointNW CONSTANT)
    Q_PROPERTY(QGeoCoordinate pointSE MEMBER pointSE CONSTANT)

public:
    explicit QGCGeoBoundingCube(QObject *parent = nullptr);

    QGCGeoBoundingCube(const QGCGeoBoundingCube &other)
        : QObject()
    {
        pointNW = other.pointNW;
        pointSE = other.pointSE;
    }

    QGCGeoBoundingCube(const QGeoCoordinate &p1, const QGeoCoordinate &p2)
        : pointNW(p1)
        , pointSE(p2)
    {
    }

    Q_INVOKABLE void reset();
    Q_INVOKABLE bool isValid() const;
    Q_INVOKABLE QGeoCoordinate center() const;

    inline bool operator==(const QGCGeoBoundingCube &other) const
    {
        return ((pointNW == other.pointNW) && (pointSE == other.pointSE));
    }

    bool operator==(const QList<QGeoCoordinate> &coords) const;

    inline bool operator!=(const QGCGeoBoundingCube& other)
    {
        return !(*this == other);
    }

    inline const QGCGeoBoundingCube &operator=(const QGCGeoBoundingCube &other)
    {
        pointNW = other.pointNW;
        pointSE = other.pointSE;
        return *this;
    }

    Q_INVOKABLE QList<QGeoCoordinate> polygon2D(double clipTo = 0.0) const;

    Q_INVOKABLE double width() const;
    Q_INVOKABLE double height() const;
    Q_INVOKABLE double area() const;
    Q_INVOKABLE double radius() const;

    QGeoCoordinate pointNW;
    QGeoCoordinate pointSE;
    static constexpr double MaxAlt = 1000000.0;
    static constexpr double MinAlt = -1000000.0;
    static constexpr double MaxNorth =  90.0;
    static constexpr double MaxSouth = -90.0;
    static constexpr double MaxWest = -180.0;
    static constexpr double MaxEast = 180.0;
};
