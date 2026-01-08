#include "QGCCoordinateParser.h"

#include <QtCore/QRegularExpression>

#include <cmath>

Q_LOGGING_CATEGORY(QGCCoordinateParserLog, "Utilities.QGCCoordinateParser")

QGCCoordinateParser::QGCCoordinateParser(QObject *parent)
    : QObject(parent)
{
}

QString QGCCoordinateParser::normalizeInput(const QString &text)
{
    QString normalized = text.trimmed();

    // Replace various separators with standard ones
    normalized.replace(QChar(0x00B0), QStringLiteral("°"));  // degree symbol variants
    normalized.replace(QChar(0x2032), QStringLiteral("'"));  // prime
    normalized.replace(QChar(0x2033), QStringLiteral("\""));  // double prime
    normalized.replace(QStringLiteral("''"), QStringLiteral("\""));

    return normalized;
}

QGCCoordinateParser::Format QGCCoordinateParser::detectFormat(const QString &text)
{
    const QString normalized = normalizeInput(text);

    // Check for DMS format: contains degree symbol and minutes/seconds
    static const QRegularExpression dmsPattern(
        QStringLiteral(R"(\d+°\s*\d+['′]\s*[\d.]+[\"″]?\s*[NSEW])"),
        QRegularExpression::CaseInsensitiveOption);
    if (dmsPattern.match(normalized).hasMatch()) {
        return Format::DegreesMinutesSeconds;
    }

    // Check for DMM format: degree symbol with decimal minutes, no seconds
    static const QRegularExpression dmmPattern(
        QStringLiteral(R"(\d+°\s*[\d.]+['′]\s*[NSEW])"),
        QRegularExpression::CaseInsensitiveOption);
    if (dmmPattern.match(normalized).hasMatch() && !dmsPattern.match(normalized).hasMatch()) {
        return Format::DegreesDecimalMinutes;
    }

    // Check for UTM format: zone + letter + easting + northing
    static const QRegularExpression utmPattern(
        QStringLiteral(R"(^\d{1,2}[C-X]\s+\d+\.?\d*\s+\d+\.?\d*$)"),
        QRegularExpression::CaseInsensitiveOption);
    if (utmPattern.match(normalized).hasMatch()) {
        return Format::UTM;
    }

    // Check for MGRS format: zone + grid letters + digits
    static const QRegularExpression mgrsPattern(
        QStringLiteral(R"(^\d{1,2}[C-X][A-HJ-NP-Z]{2}\d{2,10}$)"),
        QRegularExpression::CaseInsensitiveOption);
    if (mgrsPattern.match(normalized).hasMatch()) {
        return Format::MGRS;
    }

    // Check for decimal degrees: two numbers separated by comma or space
    static const QRegularExpression ddPattern(
        QStringLiteral(R"(^-?\d+\.?\d*[,\s]+-?\d+\.?\d*$)"));
    if (ddPattern.match(normalized).hasMatch()) {
        return Format::DecimalDegrees;
    }

    // Also check for decimal with direction letters
    static const QRegularExpression ddWithDirPattern(
        QStringLiteral(R"(\d+\.?\d*\s*[NS][,\s]+\d+\.?\d*\s*[EW])"),
        QRegularExpression::CaseInsensitiveOption);
    if (ddWithDirPattern.match(normalized).hasMatch()) {
        return Format::DecimalDegrees;
    }

    return Format::Unknown;
}

QGeoCoordinate QGCCoordinateParser::parse(const QString &text)
{
    const Format format = detectFormat(text);
    return parse(text, format);
}

QGeoCoordinate QGCCoordinateParser::parse(const QString &text, Format format)
{
    switch (format) {
    case Format::DecimalDegrees:
        return parseDecimalDegrees(text);
    case Format::DegreesMinutesSeconds:
        return parseDMS(text);
    case Format::DegreesDecimalMinutes:
        return parseDMM(text);
    case Format::UTM:
        return parseUTM(text);
    case Format::MGRS:
        qCWarning(QGCCoordinateParserLog) << "MGRS parsing not implemented (requires proj library)";
        return {};
    case Format::Unknown:
    default:
        qCWarning(QGCCoordinateParserLog) << "Unknown coordinate format:" << text;
        return {};
    }
}

