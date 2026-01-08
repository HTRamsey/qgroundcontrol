#include "KMLHelper.h"
#include "KMLSchemaValidator.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtXml/QDomDocument>

#include <algorithm>

QGC_LOGGING_CATEGORY(KMLHelperLog, "Utilities.KMLHelper")

namespace KMLHelper
{
    QDomDocument _loadFile(const QString &kmlFile, QString &errorString);
    bool _parseCoordinateString(const QString &coordinatesString, QList<QGeoCoordinate> &coords, QString &errorString);
    void _checkAltitudeMode(const QDomNode &geometryNode, const QString &geometryType, int index);

    constexpr const char *_errorPrefix = QT_TR_NOOP("KML file load failed. %1");
}

QDomDocument KMLHelper::_loadFile(const QString &kmlFile, QString &errorString)
{
    errorString.clear();

    QFile file(kmlFile);
    if (!file.exists()) {
        errorString = QString(_errorPrefix).arg(QString(QT_TRANSLATE_NOOP("KML", "File not found: %1")).arg(kmlFile));
        return QDomDocument();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        errorString = QString(_errorPrefix).arg(QString(QT_TRANSLATE_NOOP("KML", "Unable to open file: %1 error: %2")).arg(kmlFile, file.errorString()));
        return QDomDocument();
    }

    QDomDocument doc;
    const QDomDocument::ParseResult result = doc.setContent(&file, QDomDocument::ParseOption::Default);
    if (!result) {
        errorString = QString(_errorPrefix).arg(QString(QT_TRANSLATE_NOOP("KML", "Unable to parse KML file: %1 error: %2 line: %3")).arg(kmlFile).arg(result.errorMessage).arg(result.errorLine));
        return QDomDocument();
    }

    return doc;
}

bool KMLHelper::_parseCoordinateString(const QString &coordinatesString, QList<QGeoCoordinate> &coords, QString &errorString)
{
    coords.clear();
    const QString simplified = coordinatesString.simplified();
    if (simplified.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "Empty coordinates string"));
        return false;
    }

    const QStringList rgCoordinateStrings = simplified.split(' ');
    for (const QString &coordinateString : rgCoordinateStrings) {
        if (coordinateString.isEmpty()) {
            continue;
        }
        const QStringList rgValueStrings = coordinateString.split(',');
        if (rgValueStrings.size() < 2) {
            qCWarning(KMLHelperLog) << "Invalid coordinate format, expected lon,lat[,alt]:" << coordinateString;
            continue;
        }
        bool lonOk = false, latOk = false;
        const double lon = rgValueStrings[0].toDouble(&lonOk);
        const double lat = rgValueStrings[1].toDouble(&latOk);
        if (!lonOk || !latOk) {
            qCWarning(KMLHelperLog) << "Failed to parse coordinate values:" << coordinateString;
            continue;
        }
        if (lat < -90.0 || lat > 90.0) {
            qCWarning(KMLHelperLog) << "Latitude out of range [-90, 90]:" << lat << "in:" << coordinateString;
            continue;
        }
        if (lon < -180.0 || lon > 180.0) {
            qCWarning(KMLHelperLog) << "Longitude out of range [-180, 180]:" << lon << "in:" << coordinateString;
            continue;
        }
        double alt = 0.0;
        if (rgValueStrings.size() >= 3) {
            alt = rgValueStrings[2].toDouble();
        }
        coords.append(QGeoCoordinate(lat, lon, alt));
    }

    if (coords.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No valid coordinates found"));
        return false;
    }
    return true;
}


