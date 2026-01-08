#pragma once

#include "TimestampedTrajectoryPoint.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QString>

/// Builds OGC Moving Features Trajectory JSON documents
/// See: https://schemas.opengis.net/movingfeatures/1.0/MF-JSON_Trajectory.schema.json
class MovingFeaturesDocument
{
public:
    MovingFeaturesDocument();

    void setTrajectoryId(const QString& id) { _trajectoryId = id; }
    void setVehicleId(int id) { _vehicleId = id; }
    void setStartTime(const QDateTime& time) { _startTime = time; }
    void setEndTime(const QDateTime& time) { _endTime = time; }

    void addPoints(const QList<TimestampedTrajectoryPoint>& points);
    void addPoint(const TimestampedTrajectoryPoint& point);
    void clearPoints() { _points.clear(); }

    void setProperty(const QString& key, const QJsonValue& value);

    [[nodiscard]] QJsonObject toJsonObject() const;
    [[nodiscard]] QByteArray toJson() const;

    [[nodiscard]] bool isValid(QString* errorMessage = nullptr) const;
    [[nodiscard]] int pointCount() const { return static_cast<int>(_points.count()); }

private:
    static QJsonArray coordToJsonArray(const QGeoCoordinate& coord);
    static QString toIso8601(const QDateTime& dt);

    QString _trajectoryId;
    int _vehicleId = 0;
    QDateTime _startTime;
    QDateTime _endTime;
    QList<TimestampedTrajectoryPoint> _points;
    QJsonObject _customProperties;
};
