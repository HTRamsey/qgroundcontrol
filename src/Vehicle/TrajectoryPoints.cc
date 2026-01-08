#include "TrajectoryPoints.h"
#include "Vehicle.h"
#include "FactGroup.h"

#include <QtCore/QDateTime>

TrajectoryPoints::TrajectoryPoints(Vehicle* vehicle, QObject* parent)
    : QObject       (parent)
    , _vehicle      (vehicle)
    , _lastAzimuth  (qQNaN())
{
}

void TrajectoryPoints::_vehicleCoordinateChanged(QGeoCoordinate coordinate)
{
    // The goal of this algorithm is to limit the number of trajectory points which represent the vehicle path.
    // Fewer points means higher performance of map display.

    const QDateTime now = QDateTime::currentDateTimeUtc();

    if (_lastPoint.isValid()) {
        double distance = _lastPoint.distanceTo(coordinate);
        if (distance > _distanceTolerance) {
            //-- Update flight distance
            _vehicle->updateFlightDistance(distance);
            // Vehicle has moved far enough from previous point for an update
            double newAzimuth = _lastPoint.azimuthTo(coordinate);
            if (qIsNaN(_lastAzimuth) || qAbs(newAzimuth - _lastAzimuth) > _azimuthTolerance) {
                // The new position IS NOT colinear with the last segment. Append the new position to the list.
                _lastAzimuth = _lastPoint.azimuthTo(coordinate);
                _lastPoint = coordinate;
                _points.append(QVariant::fromValue(coordinate));

                // Store timestamped point for export
                TimestampedTrajectoryPoint tp;
                tp.coordinate = coordinate;
                tp.timestamp = now;
                tp.heading = _vehicle->heading()->rawValue().toDouble();
                tp.speed = _vehicle->groundSpeed()->rawValue().toDouble();
                _timestampedPoints.append(tp);

                emit pointAdded(coordinate);
                emit countChanged();
            } else {
                // The new position IS colinear with the last segment. Don't add a new point, just update
                // the last point to be the new position.
                _lastPoint = coordinate;
                _points[_points.count() - 1] = QVariant::fromValue(coordinate);

                // Update last timestamped point
                if (!_timestampedPoints.isEmpty()) {
                    _timestampedPoints.last().coordinate = coordinate;
                    _timestampedPoints.last().timestamp = now;
                }

                emit updateLastPoint(coordinate);
            }
        }
    } else {
        // Add the very first trajectory point to the list
        _lastPoint = coordinate;
        _points.append(QVariant::fromValue(coordinate));

        // Store timestamped point for export
        TimestampedTrajectoryPoint tp;
        tp.coordinate = coordinate;
        tp.timestamp = now;
        tp.heading = _vehicle->heading()->rawValue().toDouble();
        tp.speed = _vehicle->groundSpeed()->rawValue().toDouble();
        _timestampedPoints.append(tp);

        emit pointAdded(coordinate);
        emit countChanged();
    }
}

void TrajectoryPoints::start()
{
    clear();
    connect(_vehicle, &Vehicle::coordinateChanged, this, &TrajectoryPoints::_vehicleCoordinateChanged);
}

void TrajectoryPoints::stop()
{
    disconnect(_vehicle, &Vehicle::coordinateChanged, this, &TrajectoryPoints::_vehicleCoordinateChanged);
}

void TrajectoryPoints::clear()
{
    _points.clear();
    _timestampedPoints.clear();
    _lastPoint = QGeoCoordinate();
    _lastAzimuth = qQNaN();
    emit pointsCleared();
    emit countChanged();
}