void KMLHelper::_checkAltitudeMode(const QDomNode &geometryNode, const QString &geometryType, int index)
{
    // Validate altitudeMode using schema-derived rules
    // QGC treats all coordinates as absolute (AMSL), so warn if a different mode is specified
    const QDomNode altModeNode = geometryNode.namedItem("altitudeMode");
    if (!altModeNode.isNull()) {
        const QString mode = altModeNode.toElement().text();
        if (mode.isEmpty()) {
            return;
        }
        const auto *validator = KMLSchemaValidator::instance();
        const QString location = QStringLiteral("(line %1)").arg(altModeNode.lineNumber());
        if (!validator->isValidEnumValue("altitudeModeEnumType", mode)) {
            qCWarning(KMLHelperLog) << geometryType << index << location << "has invalid altitudeMode:" << mode
                                    << "- valid values are:" << validator->validEnumValues("altitudeModeEnumType").join(", ");
        } else if (mode != "absolute") {
            qCWarning(KMLHelperLog) << geometryType << index << location << "uses altitudeMode:" << mode
                                    << "- QGC will treat coordinates as absolute (AMSL)";
        }
    }
}

ShapeFileHelper::ShapeType KMLHelper::determineShapeType(const QString &kmlFile, QString &errorString)
{
    using ShapeType = ShapeFileHelper::ShapeType;

    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return ShapeType::Error;
    }

    const QDomNodeList rgNodesPolygon = domDocument.elementsByTagName("Polygon");
    if (!rgNodesPolygon.isEmpty()) {
        return ShapeType::Polygon;
    }

    const QDomNodeList rgNodesLineString = domDocument.elementsByTagName("LineString");
    if (!rgNodesLineString.isEmpty()) {
        return ShapeType::Polyline;
    }

    const QDomNodeList rgNodesPoint = domDocument.elementsByTagName("Point");
    if (!rgNodesPoint.isEmpty()) {
        return ShapeType::Point;
    }

    errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No supported type found in KML file."));
    return ShapeType::Error;
}

int KMLHelper::getEntityCount(const QString &kmlFile, QString &errorString)
{
    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return 0;
    }

    int count = 0;
    count += domDocument.elementsByTagName("Polygon").count();
    count += domDocument.elementsByTagName("LineString").count();
    count += domDocument.elementsByTagName("Point").count();
    return count;
}

bool KMLHelper::loadPolygonFromFile(const QString &kmlFile, QList<QGeoCoordinate> &vertices, QString &errorString, double filterMeters)
{
    QList<QList<QGeoCoordinate>> polygons;
    if (!loadPolygonsFromFile(kmlFile, polygons, errorString, filterMeters)) {
        return false;
    }
    vertices = polygons.first();
    return true;
}

bool KMLHelper::loadPolygonsFromFile(const QString &kmlFile, QList<QList<QGeoCoordinate>> &polygons, QString &errorString, double filterMeters)
{
    errorString.clear();
    polygons.clear();

    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return false;
    }

    const QDomNodeList rgNodes = domDocument.elementsByTagName("Polygon");
    if (rgNodes.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "Unable to find Polygon node in KML"));
        return false;
    }

    for (int nodeIdx = 0; nodeIdx < rgNodes.count(); nodeIdx++) {
        const QDomNode polygonNode = rgNodes.item(nodeIdx);
        _checkAltitudeMode(polygonNode, "Polygon", nodeIdx);

        // Note: This function ignores holes. Use loadPolygonsWithHolesFromFile() to preserve hole information.
        const QDomNode coordinatesNode = polygonNode.namedItem("outerBoundaryIs").namedItem("LinearRing").namedItem("coordinates");
        if (coordinatesNode.isNull()) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << QStringLiteral("(line %1)").arg(polygonNode.lineNumber())
                                    << "missing coordinates node, skipping";
            continue;
        }

        QList<QGeoCoordinate> rgCoords;
        if (!_parseCoordinateString(coordinatesNode.toElement().text(), rgCoords, errorString)) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << QStringLiteral("(line %1)").arg(coordinatesNode.lineNumber())
                                    << "failed to parse coordinates:" << errorString;
            errorString.clear();
            continue;
        }

        if (rgCoords.count() < 3) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << QStringLiteral("(line %1)").arg(polygonNode.lineNumber())
                                    << "has fewer than 3 vertices, skipping";
            continue;
        }

        // Remove duplicate closing vertex (KML polygons repeat first vertex at end)
        if (rgCoords.count() > 3 && rgCoords.first().latitude() == rgCoords.last().latitude() &&
            rgCoords.first().longitude() == rgCoords.last().longitude()) {
            rgCoords.removeLast();
        }

        // Determine winding, reverse if needed. QGC wants clockwise winding
        double sum = 0;
        for (int i = 0; i < rgCoords.count(); i++) {
            const QGeoCoordinate &coord1 = rgCoords[i];
            const QGeoCoordinate &coord2 = rgCoords[(i + 1) % rgCoords.count()];
            sum += (coord2.longitude() - coord1.longitude()) * (coord2.latitude() + coord1.latitude());
        }
        if (sum < 0.0) {
            std::reverse(rgCoords.begin(), rgCoords.end());
        }

        ShapeFileHelper::filterVertices(rgCoords, filterMeters, 3);
        polygons.append(rgCoords);
    }

    if (polygons.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No valid polygons found in KML file"));
        return false;
    }

    return true;
}

