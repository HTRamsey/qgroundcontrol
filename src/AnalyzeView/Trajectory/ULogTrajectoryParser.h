#pragma once

#include "TimestampedTrajectoryPoint.h"

#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

Q_DECLARE_LOGGING_CATEGORY(ULogTrajectoryParserLog)

/// Parses ULog files for trajectory export
class ULogTrajectoryParser
{
public:
    ULogTrajectoryParser();

    bool parse(const QByteArray& logData, QString& errorMessage);
    bool parseFile(const QString& filename, QString& errorMessage);

    [[nodiscard]] QList<TimestampedTrajectoryPoint> trajectoryPoints() const { return _points; }

    [[nodiscard]] QDateTime startTime() const;
    [[nodiscard]] QDateTime endTime() const;

    void setMinDistance(double meters) { _minDistance = meters; }
    void setSubsampleRate(int rate) { _subsampleRate = qMax(1, rate); }

private:
    QList<TimestampedTrajectoryPoint> _points;
    double _minDistance = 1.0;
    int _subsampleRate = 1;
};
