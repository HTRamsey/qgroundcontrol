#include "MovingFeaturesDocument.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>

MovingFeaturesDocument::MovingFeaturesDocument() = default;

void MovingFeaturesDocument::addPoints(const QList<TimestampedTrajectoryPoint>& points)
{
    _points.append(points);
}

void MovingFeaturesDocument::addPoint(const TimestampedTrajectoryPoint& point)
{
    _points.append(point);
}

void MovingFeaturesDocument::setProperty(const QString& key, const QJsonValue& value)
{
    _customProperties[key] = value;
}

QJsonObject MovingFeaturesDocument::toJsonObject() const
{
    QJsonArray coordinates;
    QJsonArray datetimes;

    for (const auto& point : _points) {
        coordinates.append(coordToJsonArray(point.coordinate));
        datetimes.append(toIso8601(point.timestamp));
    }

    // Geometry (OGC MF-JSON uses LineString for trajectory)
    QJsonObject geometry;
    geometry[QStringLiteral("type")] = QStringLiteral("LineString");
    geometry[QStringLiteral("coordinates")] = coordinates;

    // Properties - datetimes is required by OGC MF-JSON spec
    QJsonObject properties;
    properties[QStringLiteral("datetimes")] = datetimes;

    // Add metadata
    if (!_trajectoryId.isEmpty()) {
        properties[QStringLiteral("trajectoryId")] = _trajectoryId;
    }
    if (_vehicleId > 0) {
        properties[QStringLiteral("vehicleId")] = _vehicleId;
    }
    if (_startTime.isValid()) {
        properties[QStringLiteral("startTime")] = toIso8601(_startTime);
    }
    if (_endTime.isValid()) {
        properties[QStringLiteral("endTime")] = toIso8601(_endTime);
    }

    // Generator info
    properties[QStringLiteral("generator")] = QCoreApplication::applicationName();
    properties[QStringLiteral("generatorVersion")] = QCoreApplication::applicationVersion();

    // Merge custom properties
    for (auto it = _customProperties.begin(); it != _customProperties.end(); ++it) {
        properties[it.key()] = it.value();
    }

    // Build Feature object
    QJsonObject feature;
    feature[QStringLiteral("type")] = QStringLiteral("Feature");
    feature[QStringLiteral("geometry")] = geometry;
    feature[QStringLiteral("properties")] = properties;

    return feature;
}

QByteArray MovingFeaturesDocument::toJson() const
{
    QJsonDocument doc(toJsonObject());
    return doc.toJson(QJsonDocument::Indented);
}

bool MovingFeaturesDocument::isValid(QString* errorMessage) const
{
    // OGC MF-JSON requires at least 2 points for LineString
    if (_points.count() < 2) {
        if (errorMessage) {
            *errorMessage = QObject::tr("Trajectory requires at least 2 points");
        }
        return false;
    }

    // Verify all points have valid timestamps
    for (const auto& point : _points) {
        if (!point.isValid()) {
            if (errorMessage) {
                *errorMessage = QObject::tr("Trajectory contains invalid points");
            }
            return false;
        }
    }

    // Verify datetimes are unique (OGC spec requirement)
    QSet<qint64> timestamps;
    for (const auto& point : _points) {
        qint64 msecs = point.timestamp.toMSecsSinceEpoch();
        if (timestamps.contains(msecs)) {
            if (errorMessage) {
                *errorMessage = QObject::tr("Trajectory contains duplicate timestamps");
            }
            return false;
        }
        timestamps.insert(msecs);
    }

    return true;
}

QJsonArray MovingFeaturesDocument::coordToJsonArray(const QGeoCoordinate& coord)
{
    QJsonArray arr;
    // GeoJSON order: [longitude, latitude, altitude]
    arr.append(coord.longitude());
    arr.append(coord.latitude());
    if (!qIsNaN(coord.altitude())) {
        arr.append(coord.altitude());
    }
    return arr;
}

QString MovingFeaturesDocument::toIso8601(const QDateTime& dt)
{
    return dt.toUTC().toString(Qt::ISODateWithMs);
}