bool KMLHelper::loadPolygonWithHolesFromFile(const QString &kmlFile, QGeoPolygon &polygon, QString &errorString)
{
    QList<QGeoPolygon> polygons;
    if (!loadPolygonsWithHolesFromFile(kmlFile, polygons, errorString)) {
        return false;
    }
    polygon = polygons.first();
    return true;
}

bool KMLHelper::loadPolygonsWithHolesFromFile(const QString &kmlFile, QList<QGeoPolygon> &polygons, QString &errorString)
{
    errorString.clear();
    polygons.clear();

    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return false;
    }

    const QDomNodeList rgNodes = domDocument.elementsByTagName("Polygon");
    if (rgNodes.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "Unable to find Polygon node in KML"));
        return false;
    }

    for (int nodeIdx = 0; nodeIdx < rgNodes.count(); nodeIdx++) {
        const QDomNode polygonNode = rgNodes.item(nodeIdx);
        _checkAltitudeMode(polygonNode, "Polygon", nodeIdx);

        // Parse outer boundary
        const QDomNode outerCoordinatesNode = polygonNode.namedItem("outerBoundaryIs").namedItem("LinearRing").namedItem("coordinates");
        if (outerCoordinatesNode.isNull()) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << QStringLiteral("(line %1)").arg(polygonNode.lineNumber())
                                    << "missing outer boundary coordinates, skipping";
            continue;
        }

        QList<QGeoCoordinate> outerRing;
        if (!_parseCoordinateString(outerCoordinatesNode.toElement().text(), outerRing, errorString)) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << "failed to parse outer boundary:" << errorString;
            errorString.clear();
            continue;
        }

        if (outerRing.count() < 3) {
            qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << "outer boundary has fewer than 3 vertices, skipping";
            continue;
        }

        // Remove duplicate closing vertex
        if (outerRing.count() > 3 && outerRing.first().latitude() == outerRing.last().latitude() &&
            outerRing.first().longitude() == outerRing.last().longitude()) {
            outerRing.removeLast();
        }

        // Ensure clockwise winding for outer ring
        double sum = 0;
        for (int i = 0; i < outerRing.count(); i++) {
            const QGeoCoordinate &coord1 = outerRing[i];
            const QGeoCoordinate &coord2 = outerRing[(i + 1) % outerRing.count()];
            sum += (coord2.longitude() - coord1.longitude()) * (coord2.latitude() + coord1.latitude());
        }
        if (sum < 0.0) {
            std::reverse(outerRing.begin(), outerRing.end());
        }

        QGeoPolygon geoPolygon(outerRing);

        // Parse inner boundaries (holes)
        const QDomNodeList innerBoundaries = polygonNode.toElement().elementsByTagName("innerBoundaryIs");
        for (int holeIdx = 0; holeIdx < innerBoundaries.count(); holeIdx++) {
            const QDomNode innerNode = innerBoundaries.item(holeIdx);
            const QDomNode innerCoordinatesNode = innerNode.namedItem("LinearRing").namedItem("coordinates");
            if (innerCoordinatesNode.isNull()) {
                qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << "hole" << holeIdx << "missing coordinates, skipping hole";
                continue;
            }

            QList<QGeoCoordinate> holeRing;
            if (!_parseCoordinateString(innerCoordinatesNode.toElement().text(), holeRing, errorString)) {
                qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << "hole" << holeIdx << "failed to parse:" << errorString;
                errorString.clear();
                continue;
            }

            if (holeRing.count() < 3) {
                qCWarning(KMLHelperLog) << "Polygon" << nodeIdx << "hole" << holeIdx << "has fewer than 3 vertices, skipping";
                continue;
            }

            // Remove duplicate closing vertex
            if (holeRing.count() > 3 && holeRing.first().latitude() == holeRing.last().latitude() &&
                holeRing.first().longitude() == holeRing.last().longitude()) {
                holeRing.removeLast();
            }

            geoPolygon.addHole(holeRing);
        }

        polygons.append(geoPolygon);
    }

    if (polygons.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No valid polygons found in KML file"));
        return false;
    }

    return true;
}

