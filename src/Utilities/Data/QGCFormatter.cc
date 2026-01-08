#include "QGCFormatter.h"

#include <QtCore/QDateTime>

#include <cmath>

Q_LOGGING_CATEGORY(QGCFormatterLog, "Utilities.QGCFormatter")

QGCFormatter::QGCFormatter(QObject *parent)
    : QObject(parent)
{
}

QString QGCFormatter::formatDistance(double meters, DistanceFormat format, int precision)
{
    switch (format) {
    case DistanceFormat::Imperial:
        if (meters < 1609.344) {
            return QStringLiteral("%1 ft").arg(meters / 0.3048, 0, 'f', precision);
        }
        return QStringLiteral("%1 mi").arg(meters / 1609.344, 0, 'f', precision);

    case DistanceFormat::Nautical:
        return QStringLiteral("%1 nm").arg(meters / 1852.0, 0, 'f', precision);

    case DistanceFormat::Metric:
    default:
        if (meters < 1000.0) {
            return QStringLiteral("%1 m").arg(meters, 0, 'f', precision);
        }
        return QStringLiteral("%1 km").arg(meters / 1000.0, 0, 'f', precision);
    }
}

QString QGCFormatter::formatAltitude(double meters, DistanceFormat format, int precision)
{
    switch (format) {
    case DistanceFormat::Imperial:
    case DistanceFormat::Nautical:
        return QStringLiteral("%1 ft").arg(meters / 0.3048, 0, 'f', precision);

    case DistanceFormat::Metric:
    default:
        return QStringLiteral("%1 m").arg(meters, 0, 'f', precision);
    }
}

QString QGCFormatter::formatSpeed(double mps, DistanceFormat format, int precision)
{
    switch (format) {
    case DistanceFormat::Imperial:
        return QStringLiteral("%1 mph").arg(mps / 0.44704, 0, 'f', precision);

    case DistanceFormat::Nautical:
        return QStringLiteral("%1 kn").arg(mps / 0.514444, 0, 'f', precision);

    case DistanceFormat::Metric:
    default:
        return QStringLiteral("%1 km/h").arg(mps * 3.6, 0, 'f', precision);
    }
}

QString QGCFormatter::formatVerticalSpeed(double mps, int precision)
{
    return QStringLiteral("%1 m/s").arg(mps, 0, 'f', precision);
}

QString QGCFormatter::formatLatitude(double lat, CoordinateFormat format, int precision)
{
    const QChar dir = lat >= 0 ? 'N' : 'S';
    lat = std::abs(lat);

    switch (format) {
    case CoordinateFormat::DMS: {
        const int deg = static_cast<int>(lat);
        lat = (lat - deg) * 60.0;
        const int min = static_cast<int>(lat);
        const double sec = (lat - min) * 60.0;
        return QStringLiteral("%1°%2'%3\"%4").arg(deg).arg(min).arg(sec, 0, 'f', precision).arg(dir);
    }

    case CoordinateFormat::DMM: {
        const int deg = static_cast<int>(lat);
        const double min = (lat - deg) * 60.0;
        return QStringLiteral("%1°%2'%3").arg(deg).arg(min, 0, 'f', precision).arg(dir);
    }

    case CoordinateFormat::Decimal:
    default:
        return QStringLiteral("%1°%2").arg(lat, 0, 'f', precision).arg(dir);
    }
}

QString QGCFormatter::formatLongitude(double lon, CoordinateFormat format, int precision)
{
    const QChar dir = lon >= 0 ? 'E' : 'W';
    lon = std::abs(lon);

    switch (format) {
    case CoordinateFormat::DMS: {
        const int deg = static_cast<int>(lon);
        lon = (lon - deg) * 60.0;
        const int min = static_cast<int>(lon);
        const double sec = (lon - min) * 60.0;
        return QStringLiteral("%1°%2'%3\"%4").arg(deg).arg(min).arg(sec, 0, 'f', precision).arg(dir);
    }

    case CoordinateFormat::DMM: {
        const int deg = static_cast<int>(lon);
        const double min = (lon - deg) * 60.0;
        return QStringLiteral("%1°%2'%3").arg(deg).arg(min, 0, 'f', precision).arg(dir);
    }

    case CoordinateFormat::Decimal:
    default:
        return QStringLiteral("%1°%2").arg(lon, 0, 'f', precision).arg(dir);
    }
}