QGeoCoordinate QGCCoordinateParser::parseDecimalDegrees(const QString &text)
{
    const QString normalized = normalizeInput(text);

    // Try pattern: lat, lon or lat lon (with optional direction letters)
    static const QRegularExpression pattern(
        QStringLiteral(R"((-?\d+\.?\d*)\s*([NS])?\s*[,\s]+\s*(-?\d+\.?\d*)\s*([EW])?)"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = pattern.match(normalized);
    if (!match.hasMatch()) {
        return {};
    }

    bool ok1 = false;
    bool ok2 = false;
    double lat = match.captured(1).toDouble(&ok1);
    double lon = match.captured(3).toDouble(&ok2);

    if (!ok1 || !ok2) {
        return {};
    }

    // Apply direction
    const QString latDir = match.captured(2).toUpper();
    const QString lonDir = match.captured(4).toUpper();

    if (latDir == QStringLiteral("S")) {
        lat = -std::abs(lat);
    } else if (latDir == QStringLiteral("N")) {
        lat = std::abs(lat);
    }

    if (lonDir == QStringLiteral("W")) {
        lon = -std::abs(lon);
    } else if (lonDir == QStringLiteral("E")) {
        lon = std::abs(lon);
    }

    // Validate ranges
    if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
        qCWarning(QGCCoordinateParserLog) << "Coordinate out of range:" << lat << lon;
        return {};
    }

    return QGeoCoordinate(lat, lon);
}

double QGCCoordinateParser::parseDMSComponent(const QString &text, bool &isNegative)
{
    isNegative = false;

    // Pattern: degrees°minutes'seconds"[NSEW]
    static const QRegularExpression pattern(
        QStringLiteral(R"((\d+)°\s*(\d+)['′]\s*([\d.]+)[\"″]?\s*([NSEW])?)"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = pattern.match(text);
    if (!match.hasMatch()) {
        return std::nan("");
    }

    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    const int degrees = match.captured(1).toInt(&ok1);
    const int minutes = match.captured(2).toInt(&ok2);
    const double seconds = match.captured(3).toDouble(&ok3);

    if (!ok1 || !ok2 || !ok3) {
        return std::nan("");
    }

    const QString direction = match.captured(4).toUpper();
    if (direction == QStringLiteral("S") || direction == QStringLiteral("W")) {
        isNegative = true;
    }

    double result = degrees + (minutes / 60.0) + (seconds / 3600.0);
    return isNegative ? -result : result;
}

QGeoCoordinate QGCCoordinateParser::parseDMS(const QString &text)
{
    const QString normalized = normalizeInput(text);

    // Split into latitude and longitude parts
    // Look for the boundary between lat and lon (typically after N/S before a digit)
    static const QRegularExpression splitPattern(
        QStringLiteral(R"(([^,]+[NS])\s*[,\s]+\s*([^,]+[EW]))"),
        QRegularExpression::CaseInsensitiveOption);

    auto match = splitPattern.match(normalized);
    if (!match.hasMatch()) {
        // Try without requiring direction letters
        static const QRegularExpression altPattern(
            QStringLiteral(R"((\d+°[^,]+)\s*[,\s]+\s*(\d+°.+))"));
        match = altPattern.match(normalized);
        if (!match.hasMatch()) {
            return {};
        }
    }

    bool latNeg = false;
    bool lonNeg = false;
    const double lat = parseDMSComponent(match.captured(1), latNeg);
    const double lon = parseDMSComponent(match.captured(2), lonNeg);

    if (std::isnan(lat) || std::isnan(lon)) {
        return {};
    }

    if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
        return {};
    }

    return QGeoCoordinate(lat, lon);
}

double QGCCoordinateParser::parseDMMComponent(const QString &text, bool &isNegative)
{
    isNegative = false;

    // Pattern: degrees°minutes.decimal'[NSEW]
    static const QRegularExpression pattern(
        QStringLiteral(R"((\d+)°\s*([\d.]+)['′]\s*([NSEW])?)"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = pattern.match(text);
    if (!match.hasMatch()) {
        return std::nan("");
    }

    bool ok1 = false;
    bool ok2 = false;
    const int degrees = match.captured(1).toInt(&ok1);
    const double minutes = match.captured(2).toDouble(&ok2);

    if (!ok1 || !ok2) {
        return std::nan("");
    }

    const QString direction = match.captured(3).toUpper();
    if (direction == QStringLiteral("S") || direction == QStringLiteral("W")) {
        isNegative = true;
    }

    double result = degrees + (minutes / 60.0);
    return isNegative ? -result : result;
}

QGeoCoordinate QGCCoordinateParser::parseDMM(const QString &text)
{
    const QString normalized = normalizeInput(text);

    // Split into latitude and longitude parts
    static const QRegularExpression splitPattern(
        QStringLiteral(R"(([^,]+[NS])\s*[,\s]+\s*([^,]+[EW]))"),
        QRegularExpression::CaseInsensitiveOption);

    auto match = splitPattern.match(normalized);
    if (!match.hasMatch()) {
        static const QRegularExpression altPattern(
            QStringLiteral(R"((\d+°[^,]+)\s*[,\s]+\s*(\d+°.+))"));
        match = altPattern.match(normalized);
        if (!match.hasMatch()) {
            return {};
        }
    }

    bool latNeg = false;
    bool lonNeg = false;
    const double lat = parseDMMComponent(match.captured(1), latNeg);
    const double lon = parseDMMComponent(match.captured(2), lonNeg);

    if (std::isnan(lat) || std::isnan(lon)) {
        return {};
    }

    if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
        return {};
    }

    return QGeoCoordinate(lat, lon);
}

QGeoCoordinate QGCCoordinateParser::parseUTM(const QString &text)
{
    const QString normalized = normalizeInput(text);

    // Pattern: zone letter easting northing
    static const QRegularExpression pattern(
        QStringLiteral(R"((\d{1,2})([C-X])\s+([\d.]+)\s+([\d.]+))"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = pattern.match(normalized);
    if (!match.hasMatch()) {
        return {};
    }

    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    const int zone = match.captured(1).toInt(&ok1);
    const QChar letter = match.captured(2).toUpper().at(0);
    const double easting = match.captured(3).toDouble(&ok2);
    const double northing = match.captured(4).toDouble(&ok3);

    if (!ok1 || !ok2 || !ok3 || zone < 1 || zone > 60) {
        return {};
    }

    // UTM to lat/lon conversion
    // Based on WGS84 ellipsoid parameters
    constexpr double a = 6378137.0;  // semi-major axis
    constexpr double f = 1.0 / 298.257223563;  // flattening
    constexpr double k0 = 0.9996;  // scale factor

    const double e = std::sqrt(f * (2.0 - f));  // eccentricity
    const double e2 = e * e;
    const double e1 = (1.0 - std::sqrt(1.0 - e2)) / (1.0 + std::sqrt(1.0 - e2));

    const bool northern = letter >= 'N';
    const double x = easting - 500000.0;
    double y = northing;
    if (!northern) {
        y -= 10000000.0;
    }

    const double lon0 = ((zone - 1) * 6.0 - 180.0 + 3.0) * M_PI / 180.0;

    const double M = y / k0;
    const double mu = M / (a * (1.0 - e2 / 4.0 - 3.0 * e2 * e2 / 64.0 - 5.0 * e2 * e2 * e2 / 256.0));

    const double phi1 = mu + (3.0 * e1 / 2.0 - 27.0 * e1 * e1 * e1 / 32.0) * std::sin(2.0 * mu)
                        + (21.0 * e1 * e1 / 16.0 - 55.0 * e1 * e1 * e1 * e1 / 32.0) * std::sin(4.0 * mu)
                        + (151.0 * e1 * e1 * e1 / 96.0) * std::sin(6.0 * mu);

    const double N1 = a / std::sqrt(1.0 - e2 * std::sin(phi1) * std::sin(phi1));
    const double T1 = std::tan(phi1) * std::tan(phi1);
    const double C1 = e2 * std::cos(phi1) * std::cos(phi1) / (1.0 - e2);
    const double R1 = a * (1.0 - e2) / std::pow(1.0 - e2 * std::sin(phi1) * std::sin(phi1), 1.5);
    const double D = x / (N1 * k0);

    double lat = phi1 - (N1 * std::tan(phi1) / R1) *
                 (D * D / 2.0 - (5.0 + 3.0 * T1 + 10.0 * C1 - 4.0 * C1 * C1 - 9.0 * e2) * D * D * D * D / 24.0
                  + (61.0 + 90.0 * T1 + 298.0 * C1 + 45.0 * T1 * T1 - 252.0 * e2 - 3.0 * C1 * C1)
                    * D * D * D * D * D * D / 720.0);

    double lon = lon0 + (D - (1.0 + 2.0 * T1 + C1) * D * D * D / 6.0
                         + (5.0 - 2.0 * C1 + 28.0 * T1 - 3.0 * C1 * C1 + 8.0 * e2 + 24.0 * T1 * T1)
                           * D * D * D * D * D / 120.0) / std::cos(phi1);

    lat = lat * 180.0 / M_PI;
    lon = lon * 180.0 / M_PI;

    if (lat < -90.0 || lat > 90.0 || lon < -180.0 || lon > 180.0) {
        return {};
    }

    return QGeoCoordinate(lat, lon);
}

bool QGCCoordinateParser::isValidCoordinateString(const QString &text)
{
    return detectFormat(text) != Format::Unknown;
}

QString QGCCoordinateParser::toString(const QGeoCoordinate &coord, Format format, int precision)
{
    if (!coord.isValid()) {
        return {};
    }

    switch (format) {
    case Format::DecimalDegrees:
        return toDecimalDegrees(coord, precision);
    case Format::DegreesMinutesSeconds:
        return toDMS(coord, precision);
    case Format::DegreesDecimalMinutes:
        return toDMM(coord, precision);
    default:
        return toDecimalDegrees(coord, precision);
    }
}

QString QGCCoordinateParser::toDecimalDegrees(const QGeoCoordinate &coord, int precision)
{
    if (!coord.isValid()) {
        return {};
    }

    return QStringLiteral("%1, %2")
        .arg(coord.latitude(), 0, 'f', precision)
        .arg(coord.longitude(), 0, 'f', precision);
}

QString QGCCoordinateParser::toDMS(const QGeoCoordinate &coord, int precision)
{
    if (!coord.isValid()) {
        return {};
    }

    auto toDMSPart = [precision](double value, bool isLat) -> QString {
        const QChar dir = isLat ? (value >= 0 ? 'N' : 'S') : (value >= 0 ? 'E' : 'W');
        value = std::abs(value);
        const int deg = static_cast<int>(value);
        value = (value - deg) * 60.0;
        const int min = static_cast<int>(value);
        const double sec = (value - min) * 60.0;

        return QStringLiteral("%1°%2'%3\"%4")
            .arg(deg)
            .arg(min)
            .arg(sec, 0, 'f', precision)
            .arg(dir);
    };

    return QStringLiteral("%1 %2")
        .arg(toDMSPart(coord.latitude(), true))
        .arg(toDMSPart(coord.longitude(), false));
}

QString QGCCoordinateParser::toDMM(const QGeoCoordinate &coord, int precision)
{
    if (!coord.isValid()) {
        return {};
    }

    auto toDMMPart = [precision](double value, bool isLat) -> QString {
        const QChar dir = isLat ? (value >= 0 ? 'N' : 'S') : (value >= 0 ? 'E' : 'W');
        value = std::abs(value);
        const int deg = static_cast<int>(value);
        const double min = (value - deg) * 60.0;

        return QStringLiteral("%1°%2'%3")
            .arg(deg)
            .arg(min, 0, 'f', precision)
            .arg(dir);
    };

    return QStringLiteral("%1 %2")
        .arg(toDMMPart(coord.latitude(), true))
        .arg(toDMMPart(coord.longitude(), false));
}

QString QGCCoordinateParser::formatName(Format format)
{
    switch (format) {
    case Format::DecimalDegrees:
        return tr("Decimal Degrees");
    case Format::DegreesMinutesSeconds:
        return tr("Degrees Minutes Seconds");
    case Format::DegreesDecimalMinutes:
        return tr("Degrees Decimal Minutes");
    case Format::UTM:
        return tr("UTM");
    case Format::MGRS:
        return tr("MGRS");
    case Format::Unknown:
    default:
        return tr("Unknown");
    }
}