bool KMLHelper::loadPolylineFromFile(const QString &kmlFile, QList<QGeoCoordinate> &coords, QString &errorString, double filterMeters)
{
    QList<QList<QGeoCoordinate>> polylines;
    if (!loadPolylinesFromFile(kmlFile, polylines, errorString, filterMeters)) {
        return false;
    }
    coords = polylines.first();
    return true;
}

bool KMLHelper::loadPolylinesFromFile(const QString &kmlFile, QList<QList<QGeoCoordinate>> &polylines, QString &errorString, double filterMeters)
{
    errorString.clear();
    polylines.clear();

    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return false;
    }

    const QDomNodeList rgNodes = domDocument.elementsByTagName("LineString");
    if (rgNodes.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "Unable to find LineString node in KML"));
        return false;
    }

    for (int nodeIdx = 0; nodeIdx < rgNodes.count(); nodeIdx++) {
        const QDomNode lineStringNode = rgNodes.item(nodeIdx);
        _checkAltitudeMode(lineStringNode, "LineString", nodeIdx);

        const QDomNode coordinatesNode = lineStringNode.namedItem("coordinates");
        if (coordinatesNode.isNull()) {
            qCWarning(KMLHelperLog) << "LineString" << nodeIdx << QStringLiteral("(line %1)").arg(lineStringNode.lineNumber())
                                    << "missing coordinates node, skipping";
            continue;
        }

        QList<QGeoCoordinate> rgCoords;
        if (!_parseCoordinateString(coordinatesNode.toElement().text(), rgCoords, errorString)) {
            qCWarning(KMLHelperLog) << "LineString" << nodeIdx << QStringLiteral("(line %1)").arg(coordinatesNode.lineNumber())
                                    << "failed to parse coordinates:" << errorString;
            errorString.clear();
            continue;
        }

        if (rgCoords.count() < 2) {
            qCWarning(KMLHelperLog) << "LineString" << nodeIdx << QStringLiteral("(line %1)").arg(lineStringNode.lineNumber())
                                    << "has fewer than 2 vertices, skipping";
            continue;
        }

        ShapeFileHelper::filterVertices(rgCoords, filterMeters, 2);
        polylines.append(rgCoords);
    }

    if (polylines.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No valid polylines found in KML file"));
        return false;
    }

    return true;
}

