#include "GPXHelper.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>

QGC_LOGGING_CATEGORY(GPXHelperLog, "qgc.utilities.geo.gpxhelper")

namespace {
    constexpr const char *_errorPrefix = QT_TR_NOOP("GPX file load failed. %1");
    constexpr const char *_saveErrorPrefix = QT_TR_NOOP("GPX file save failed. %1");
    constexpr double _polygonClosureThresholdMeters = 1.0;

    constexpr const char *_gpxNamespace = "http://www.topografix.com/GPX/1/1";
    constexpr const char *_gpxSchemaLocation = "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd";

    bool openFileForRead(const QString &filePath, QFile &file, QString &errorString)
    {
        file.setFileName(filePath);
        if (!file.exists()) {
            errorString = QString(_errorPrefix).arg(QObject::tr("File not found: %1").arg(filePath));
            return false;
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            errorString = QString(_errorPrefix).arg(QObject::tr("Unable to open file: %1").arg(file.errorString()));
            return false;
        }
        return true;
    }

    bool openFileForWrite(const QString &filePath, QFile &file, QString &errorString)
    {
        file.setFileName(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            errorString = QString(_saveErrorPrefix).arg(QObject::tr("Unable to open file for writing: %1").arg(file.errorString()));
            return false;
        }
        return true;
    }

    QGeoCoordinate parsePoint(QXmlStreamReader &xml)
    {
        const QXmlStreamAttributes attrs = xml.attributes();
        bool latOk = false, lonOk = false;
        const double lat = attrs.value(QStringLiteral("lat")).toDouble(&latOk);
        const double lon = attrs.value(QStringLiteral("lon")).toDouble(&lonOk);

        if (!latOk || !lonOk) {
            return QGeoCoordinate();
        }

        double altitude = qQNaN();

        while (xml.readNextStartElement()) {
            if (xml.name() == QStringLiteral("ele")) {
                bool ok = false;
                const double ele = xml.readElementText().toDouble(&ok);
                if (ok) {
                    altitude = ele;
                }
            } else {
                xml.skipCurrentElement();
            }
        }

        if (std::isnan(altitude)) {
            return QGeoCoordinate(lat, lon);
        }
        return QGeoCoordinate(lat, lon, altitude);
    }

    QList<QGeoCoordinate> parseRoute(QXmlStreamReader &xml)
    {
        QList<QGeoCoordinate> coords;

        while (xml.readNextStartElement()) {
            if (xml.name() == QStringLiteral("rtept")) {
                const QGeoCoordinate coord = parsePoint(xml);
                if (coord.isValid()) {
                    coords.append(coord);
                }
            } else {
                xml.skipCurrentElement();
            }
        }

        return coords;
    }

    QList<QGeoCoordinate> parseTrackSegment(QXmlStreamReader &xml)
    {
        QList<QGeoCoordinate> coords;

        while (xml.readNextStartElement()) {
            if (xml.name() == QStringLiteral("trkpt")) {
                const QGeoCoordinate coord = parsePoint(xml);
                if (coord.isValid()) {
                    coords.append(coord);
                }
            } else {
                xml.skipCurrentElement();
            }
        }

        return coords;
    }

    QList<QList<QGeoCoordinate>> parseTrack(QXmlStreamReader &xml)
    {
        QList<QList<QGeoCoordinate>> segments;

        while (xml.readNextStartElement()) {
            if (xml.name() == QStringLiteral("trkseg")) {
                const QList<QGeoCoordinate> segment = parseTrackSegment(xml);
                if (!segment.isEmpty()) {
                    segments.append(segment);
                }
            } else {
                xml.skipCurrentElement();
            }
        }

        return segments;
    }

    bool isClosed(const QList<QGeoCoordinate> &coords)
    {
        if (coords.count() < 3) {
            return false;
        }
        return coords.first().distanceTo(coords.last()) < _polygonClosureThresholdMeters;
    }

