#include "CsvPlanImporter.h"
#include "PlanImporter.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <cmath>

CsvPlanImporter::CsvPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(CsvPlanImporter)

bool CsvPlanImporter::_parseLine(const QString& line, QChar delimiter, QStringList& fields)
{
    fields.clear();
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.length(); i++) {
        QChar c = line[i];

        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < line.length() && line[i + 1] == '"') {
                    current += '"';
                    i++;
                } else {
                    inQuotes = false;
                }
            } else {
                current += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == delimiter) {
                fields.append(current.trimmed());
                current.clear();
            } else {
                current += c;
            }
        }
    }

    fields.append(current.trimmed());
    return true;
}

int CsvPlanImporter::_findColumn(const QStringList& headers, const QStringList& possibleNames)
{
    for (int i = 0; i < headers.count(); i++) {
        QString header = headers[i].toLower().trimmed();
        for (const QString& name : possibleNames) {
            if (header == name.toLower() || header.contains(name.toLower())) {
                return i;
            }
        }
    }
    return -1;
}

PlanImportResult CsvPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    result.formatDescription = formatName();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorString = tr("Cannot open file: %1").arg(file.errorString());
        return result;
    }

    QTextStream stream(&file);
    QChar delimiter = ',';

    // Read first line to detect delimiter and headers
    QString firstLine = stream.readLine();
    if (firstLine.isEmpty()) {
        result.errorString = tr("File is empty");
        return result;
    }

    // Detect delimiter (try comma, tab, semicolon)
    if (firstLine.count('\t') > firstLine.count(',')) {
        delimiter = '\t';
    } else if (firstLine.count(';') > firstLine.count(',')) {
        delimiter = ';';
    }

    QStringList headers;
    _parseLine(firstLine, delimiter, headers);

    // Try to find coordinate columns
    QStringList latNames = {QStringLiteral("latitude"), QStringLiteral("lat"), QStringLiteral("y")};
    QStringList lonNames = {QStringLiteral("longitude"), QStringLiteral("lon"), QStringLiteral("lng"), QStringLiteral("long"), QStringLiteral("x")};
    QStringList altNames = {QStringLiteral("altitude"), QStringLiteral("alt"), QStringLiteral("elevation"), QStringLiteral("ele"), QStringLiteral("z")};
    QStringList nameNames = {QStringLiteral("name"), QStringLiteral("label"), QStringLiteral("waypoint"), QStringLiteral("id")};

    int latCol = _findColumn(headers, latNames);
    int lonCol = _findColumn(headers, lonNames);
    int altCol = _findColumn(headers, altNames);
    int nameCol = _findColumn(headers, nameNames);

    // Check if first row is a header (non-numeric latitude)
    bool hasHeader = false;
    if (latCol >= 0 && latCol < headers.count()) {
        bool ok;
        headers[latCol].toDouble(&ok);
        hasHeader = !ok;
    } else {
        // No recognizable headers - assume standard order: name, lat, lon, alt
        if (headers.count() >= 2) {
            bool ok;
            headers[0].toDouble(&ok);
            if (!ok && headers.count() >= 3) {
                // First column is non-numeric, assume: name, lat, lon, [alt]
                nameCol = 0;
                latCol = 1;
                lonCol = 2;
                altCol = headers.count() >= 4 ? 3 : -1;
            } else {
                // First column is numeric, assume: lat, lon, [alt]
                latCol = 0;
                lonCol = 1;
                altCol = headers.count() >= 3 ? 2 : -1;
            }
        }
    }

    if (latCol < 0 || lonCol < 0) {
        result.errorString = tr("Cannot find latitude/longitude columns. Expected columns named 'Latitude', 'Longitude' or similar.");
        return result;
    }

    // If no header, process the first line as data
    int lineNum = 1;
    if (!hasHeader) {
        stream.seek(0);
        lineNum = 0;
    }

    while (!stream.atEnd()) {
        QString line = stream.readLine();
        lineNum++;

        if (line.trimmed().isEmpty() || line.startsWith('#')) {
            continue;
        }

        QStringList fields;
        _parseLine(line, delimiter, fields);

        if (latCol >= fields.count() || lonCol >= fields.count()) {
            continue;
        }

        bool latOk, lonOk;
        double lat = fields[latCol].toDouble(&latOk);
        double lon = fields[lonCol].toDouble(&lonOk);

        if (!latOk || !lonOk) {
            continue;
        }

        double alt = 0.0;
        if (altCol >= 0 && altCol < fields.count()) {
            bool altOk;
            alt = fields[altCol].toDouble(&altOk);
            if (!altOk) {
                alt = std::nan("");
            }
        } else {
            alt = std::nan("");
        }

        QGeoCoordinate coord(lat, lon);
        if (!std::isnan(alt)) {
            coord.setAltitude(alt);
        }

        result.waypoints.append(coord);

        if (nameCol >= 0 && nameCol < fields.count()) {
            result.waypointNames.append(fields[nameCol]);
        } else {
            result.waypointNames.append(QStringLiteral("WPT%1").arg(result.waypoints.count(), 3, 10, QLatin1Char('0')));
        }
    }

    if (result.waypoints.isEmpty()) {
        result.errorString = tr("No valid coordinates found in file");
        return result;
    }

    result.success = true;
    return result;
}