bool KMLHelper::loadPointsFromFile(const QString &kmlFile, QList<QGeoCoordinate> &points, QString &errorString)
{
    errorString.clear();
    points.clear();

    const QDomDocument domDocument = KMLHelper::_loadFile(kmlFile, errorString);
    if (!errorString.isEmpty()) {
        return false;
    }

    const QDomNodeList rgNodes = domDocument.elementsByTagName("Point");
    if (rgNodes.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "Unable to find Point node in KML"));
        return false;
    }

    for (int nodeIdx = 0; nodeIdx < rgNodes.count(); nodeIdx++) {
        const QDomNode pointNode = rgNodes.item(nodeIdx);
        _checkAltitudeMode(pointNode, "Point", nodeIdx);

        const QDomNode coordinatesNode = pointNode.namedItem("coordinates");
        if (coordinatesNode.isNull()) {
            qCWarning(KMLHelperLog) << "Point" << nodeIdx << QStringLiteral("(line %1)").arg(pointNode.lineNumber())
                                    << "missing coordinates node, skipping";
            continue;
        }

        QList<QGeoCoordinate> coords;
        if (!_parseCoordinateString(coordinatesNode.toElement().text(), coords, errorString)) {
            qCWarning(KMLHelperLog) << "Point" << nodeIdx << QStringLiteral("(line %1)").arg(coordinatesNode.lineNumber())
                                    << "failed to parse coordinates:" << errorString;
            errorString.clear();
            continue;
        }

        if (!coords.isEmpty()) {
            points.append(coords.first());
        }
    }

    if (points.isEmpty()) {
        errorString = QString(_errorPrefix).arg(QT_TRANSLATE_NOOP("KML", "No valid points found in KML file"));
        return false;
    }

    return true;
}

// ============================================================================
// Save functions
// ============================================================================

namespace KMLHelper
{
    constexpr const char *_saveErrorPrefix = QT_TR_NOOP("KML file save failed. %1");
    constexpr const char *_kmlNamespace = "http://www.opengis.net/kml/2.2";

    QString _formatCoordinate(const QGeoCoordinate &coord)
    {
        if (qIsNaN(coord.altitude())) {
            return QStringLiteral("%1,%2,0").arg(coord.longitude(), 0, 'f', 8).arg(coord.latitude(), 0, 'f', 8);
        }
        return QStringLiteral("%1,%2,%3").arg(coord.longitude(), 0, 'f', 8).arg(coord.latitude(), 0, 'f', 8).arg(coord.altitude(), 0, 'f', 2);
    }

    QString _formatCoordinates(const QList<QGeoCoordinate> &coords)
    {
        QStringList coordStrings;
        for (const QGeoCoordinate &coord : coords) {
            coordStrings.append(_formatCoordinate(coord));
        }
        return coordStrings.join(' ');
    }

    bool _hasAnyAltitude(const QList<QGeoCoordinate> &coords)
    {
        for (const QGeoCoordinate &coord : coords) {
            if (!qIsNaN(coord.altitude())) {
                return true;
            }
        }
        return false;
    }

    QDomDocument _createKmlDocument()
    {
        QDomDocument doc;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
        QDomElement kml = doc.createElement("kml");
        kml.setAttribute("xmlns", _kmlNamespace);
        doc.appendChild(kml);
        QDomElement document = doc.createElement("Document");
        kml.appendChild(document);
        return doc;
    }

    QDomElement _getDocumentElement(QDomDocument &doc)
    {
        return doc.documentElement().firstChildElement("Document");
    }

    void _addAltitudeMode(QDomDocument &doc, QDomElement &parent, bool hasAltitude)
    {
        if (hasAltitude) {
            QDomElement altMode = doc.createElement("altitudeMode");
            altMode.appendChild(doc.createTextNode("absolute"));
            parent.appendChild(altMode);
        }
    }

    bool _saveDocument(const QDomDocument &doc, const QString &kmlFile, QString &errorString)
    {
        QFile file(kmlFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            errorString = QString(_saveErrorPrefix).arg(
                QObject::tr("Unable to create file: %1").arg(file.errorString()));
            return false;
        }
        QTextStream stream(&file);
        stream << doc.toString(2);
        return true;
    }
}

