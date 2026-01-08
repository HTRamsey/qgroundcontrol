#pragma once

#include "TimestampedTrajectoryPoint.h"

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>
#include <QtQmlIntegration/QtQmlIntegration>

class Vehicle;

class TrajectoryPoints : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    TrajectoryPoints(Vehicle* vehicle, QObject* parent = nullptr);

    Q_INVOKABLE QVariantList list() const { return _points; }

    /// Returns timestamped trajectory points for export
    QList<TimestampedTrajectoryPoint> timestampedPoints() const { return _timestampedPoints; }

    /// Returns the number of trajectory points
    int count() const { return static_cast<int>(_points.count()); }

    void start();
    void stop();

public slots:
    void clear();

signals:
    void pointAdded(QGeoCoordinate coordinate);
    void updateLastPoint(QGeoCoordinate coordinate);
    void pointsCleared();
    void countChanged();

private slots:
    void _vehicleCoordinateChanged(QGeoCoordinate coordinate);

private:
    Vehicle*        _vehicle;
    QVariantList    _points;
    QList<TimestampedTrajectoryPoint> _timestampedPoints;
    QGeoCoordinate  _lastPoint;
    double          _lastAzimuth;

    static constexpr double _distanceTolerance = 2.0;
    static constexpr double _azimuthTolerance = 1.5;
};