    void writeGpxHeader(QXmlStreamWriter &xml, const QString &creator = QStringLiteral("QGroundControl"))
    {
        xml.setAutoFormatting(true);
        xml.setAutoFormattingIndent(2);
        xml.writeStartDocument();
        xml.writeStartElement(QStringLiteral("gpx"));
        xml.writeAttribute(QStringLiteral("version"), QStringLiteral("1.1"));
        xml.writeAttribute(QStringLiteral("creator"), creator);
        xml.writeDefaultNamespace(QString::fromLatin1(_gpxNamespace));
        xml.writeAttribute(QStringLiteral("xmlns:xsi"), QStringLiteral("http://www.w3.org/2001/XMLSchema-instance"));
        xml.writeAttribute(QStringLiteral("xsi:schemaLocation"), QString::fromLatin1(_gpxSchemaLocation));
    }

    void writePoint(QXmlStreamWriter &xml, const QString &elementName, const QGeoCoordinate &coord, const QString &name = QString())
    {
        xml.writeStartElement(elementName);
        xml.writeAttribute(QStringLiteral("lat"), QString::number(coord.latitude(), 'f', 8));
        xml.writeAttribute(QStringLiteral("lon"), QString::number(coord.longitude(), 'f', 8));

        if (!std::isnan(coord.altitude())) {
            xml.writeTextElement(QStringLiteral("ele"), QString::number(coord.altitude(), 'f', 2));
        }

        if (!name.isEmpty()) {
            xml.writeTextElement(QStringLiteral("name"), name);
        }

        xml.writeEndElement();
    }

    void writeRoute(QXmlStreamWriter &xml, const QList<QGeoCoordinate> &coords, const QString &name = QString())
    {
        xml.writeStartElement(QStringLiteral("rte"));

        if (!name.isEmpty()) {
            xml.writeTextElement(QStringLiteral("name"), name);
        }

        for (const QGeoCoordinate &coord : coords) {
            writePoint(xml, QStringLiteral("rtept"), coord);
        }

        xml.writeEndElement();
    }

    void writeTrack(QXmlStreamWriter &xml, const QList<QGeoCoordinate> &coords, const QString &name = QString())
    {
        xml.writeStartElement(QStringLiteral("trk"));

        if (!name.isEmpty()) {
            xml.writeTextElement(QStringLiteral("name"), name);
        }

        xml.writeStartElement(QStringLiteral("trkseg"));
        for (const QGeoCoordinate &coord : coords) {
            writePoint(xml, QStringLiteral("trkpt"), coord);
        }
        xml.writeEndElement();

        xml.writeEndElement();
    }

    struct GPXData {
        QList<QGeoCoordinate> waypoints;
        QList<QList<QGeoCoordinate>> routes;
        QList<QList<QGeoCoordinate>> trackSegments;
    };

    bool parseGPXFile(const QString &filePath, GPXData &data, QString &errorString)
    {
        QFile file;
        if (!openFileForRead(filePath, file, errorString)) {
            return false;
        }

        QXmlStreamReader xml(&file);

        while (!xml.atEnd() && !xml.hasError()) {
            const QXmlStreamReader::TokenType token = xml.readNext();

            if (token == QXmlStreamReader::StartElement) {
                if (xml.name() == QStringLiteral("wpt")) {
                    const QGeoCoordinate coord = parsePoint(xml);
                    if (coord.isValid()) {
                        data.waypoints.append(coord);
                    }
                } else if (xml.name() == QStringLiteral("rte")) {
                    const QList<QGeoCoordinate> route = parseRoute(xml);
                    if (!route.isEmpty()) {
                        data.routes.append(route);
                    }
                } else if (xml.name() == QStringLiteral("trk")) {
                    const QList<QList<QGeoCoordinate>> segments = parseTrack(xml);
                    data.trackSegments.append(segments);
                }
            }
        }

        if (xml.hasError()) {
            errorString = QString(_errorPrefix).arg(QObject::tr("XML parse error at line %1: %2")
                .arg(xml.lineNumber())
                .arg(xml.errorString()));
            return false;
        }

        return true;
    }
}

namespace GPXHelper {

ShapeFileHelper::ShapeType determineShapeType(const QString &filePath, QString &errorString)
{
    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return ShapeFileHelper::ShapeType::Error;
    }