bool KMLHelper::savePolygonToFile(const QString &kmlFile, const QList<QGeoCoordinate> &vertices, QString &errorString)
{
    return savePolygonsToFile(kmlFile, {vertices}, errorString);
}

bool KMLHelper::savePolygonsToFile(const QString &kmlFile, const QList<QList<QGeoCoordinate>> &polygons, QString &errorString)
{
    errorString.clear();

    if (polygons.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No polygons to save"));
        return false;
    }

    QDomDocument doc = _createKmlDocument();
    QDomElement document = _getDocumentElement(doc);

    for (int i = 0; i < polygons.count(); i++) {
        const QList<QGeoCoordinate> &vertices = polygons[i];
        if (vertices.count() < 3) {
            qCWarning(KMLHelperLog) << "Skipping polygon" << i << "with fewer than 3 vertices";
            continue;
        }

        QDomElement placemark = doc.createElement("Placemark");
        document.appendChild(placemark);

        QDomElement polygon = doc.createElement("Polygon");
        placemark.appendChild(polygon);

        const bool hasAltitude = _hasAnyAltitude(vertices);
        _addAltitudeMode(doc, polygon, hasAltitude);

        QDomElement outerBoundary = doc.createElement("outerBoundaryIs");
        polygon.appendChild(outerBoundary);

        QDomElement linearRing = doc.createElement("LinearRing");
        outerBoundary.appendChild(linearRing);

        // Close the ring (KML requires first == last for polygons)
        QList<QGeoCoordinate> closedVertices = vertices;
        if (closedVertices.first().latitude() != closedVertices.last().latitude() ||
            closedVertices.first().longitude() != closedVertices.last().longitude()) {
            closedVertices.append(closedVertices.first());
        }

        QDomElement coordinates = doc.createElement("coordinates");
        coordinates.appendChild(doc.createTextNode(_formatCoordinates(closedVertices)));
        linearRing.appendChild(coordinates);
    }

    return _saveDocument(doc, kmlFile, errorString);
}

bool KMLHelper::savePolygonWithHolesToFile(const QString &kmlFile, const QGeoPolygon &polygon, QString &errorString)
{
    return savePolygonsWithHolesToFile(kmlFile, {polygon}, errorString);
}

bool KMLHelper::savePolygonsWithHolesToFile(const QString &kmlFile, const QList<QGeoPolygon> &polygons, QString &errorString)
{
    errorString.clear();

    if (polygons.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No polygons to save"));
        return false;
    }

    QDomDocument doc = _createKmlDocument();
    QDomElement document = _getDocumentElement(doc);

    for (int i = 0; i < polygons.count(); i++) {
        const QGeoPolygon &geoPolygon = polygons[i];
        const QList<QGeoCoordinate> perimeter = geoPolygon.perimeter();

        if (perimeter.count() < 3) {
            qCWarning(KMLHelperLog) << "Skipping polygon" << i << "with fewer than 3 vertices";
            continue;
        }

        QDomElement placemark = doc.createElement("Placemark");
        document.appendChild(placemark);

        QDomElement polygon = doc.createElement("Polygon");
        placemark.appendChild(polygon);

        // Check if any coordinate in outer ring or holes has altitude
        bool hasAltitude = _hasAnyAltitude(perimeter);
        for (int h = 0; !hasAltitude && h < geoPolygon.holesCount(); h++) {
            hasAltitude = _hasAnyAltitude(geoPolygon.holePath(h));
        }
        _addAltitudeMode(doc, polygon, hasAltitude);

        // Outer boundary
        QDomElement outerBoundary = doc.createElement("outerBoundaryIs");
        polygon.appendChild(outerBoundary);

        QDomElement outerLinearRing = doc.createElement("LinearRing");
        outerBoundary.appendChild(outerLinearRing);

        // Close the ring
        QList<QGeoCoordinate> closedPerimeter = perimeter;
        if (closedPerimeter.first().latitude() != closedPerimeter.last().latitude() ||
            closedPerimeter.first().longitude() != closedPerimeter.last().longitude()) {
            closedPerimeter.append(closedPerimeter.first());
        }

        QDomElement outerCoordinates = doc.createElement("coordinates");
        outerCoordinates.appendChild(doc.createTextNode(_formatCoordinates(closedPerimeter)));
        outerLinearRing.appendChild(outerCoordinates);

        // Inner boundaries (holes)
        for (int h = 0; h < geoPolygon.holesCount(); h++) {
            const QList<QGeoCoordinate> holePath = geoPolygon.holePath(h);

            QDomElement innerBoundary = doc.createElement("innerBoundaryIs");
            polygon.appendChild(innerBoundary);

            QDomElement innerLinearRing = doc.createElement("LinearRing");
            innerBoundary.appendChild(innerLinearRing);

            // Close the ring
            QList<QGeoCoordinate> closedHole = holePath;
            if (closedHole.first().latitude() != closedHole.last().latitude() ||
                closedHole.first().longitude() != closedHole.last().longitude()) {
                closedHole.append(closedHole.first());
            }

            QDomElement innerCoordinates = doc.createElement("coordinates");
            innerCoordinates.appendChild(doc.createTextNode(_formatCoordinates(closedHole)));
            innerLinearRing.appendChild(innerCoordinates);
        }
    }

    return _saveDocument(doc, kmlFile, errorString);
}