QString QGCFormatter::formatCoordinate(const QGeoCoordinate &coord, CoordinateFormat format)
{
    return QStringLiteral("%1, %2")
        .arg(formatLatitude(coord.latitude(), format))
        .arg(formatLongitude(coord.longitude(), format));
}

QString QGCFormatter::formatDuration(qint64 seconds)
{
    if (seconds < 0) {
        return QStringLiteral("-") + formatDuration(-seconds);
    }

    const int hours = seconds / 3600;
    const int mins = (seconds % 3600) / 60;
    const int secs = seconds % 60;

    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(mins, 2, 10, QLatin1Char('0'))
            .arg(secs, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2")
        .arg(mins)
        .arg(secs, 2, 10, QLatin1Char('0'));
}

QString QGCFormatter::formatDurationMs(qint64 milliseconds)
{
    return formatDuration(milliseconds / 1000);
}

QString QGCFormatter::formatTimeAgo(qint64 timestamp)
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 diff = (now - timestamp) / 1000;  // to seconds

    if (diff < 0) {
        return tr("in the future");
    }
    if (diff < 60) {
        return tr("just now");
    }
    if (diff < 3600) {
        const int mins = diff / 60;
        return tr("%n minute(s) ago", "", mins);
    }
    if (diff < 86400) {
        const int hours = diff / 3600;
        return tr("%n hour(s) ago", "", hours);
    }
    const int days = diff / 86400;
    return tr("%n day(s) ago", "", days);
}

QString QGCFormatter::formatDateTime(qint64 timestamp, const QString &format)
{
    return QDateTime::fromMSecsSinceEpoch(timestamp).toString(format);
}

QString QGCFormatter::formatNumber(double value, int precision)
{
    return QString::number(value, 'f', precision);
}

QString QGCFormatter::formatPercent(double value, int precision)
{
    return QStringLiteral("%1%").arg(value, 0, 'f', precision);
}

QString QGCFormatter::formatBytes(qint64 bytes, int precision)
{
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', precision);
    }
    if (bytes < 1024 * 1024 * 1024) {
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', precision);
    }
    return QStringLiteral("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', precision);
}

QString QGCFormatter::formatHeading(double degrees, bool includeCardinal)
{
    degrees = std::fmod(degrees, 360.0);
    if (degrees < 0) {
        degrees += 360.0;
    }

    QString result = QStringLiteral("%1°").arg(degrees, 0, 'f', 0);

    if (includeCardinal) {
        static const char *cardinals[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
        const int index = static_cast<int>((degrees + 22.5) / 45.0) % 8;
        result += QStringLiteral(" %1").arg(QLatin1String(cardinals[index]));
    }

    return result;
}

QString QGCFormatter::formatBearing(double degrees)
{
    return formatHeading(degrees, false);
}

QString QGCFormatter::formatTemperature(double celsius, bool useFahrenheit, int precision)
{
    if (useFahrenheit) {
        return QStringLiteral("%1°F").arg(celsius * 9.0 / 5.0 + 32.0, 0, 'f', precision);
    }
    return QStringLiteral("%1°C").arg(celsius, 0, 'f', precision);
}

QString QGCFormatter::formatVoltage(double volts, int precision)
{
    return QStringLiteral("%1 V").arg(volts, 0, 'f', precision);
}

QString QGCFormatter::formatBatteryPercent(double percent)
{
    return QStringLiteral("%1%").arg(percent, 0, 'f', 0);
}

QString QGCFormatter::formatSignalStrength(int percent)
{
    if (percent >= 80) {
        return QStringLiteral("Excellent (%1%)").arg(percent);
    }
    if (percent >= 60) {
        return QStringLiteral("Good (%1%)").arg(percent);
    }
    if (percent >= 40) {
        return QStringLiteral("Fair (%1%)").arg(percent);
    }
    if (percent >= 20) {
        return QStringLiteral("Poor (%1%)").arg(percent);
    }
    return QStringLiteral("Very Poor (%1%)").arg(percent);
}

QString QGCFormatter::formatRSSI(int dbm)
{
    return QStringLiteral("%1 dBm").arg(dbm);
}
