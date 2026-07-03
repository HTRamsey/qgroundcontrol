#pragma once
#include <QtCore/QObject>
#include <QtQmlIntegration/QtQmlIntegration>

/// Pure formatting helpers for GPS/GNSS display values.
/// Exposed as a QML singleton — use GPSFormatter.formatXxx() in QML.
class GPSFormatter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit GPSFormatter(QObject *parent = nullptr) : QObject(parent) {}

    // All helpers are pure stateless functions — exposed via Q_INVOKABLE so that
    // QML (through the QML_SINGLETON instance) and C++ tests (via static dispatch)
    // share one implementation. Qt 6.5+ supports Q_INVOKABLE static methods on
    // QML_SINGLETON types; we intentionally rely on that here rather than splitting
    // the API into instance-only or free-function variants. This is the single
    // supported calling convention.
    Q_INVOKABLE static QString formatDuration(double secs);
    Q_INVOKABLE static QString formatDataSize(double bytes);
    Q_INVOKABLE static QString formatDataRate(double bytesPerSec);
    Q_INVOKABLE static QString formatLatitude(double lat, int precision = 7, bool withHemisphere = false);
    Q_INVOKABLE static QString formatLongitude(double lon, int precision = 7, bool withHemisphere = false);
    Q_INVOKABLE static QString formatCoordinate(double lat, double lon, int precision = 7, bool withHemisphere = false);
    Q_INVOKABLE static QString formatHeading(double degrees);

    static constexpr int defaultCoordPrecision = 7;
};