bool KMLHelper::savePolylineToFile(const QString &kmlFile, const QList<QGeoCoordinate> &coords, QString &errorString)
{
    return savePolylinesToFile(kmlFile, {coords}, errorString);
}

bool KMLHelper::savePolylinesToFile(const QString &kmlFile, const QList<QList<QGeoCoordinate>> &polylines, QString &errorString)
{
    errorString.clear();

    if (polylines.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No polylines to save"));
        return false;
    }

    QDomDocument doc = _createKmlDocument();
    QDomElement document = _getDocumentElement(doc);

    for (int i = 0; i < polylines.count(); i++) {
        const QList<QGeoCoordinate> &coords = polylines[i];
        if (coords.count() < 2) {
            qCWarning(KMLHelperLog) << "Skipping polyline" << i << "with fewer than 2 vertices";
            continue;
        }

        QDomElement placemark = doc.createElement("Placemark");
        document.appendChild(placemark);

        QDomElement lineString = doc.createElement("LineString");
        placemark.appendChild(lineString);

        const bool hasAltitude = _hasAnyAltitude(coords);
        _addAltitudeMode(doc, lineString, hasAltitude);

        QDomElement coordinates = doc.createElement("coordinates");
        coordinates.appendChild(doc.createTextNode(_formatCoordinates(coords)));
        lineString.appendChild(coordinates);
    }

    return _saveDocument(doc, kmlFile, errorString);
}

bool KMLHelper::savePointsToFile(const QString &kmlFile, const QList<QGeoCoordinate> &points, QString &errorString)
{
    errorString.clear();

    if (points.isEmpty()) {
        errorString = QString(_saveErrorPrefix).arg(QObject::tr("No points to save"));
        return false;
    }

    QDomDocument doc = _createKmlDocument();
    QDomElement document = _getDocumentElement(doc);

    for (int i = 0; i < points.count(); i++) {
        const QGeoCoordinate &point = points[i];
        if (!point.isValid()) {
            qCWarning(KMLHelperLog) << "Skipping invalid point" << i;
            continue;
        }

        QDomElement placemark = doc.createElement("Placemark");
        document.appendChild(placemark);

        QDomElement pointElem = doc.createElement("Point");
        placemark.appendChild(pointElem);

        const bool hasAltitude = !qIsNaN(point.altitude());
        _addAltitudeMode(doc, pointElem, hasAltitude);

        QDomElement coordinates = doc.createElement("coordinates");
        coordinates.appendChild(doc.createTextNode(_formatCoordinate(point)));
        pointElem.appendChild(coordinates);
    }

    return _saveDocument(doc, kmlFile, errorString);
}