    for (const QList<QGeoCoordinate> &route : data.routes) {
        if (isClosed(route)) {
            return ShapeFileHelper::ShapeType::Polygon;
        }
    }

    for (const QList<QGeoCoordinate> &segment : data.trackSegments) {
        if (isClosed(segment)) {
            return ShapeFileHelper::ShapeType::Polygon;
        }
    }

    if (!data.routes.isEmpty() || !data.trackSegments.isEmpty()) {
        return ShapeFileHelper::ShapeType::Polyline;
    }

    if (!data.waypoints.isEmpty()) {
        return ShapeFileHelper::ShapeType::Point;
    }

    errorString = QString(_errorPrefix).arg(QObject::tr("No geometry found in GPX file"));
    return ShapeFileHelper::ShapeType::Error;
}

int getEntityCount(const QString &filePath, QString &errorString)
{
    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return 0;
    }

    return data.waypoints.count() + data.routes.count() + data.trackSegments.count();
}

bool loadPolygonFromFile(const QString &filePath, QList<QGeoCoordinate> &vertices, QString &errorString, double filterMeters)
{
    vertices.clear();

    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return false;
    }

    for (const QList<QGeoCoordinate> &route : data.routes) {
        if (isClosed(route)) {
            vertices = route;
            if (!vertices.isEmpty() && vertices.first().distanceTo(vertices.last()) < _polygonClosureThresholdMeters) {
                vertices.removeLast();
            }
            ShapeFileHelper::filterVertices(vertices, filterMeters, 3);
            return true;
        }
    }

    for (const QList<QGeoCoordinate> &segment : data.trackSegments) {
        if (isClosed(segment)) {
            vertices = segment;
            if (!vertices.isEmpty() && vertices.first().distanceTo(vertices.last()) < _polygonClosureThresholdMeters) {
                vertices.removeLast();
            }
            ShapeFileHelper::filterVertices(vertices, filterMeters, 3);
            return true;
        }
    }

    errorString = QString(_errorPrefix).arg(QObject::tr("No closed polygon found in GPX file"));
    return false;
}

bool loadPolygonsFromFile(const QString &filePath, QList<QList<QGeoCoordinate>> &polygons, QString &errorString, double filterMeters)
{
    polygons.clear();

    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return false;
    }

    for (const QList<QGeoCoordinate> &route : data.routes) {
        if (isClosed(route)) {
            QList<QGeoCoordinate> vertices = route;
            if (!vertices.isEmpty() && vertices.first().distanceTo(vertices.last()) < _polygonClosureThresholdMeters) {
                vertices.removeLast();
            }
            ShapeFileHelper::filterVertices(vertices, filterMeters, 3);
            if (vertices.count() >= 3) {
                polygons.append(vertices);
            }
        }
    }

    for (const QList<QGeoCoordinate> &segment : data.trackSegments) {
        if (isClosed(segment)) {
            QList<QGeoCoordinate> vertices = segment;
            if (!vertices.isEmpty() && vertices.first().distanceTo(vertices.last()) < _polygonClosureThresholdMeters) {
                vertices.removeLast();
            }
            ShapeFileHelper::filterVertices(vertices, filterMeters, 3);
            if (vertices.count() >= 3) {
                polygons.append(vertices);
            }
        }
    }

    if (polygons.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QObject::tr("No closed polygons found in GPX file"));
        return false;
    }

    return true;
}

bool loadPolylineFromFile(const QString &filePath, QList<QGeoCoordinate> &coords, QString &errorString, double filterMeters)
{
    coords.clear();

    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return false;
    }

    if (!data.routes.isEmpty()) {
        coords = data.routes.first();
        ShapeFileHelper::filterVertices(coords, filterMeters, 2);
        return true;
    }

    if (!data.trackSegments.isEmpty()) {
        coords = data.trackSegments.first();
        ShapeFileHelper::filterVertices(coords, filterMeters, 2);
        return true;
    }

    errorString = QString(_errorPrefix).arg(QObject::tr("No routes or tracks found in GPX file"));
    return false;
}

