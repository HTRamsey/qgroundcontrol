/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtPositioning/QGeoCoordinate>
#include <QtCore/QObject>
#include <QtCore/QVariantList>

class Vehicle;

class TrajectoryPoints : public QObject
{
    Q_OBJECT

public:
    explicit TrajectoryPoints(Vehicle *vehicle, QObject *parent = nullptr);

    Q_INVOKABLE QVariantList list() const { return _points; }

    void start();
    void stop();

signals:
    void pointAdded(const QGeoCoordinate &coordinate);
    void updateLastPoint(const QGeoCoordinate &coordinate);
    void pointsCleared();

public slots:
    void clear();

private slots:
    void _vehicleCoordinateChanged(const QGeoCoordinate &coordinate);

private:
    Vehicle *_vehicle = nullptr;
    QVariantList _points;
    QGeoCoordinate _lastPoint;
    double _lastAzimuth = qQNaN();
};
