#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(QGCFormatterLog)

/// Consistent formatting utilities for display values.
///
/// Provides locale-aware formatting for distances, speeds, coordinates, etc.
class QGCFormatter : public QObject
{
    Q_OBJECT

public:
    enum class DistanceFormat { Metric, Imperial, Nautical };
    Q_ENUM(DistanceFormat)

    enum class CoordinateFormat { Decimal, DMS, DMM };
    Q_ENUM(CoordinateFormat)

    explicit QGCFormatter(QObject *parent = nullptr);
    ~QGCFormatter() override = default;

    // Distance formatting
    static QString formatDistance(double meters, DistanceFormat format = DistanceFormat::Metric, int precision = 1);
    static QString formatAltitude(double meters, DistanceFormat format = DistanceFormat::Metric, int precision = 0);

    // Speed formatting
    static QString formatSpeed(double mps, DistanceFormat format = DistanceFormat::Metric, int precision = 1);
    static QString formatVerticalSpeed(double mps, int precision = 1);

    // Coordinate formatting
    static QString formatLatitude(double lat, CoordinateFormat format = CoordinateFormat::Decimal, int precision = 6);
    static QString formatLongitude(double lon, CoordinateFormat format = CoordinateFormat::Decimal, int precision = 6);
    static QString formatCoordinate(const QGeoCoordinate &coord, CoordinateFormat format = CoordinateFormat::Decimal);

    // Time formatting
    static QString formatDuration(qint64 seconds);
    static QString formatDurationMs(qint64 milliseconds);
    static QString formatTimeAgo(qint64 timestamp);
    static QString formatDateTime(qint64 timestamp, const QString &format = QStringLiteral("yyyy-MM-dd HH:mm:ss"));

    // Number formatting
    static QString formatNumber(double value, int precision = 2);
    static QString formatPercent(double value, int precision = 0);
    static QString formatBytes(qint64 bytes, int precision = 1);

    // Heading/bearing formatting
    static QString formatHeading(double degrees, bool includeCardinal = true);
    static QString formatBearing(double degrees);

    // Temperature
    static QString formatTemperature(double celsius, bool useFahrenheit = false, int precision = 1);

    // Battery/voltage
    static QString formatVoltage(double volts, int precision = 2);
    static QString formatBatteryPercent(double percent);

    // Signal strength
    static QString formatSignalStrength(int percent);
    static QString formatRSSI(int dbm);
};