bool loadPolylinesFromFile(const QString &filePath, QList<QList<QGeoCoordinate>> &polylines, QString &errorString, double filterMeters)
{
    polylines.clear();

    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return false;
    }

    for (QList<QGeoCoordinate> route : data.routes) {
        ShapeFileHelper::filterVertices(route, filterMeters, 2);
        if (route.count() >= 2) {
            polylines.append(route);
        }
    }

    for (QList<QGeoCoordinate> segment : data.trackSegments) {
        ShapeFileHelper::filterVertices(segment, filterMeters, 2);
        if (segment.count() >= 2) {
            polylines.append(segment);
        }
    }

    if (polylines.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QObject::tr("No routes or tracks found in GPX file"));
        return false;
    }

    return true;
}

bool loadPointsFromFile(const QString &filePath, QList<QGeoCoordinate> &points, QString &errorString)
{
    points.clear();

    GPXData data;
    if (!parseGPXFile(filePath, data, errorString)) {
        return false;
    }

    points = data.waypoints;

    if (points.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QObject::tr("No waypoints found in GPX file"));
        return false;
    }

    return true;
}

bool savePolygonToFile(const QString &filePath, const QList<QGeoCoordinate> &vertices, QString &errorString)
{
    if (vertices.count() < 3) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("Polygon must have at least 3 vertices"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    QList<QGeoCoordinate> closedVertices = vertices;
    if (closedVertices.first().distanceTo(closedVertices.last()) >= _polygonClosureThresholdMeters) {
        closedVertices.append(closedVertices.first());
    }

    writeRoute(xml, closedVertices, QStringLiteral("Polygon"));

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool savePolygonsToFile(const QString &filePath, const QList<QList<QGeoCoordinate>> &polygons, QString &errorString)
{
    if (polygons.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No polygons to save"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    int index = 1;
    for (const QList<QGeoCoordinate> &vertices : polygons) {
        if (vertices.count() < 3) {
            continue;
        }

        QList<QGeoCoordinate> closedVertices = vertices;
        if (closedVertices.first().distanceTo(closedVertices.last()) >= _polygonClosureThresholdMeters) {
            closedVertices.append(closedVertices.first());
        }

        writeRoute(xml, closedVertices, QStringLiteral("Polygon %1").arg(index++));
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool savePolylineToFile(const QString &filePath, const QList<QGeoCoordinate> &coords, QString &errorString)
{
    if (coords.count() < 2) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("Polyline must have at least 2 points"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    writeRoute(xml, coords, QStringLiteral("Route"));

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool savePolylinesToFile(const QString &filePath, const QList<QList<QGeoCoordinate>> &polylines, QString &errorString)
{
    if (polylines.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No polylines to save"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    int index = 1;
    for (const QList<QGeoCoordinate> &coords : polylines) {
        if (coords.count() >= 2) {
            writeRoute(xml, coords, QStringLiteral("Route %1").arg(index++));
        }
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool savePointsToFile(const QString &filePath, const QList<QGeoCoordinate> &points, QString &errorString)
{
    if (points.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No points to save"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    int index = 1;
    for (const QGeoCoordinate &point : points) {
        writePoint(xml, QStringLiteral("wpt"), point, QStringLiteral("WPT%1").arg(index++, 3, 10, QLatin1Char('0')));
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool saveTrackToFile(const QString &filePath, const QList<QGeoCoordinate> &coords, QString &errorString)
{
    if (coords.count() < 2) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("Track must have at least 2 points"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    writeTrack(xml, coords, QStringLiteral("Track"));

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

bool saveTracksToFile(const QString &filePath, const QList<QList<QGeoCoordinate>> &tracks, QString &errorString)
{
    if (tracks.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No tracks to save"));
        return false;
    }

    QFile file;
    if (!openFileForWrite(filePath, file, errorString)) {
        return false;
    }

    QXmlStreamWriter xml(&file);
    writeGpxHeader(xml);

    int index = 1;
    for (const QList<QGeoCoordinate> &coords : tracks) {
        if (coords.count() >= 2) {
            writeTrack(xml, coords, QStringLiteral("Track %1").arg(index++));
        }
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

} // namespace GPXHelper
