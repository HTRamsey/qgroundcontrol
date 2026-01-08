#pragma once

#include "TimestampedTrajectoryPoint.h"

#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

Q_DECLARE_LOGGING_CATEGORY(CsvTrajectoryParserLog)

/// Parses QGroundControl CSV telemetry logs for trajectory export
class CsvTrajectoryParser
{
public:
    CsvTrajectoryParser();

    bool parseFile(const QString& filename, QString& errorMessage);

    [[nodiscard]] QList<TimestampedTrajectoryPoint> trajectoryPoints() const { return _points; }

    [[nodiscard]] int vehicleId() const { return _vehicleId; }
    [[nodiscard]] QDateTime startTime() const;
    [[nodiscard]] QDateTime endTime() const;

    void setMinDistance(double meters) { _minDistance = meters; }

private:
    bool findColumnIndices(const QStringList& headers, QString& errorMessage);
    TimestampedTrajectoryPoint parseRow(const QStringList& values) const;

    QList<TimestampedTrajectoryPoint> _points;
    int _vehicleId = 0;
    double _minDistance = 1.0;

    // Column indices
    int _timestampCol = -1;
    int _latitudeCol = -1;
    int _longitudeCol = -1;
    int _altitudeCol = -1;
    int _headingCol = -1;
    int _speedCol = -1;
};
