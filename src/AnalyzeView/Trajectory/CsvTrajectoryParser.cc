#include "CsvTrajectoryParser.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>

QGC_LOGGING_CATEGORY(CsvTrajectoryParserLog, "AnalyzeView.CsvTrajectoryParser")

CsvTrajectoryParser::CsvTrajectoryParser() = default;

bool CsvTrajectoryParser::parseFile(const QString& filename, QString& errorMessage)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = QObject::tr("Cannot open file: %1").arg(filename);
        return false;
    }

    _points.clear();
    QTextStream stream(&file);

    // Parse header line
    QString headerLine = stream.readLine();
    if (headerLine.isEmpty()) {
        errorMessage = QObject::tr("Empty CSV file");
        return false;
    }

    QStringList headers = headerLine.split(',');
    if (!findColumnIndices(headers, errorMessage)) {
        return false;
    }

    // Extract vehicle ID from filename (format: "YYYY-MM-DD HH-MM-SS vehicleN.csv")
    static const QRegularExpression re(QStringLiteral("vehicle(\\d+)\\.csv$"));
    QRegularExpressionMatch match = re.match(filename);
    if (match.hasMatch()) {
        _vehicleId = match.captured(1).toInt();
    }

    // Parse data rows
    QGeoCoordinate lastCoord;
    int lineNumber = 1;

    while (!stream.atEnd()) {
        lineNumber++;
        QString line = stream.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        QStringList values = line.split(',');
        TimestampedTrajectoryPoint point = parseRow(values);

        if (!point.isValid()) {
            qCDebug(CsvTrajectoryParserLog) << "Invalid point at line" << lineNumber;
            continue;
        }

        // Distance filtering to reduce point density
        if (lastCoord.isValid() && lastCoord.distanceTo(point.coordinate) < _minDistance) {
            continue;
        }
        lastCoord = point.coordinate;

        _points.append(point);
    }

    if (_points.isEmpty()) {
        errorMessage = QObject::tr("No valid position data found in CSV file");
        return false;
    }

    qCDebug(CsvTrajectoryParserLog) << "Parsed" << _points.count() << "trajectory points from" << filename;
    return true;
}

bool CsvTrajectoryParser::findColumnIndices(const QStringList& headers, QString& errorMessage)
{
    _timestampCol = headers.indexOf(QStringLiteral("Timestamp"));

    // Try different column name patterns for GPS coordinates
    // QGC CSV format uses "gps.lat" and "gps.lon"
    _latitudeCol = headers.indexOf(QStringLiteral("gps.lat"));
    if (_latitudeCol < 0) {
        _latitudeCol = headers.indexOf(QStringLiteral("latitude"));
    }
    if (_latitudeCol < 0) {
        _latitudeCol = headers.indexOf(QStringLiteral("lat"));
    }

    _longitudeCol = headers.indexOf(QStringLiteral("gps.lon"));
    if (_longitudeCol < 0) {
        _longitudeCol = headers.indexOf(QStringLiteral("longitude"));
    }
    if (_longitudeCol < 0) {
        _longitudeCol = headers.indexOf(QStringLiteral("lon"));
    }

    // Altitude - try multiple possible column names
    _altitudeCol = headers.indexOf(QStringLiteral("altitudeAMSL"));
    if (_altitudeCol < 0) {
        _altitudeCol = headers.indexOf(QStringLiteral("gps.alt"));
    }
    if (_altitudeCol < 0) {
        _altitudeCol = headers.indexOf(QStringLiteral("altitude"));
    }

    // Optional columns
    _headingCol = headers.indexOf(QStringLiteral("heading"));
    _speedCol = headers.indexOf(QStringLiteral("groundSpeed"));

    // Validate required columns
    if (_timestampCol < 0) {
        errorMessage = QObject::tr("CSV file missing Timestamp column");
        return false;
    }
    if (_latitudeCol < 0) {
        errorMessage = QObject::tr("CSV file missing latitude column (gps.lat, latitude, or lat)");
        return false;
    }
    if (_longitudeCol < 0) {
        errorMessage = QObject::tr("CSV file missing longitude column (gps.lon, longitude, or lon)");
        return false;
    }

    qCDebug(CsvTrajectoryParserLog) << "Column indices - timestamp:" << _timestampCol
                                    << "lat:" << _latitudeCol
                                    << "lon:" << _longitudeCol
                                    << "alt:" << _altitudeCol
                                    << "heading:" << _headingCol
                                    << "speed:" << _speedCol;
    return true;
}

TimestampedTrajectoryPoint CsvTrajectoryParser::parseRow(const QStringList& values) const
{
    TimestampedTrajectoryPoint point;

    if (values.count() <= qMax(_timestampCol, qMax(_latitudeCol, _longitudeCol))) {
        return point; // Invalid - not enough columns
    }

    // Parse timestamp (format: "yyyy-MM-dd hh:mm:ss.zzz")
    QString timestampStr = values[_timestampCol];
    point.timestamp = QDateTime::fromString(timestampStr, QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"));
    if (!point.timestamp.isValid()) {
        // Try alternative format without milliseconds
        point.timestamp = QDateTime::fromString(timestampStr, QStringLiteral("yyyy-MM-dd hh:mm:ss"));
    }

    // Parse coordinates
    bool latOk = false;
    bool lonOk = false;
    double lat = values[_latitudeCol].toDouble(&latOk);
    double lon = values[_longitudeCol].toDouble(&lonOk);

    if (!latOk || !lonOk || lat == 0.0 || lon == 0.0) {
        return point; // Invalid coordinates
    }

    double alt = std::nan("");
    if (_altitudeCol >= 0 && _altitudeCol < values.count()) {
        bool altOk = false;
        alt = values[_altitudeCol].toDouble(&altOk);
        if (!altOk) {
            alt = std::nan("");
        }
    }

    point.coordinate = QGeoCoordinate(lat, lon, alt);

    // Parse optional fields
    if (_headingCol >= 0 && _headingCol < values.count()) {
        bool ok = false;
        double heading = values[_headingCol].toDouble(&ok);
        if (ok) {
            point.heading = heading;
        }
    }

    if (_speedCol >= 0 && _speedCol < values.count()) {
        bool ok = false;
        double speed = values[_speedCol].toDouble(&ok);
        if (ok) {
            point.speed = speed;
        }
    }

    return point;
}

QDateTime CsvTrajectoryParser::startTime() const
{
    if (_points.isEmpty()) {
        return {};
    }
    return _points.first().timestamp;
}

QDateTime CsvTrajectoryParser::endTime() const
{
    if (_points.isEmpty()) {
        return {};
    }
    return _points.last().timestamp;
}
