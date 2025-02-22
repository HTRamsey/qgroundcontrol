/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>

class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(TrajectoryPointsLog)

class TrajectoryPoints : public QObject
{
    Q_OBJECT

public:
    explicit TrajectoryPoints(Vehicle *vehicle, QObject *parent = nullptr);
    ~TrajectoryPoints();

    Q_INVOKABLE QVariantList list() const { return _points; }

    void start();
    void stop();

public slots:
    void clear();

signals:
    void pointAdded(QGeoCoordinate coordinate);
    void updateLastPoint(QGeoCoordinate coordinate);
    void pointsCleared();

private slots:
    void _vehicleCoordinateChanged(QGeoCoordinate coordinate);

private:
    Vehicle *_vehicle = nullptr;
    QVariantList _points;
    QGeoCoordinate _lastPoint;
    double _lastAzimuth = qQNaN();

    static constexpr double _distanceTolerance = 2.0;
    static constexpr double _azimuthTolerance = 1.5;
};
