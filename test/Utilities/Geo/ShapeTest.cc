#include "ShapeTest.h"
#include "GeoFormatRegistry.h"
#include "GeoJsonHelper.h"
#include "GeoPackageHelper.h"
#include "GPXHelper.h"
#include "KMLHelper.h"
#include "OpenAirParser.h"
#include "ShapeFileHelper.h"
#include "SHPFileHelper.h"
#include "KMLDomDocument.h"
#include "KMLSchemaValidator.h"
#include "WKBHelper.h"
#include "WKTHelper.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTextStream>
#include <QtTest/QTest>

QString ShapeTest::_copyRes(const QTemporaryDir &tmpDir, const QString &name)
{
    const QString dstPath = tmpDir.filePath(name);
    (void) QFile::remove(dstPath);
    const QString resPath = QStringLiteral(":/unittest/%1").arg(name);
    (void) QFile(resPath).copy(dstPath);
    return dstPath;
}

void ShapeTest::_writePrjFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
}

QString ShapeTest::_writeKmlFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content)
{
    const QString path = tmpDir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
    return path;
}

void ShapeTest::_testLoadPolylineFromSHP()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "pline.shp");
    (void) _copyRes(tmpDir, "pline.dbf");
    (void) _copyRes(tmpDir, "pline.shx");
    (void) _copyRes(tmpDir, "pline.prj");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    // Use 0 for filterMeters to disable vertex filtering for test
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(shpFile, rgCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(rgCoords.count() > 0);
}

void ShapeTest::_testLoadPolylineFromKML()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polyline.kml");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(shpFile, rgCoords, errorString));
    QVERIFY(errorString.isEmpty());
    QVERIFY(rgCoords.count() > 0);
}

void ShapeTest::_testLoadPolygonFromSHP()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");
    (void) _copyRes(tmpDir, "polygon.prj");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    // Use 0 for filterMeters to disable vertex filtering for test
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(shpFile, rgCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(rgCoords.count() >= 3);
}

void ShapeTest::_testLoadPolygonFromKML()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polygon.kml");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(shpFile, rgCoords, errorString));
    QVERIFY(errorString.isEmpty());
    QVERIFY(rgCoords.count() >= 3);
}

void ShapeTest::_testLoadPointsFromSHP()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "point.shp");
    (void) _copyRes(tmpDir, "point.dbf");
    (void) _copyRes(tmpDir, "point.shx");
    (void) _copyRes(tmpDir, "point.prj");
    QString errorString;
    QList<QGeoCoordinate> points;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(shpFile, points, errorString));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(points.count(), 3);

    // Verify first point coordinates
    QCOMPARE(points[0].longitude(), -122.0);
    QCOMPARE(points[0].latitude(), 37.0);
    QCOMPARE(points[0].altitude(), 100.0);

    // Verify last point coordinates
    QCOMPARE(points[2].longitude(), -121.0);
    QCOMPARE(points[2].latitude(), 38.0);
    QCOMPARE(points[2].altitude(), 200.0);
}

void ShapeTest::_testLoadPointsFromKML()
{
    const QTemporaryDir tmpDir;
    const QString kmlFile = _copyRes(tmpDir, "point.kml");
    QString errorString;
    QList<QGeoCoordinate> points;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(kmlFile, points, errorString));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(points.count(), 3);

    // Verify first point coordinates
    QCOMPARE(points[0].longitude(), -122.0);
    QCOMPARE(points[0].latitude(), 37.0);
    QCOMPARE(points[0].altitude(), 100.0);

    // Verify last point coordinates
    QCOMPARE(points[2].longitude(), -121.0);
    QCOMPARE(points[2].latitude(), 38.0);
    QCOMPARE(points[2].altitude(), 200.0);
}

void ShapeTest::_testLoadPolygonsFromSHP()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");
    (void) _copyRes(tmpDir, "polygon.prj");
    QString errorString;
    QList<QList<QGeoCoordinate>> polygons;
    // Use 0 for filterMeters to disable vertex filtering for test
    QVERIFY(ShapeFileHelper::loadPolygonsFromFile(shpFile, polygons, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(polygons.count() >= 1);
    QVERIFY(polygons.first().count() >= 3);
}

void ShapeTest::_testLoadPolylinesFromSHP()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "pline.shp");
    (void) _copyRes(tmpDir, "pline.dbf");
    (void) _copyRes(tmpDir, "pline.shx");
    (void) _copyRes(tmpDir, "pline.prj");
    QString errorString;
    QList<QList<QGeoCoordinate>> polylines;
    // Use 0 for filterMeters to disable vertex filtering for test
    QVERIFY(ShapeFileHelper::loadPolylinesFromFile(shpFile, polylines, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(polylines.count() >= 1);
    QVERIFY(polylines.first().count() >= 2);
}

void ShapeTest::_testLoadPolygonsFromKML()
{
    const QTemporaryDir tmpDir;
    const QString kmlFile = _copyRes(tmpDir, "polygon.kml");
    QString errorString;
    QList<QList<QGeoCoordinate>> polygons;
    QVERIFY(ShapeFileHelper::loadPolygonsFromFile(kmlFile, polygons, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(polygons.count() >= 1);
    QVERIFY(polygons.first().count() >= 3);
}

void ShapeTest::_testLoadPolylinesFromKML()
{
    const QTemporaryDir tmpDir;
    const QString kmlFile = _copyRes(tmpDir, "polyline.kml");
    QString errorString;
    QList<QList<QGeoCoordinate>> polylines;
    QVERIFY(ShapeFileHelper::loadPolylinesFromFile(kmlFile, polylines, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QVERIFY(polylines.count() >= 1);
    QVERIFY(polylines.first().count() >= 2);
}

void ShapeTest::_testGetEntityCount()
{
    const QTemporaryDir tmpDir;

    // Test SHP entity count
    const QString shpFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");
    (void) _copyRes(tmpDir, "polygon.prj");
    QString errorString;
    const int shpCount = ShapeFileHelper::getEntityCount(shpFile, errorString);
    QVERIFY(errorString.isEmpty());
    QVERIFY(shpCount >= 1);

    // Test KML polygon entity count
    const QString kmlPolygonFile = _copyRes(tmpDir, "polygon.kml");
    const int kmlPolygonCount = ShapeFileHelper::getEntityCount(kmlPolygonFile, errorString);
    QVERIFY(errorString.isEmpty());
    QCOMPARE(kmlPolygonCount, 1);

    // Test KML polyline entity count
    const QString kmlPolylineFile = _copyRes(tmpDir, "polyline.kml");
    const int kmlPolylineCount = ShapeFileHelper::getEntityCount(kmlPolylineFile, errorString);
    QVERIFY(errorString.isEmpty());
    QCOMPARE(kmlPolylineCount, 1);
}

void ShapeTest::_testDetermineShapeType()
{
    const QTemporaryDir tmpDir;

    // Test polygon type detection
    const QString polygonFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");
    (void) _copyRes(tmpDir, "polygon.prj");
    QString errorString;
    QCOMPARE(ShapeFileHelper::determineShapeType(polygonFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QVERIFY(errorString.isEmpty());

    // Test polyline type detection
    const QString polylineFile = _copyRes(tmpDir, "pline.shp");
    (void) _copyRes(tmpDir, "pline.dbf");
    (void) _copyRes(tmpDir, "pline.shx");
    (void) _copyRes(tmpDir, "pline.prj");
    QCOMPARE(ShapeFileHelper::determineShapeType(polylineFile, errorString), ShapeFileHelper::ShapeType::Polyline);
    QVERIFY(errorString.isEmpty());

    // Test KML type detection
    const QString kmlFile = _copyRes(tmpDir, "polygon.kml");
    QCOMPARE(ShapeFileHelper::determineShapeType(kmlFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QVERIFY(errorString.isEmpty());
}

void ShapeTest::_testUnsupportedProjectionError()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");

    // Write an unsupported projection PRJ file
    const QString prjPath = tmpDir.filePath("polygon.prj");
    _writePrjFile(prjPath, "PROJCS[\"NAD_1983_StatePlane_California_III\",GEOGCS[\"GCS_North_American_1983\"]]");

    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    QVERIFY(!ShapeFileHelper::loadPolygonFromFile(shpFile, rgCoords, errorString));
    QVERIFY(errorString.contains("NAD_1983_StatePlane_California_III"));
    QVERIFY(errorString.contains("WGS84"));
}

void ShapeTest::_testLoadFromQtResource()
{
    // Test loading directly from Qt resource path (tests QFile hooks)
    // Note: This requires the test resources to include all required files (.shp, .shx, .prj)
    QString errorString;
    QList<QGeoCoordinate> rgCoords;

    // Test KML from resource (KML already uses QFile internally)
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(":/unittest/polygon.kml", rgCoords, errorString));
    QVERIFY(errorString.isEmpty());
    QVERIFY(rgCoords.count() >= 3);
}

void ShapeTest::_testVertexFiltering()
{
    const QTemporaryDir tmpDir;
    const QString shpFile = _copyRes(tmpDir, "polygon.shp");
    (void) _copyRes(tmpDir, "polygon.dbf");
    (void) _copyRes(tmpDir, "polygon.shx");
    (void) _copyRes(tmpDir, "polygon.prj");

    QString errorString;
    QList<QGeoCoordinate> unfilteredCoords;
    QList<QGeoCoordinate> filteredCoords;

    // Load without filtering
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(shpFile, unfilteredCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());

    // Load with filtering (default 5m)
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(shpFile, filteredCoords, errorString));
    QVERIFY(errorString.isEmpty());

    // Filtered should have same or fewer vertices than unfiltered
    QVERIFY(filteredCoords.count() <= unfilteredCoords.count());

    // Both should have at least minimum valid polygon (3 vertices)
    // Note: If the test data has vertices very close together, filtered may have fewer
    QVERIFY(unfilteredCoords.count() >= 3);
}

void ShapeTest::_testKMLVertexFiltering()
{
    const QTemporaryDir tmpDir;

    // Create KML with closely spaced vertices (< 5m apart at equator)
    // 0.00001 degrees ≈ 1.1m at equator
    const QString kmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Placemark>
    <Polygon>
      <outerBoundaryIs>
        <LinearRing>
          <coordinates>
            0.0,0.0,0 0.00001,0.0,0 0.00002,0.0,0 0.00003,0.0,0 0.001,0.001,0 0.0,0.001,0 0.0,0.0,0
          </coordinates>
        </LinearRing>
      </outerBoundaryIs>
    </Polygon>
  </Placemark>
</kml>)";

    const QString kmlFile = _writeKmlFile(tmpDir, "dense_polygon.kml", kmlContent);

    QString errorString;
    QList<QGeoCoordinate> unfilteredCoords;
    QList<QGeoCoordinate> filteredCoords;

    // Load without filtering
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(kmlFile, unfilteredCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(unfilteredCoords.count(), 6);  // 7 coords - 1 duplicate closing = 6

    // Load with filtering (default 5m)
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(kmlFile, filteredCoords, errorString));
    QVERIFY(errorString.isEmpty());

    // Filtered should have fewer vertices (the closely spaced ones should be filtered)
    QVERIFY(filteredCoords.count() < unfilteredCoords.count());
    QVERIFY(filteredCoords.count() >= 3);  // Minimum valid polygon
}

void ShapeTest::_testKMLAltitudeParsing()
{
    const QTemporaryDir tmpDir;

    // Create KML with altitude values
    const QString kmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Placemark>
    <Polygon>
      <outerBoundaryIs>
        <LinearRing>
          <coordinates>
            -122.0,37.0,100.5 -122.0,38.0,200.0 -121.0,38.0,150.0 -121.0,37.0,175.5 -122.0,37.0,100.5
          </coordinates>
        </LinearRing>
      </outerBoundaryIs>
    </Polygon>
  </Placemark>
</kml>)";

    const QString kmlFile = _writeKmlFile(tmpDir, "altitude_polygon.kml", kmlContent);

    QString errorString;
    QList<QGeoCoordinate> coords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(kmlFile, coords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(coords.count(), 4);  // 5 coords - 1 duplicate closing = 4

    // Verify altitudes were parsed correctly
    QCOMPARE(coords[0].altitude(), 100.5);
    QCOMPARE(coords[1].altitude(), 200.0);
    QCOMPARE(coords[2].altitude(), 150.0);
    QCOMPARE(coords[3].altitude(), 175.5);

    // Verify lat/lon as well
    QCOMPARE(coords[0].latitude(), 37.0);
    QCOMPARE(coords[0].longitude(), -122.0);
}

void ShapeTest::_testKMLCoordinateValidation()
{
    const QTemporaryDir tmpDir;

    // Create KML with some invalid coordinates mixed with valid ones
    // Invalid: lat=91 (out of range), lon=200 (out of range), swapped lat/lon
    const QString kmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Placemark>
    <Polygon>
      <outerBoundaryIs>
        <LinearRing>
          <coordinates>
            -122.0,37.0,0 -122.0,91.0,0 200.0,38.0,0 -121.0,38.0,0 -121.0,37.0,0 -122.0,37.0,0
          </coordinates>
        </LinearRing>
      </outerBoundaryIs>
    </Polygon>
  </Placemark>
</kml>)";

    const QString kmlFile = _writeKmlFile(tmpDir, "invalid_coords.kml", kmlContent);

    // Expect warnings for invalid coordinates that will be normalized
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Latitude out of range.*91"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Longitude out of range.*200"));

    QString errorString;
    QList<QGeoCoordinate> coords;
    // Should succeed and normalize invalid coordinates (clamp lat, wrap lon)
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(kmlFile, coords, errorString, 0));
    QVERIFY(errorString.isEmpty());

    // Should have 5 coordinates (all normalized, duplicate closing removed, then reversed for clockwise winding)
    // Original KML: -122,37 | -122,91 | 200,38 | -121,38 | -121,37 | -122,37 (closing)
    // After normalize: -122,37 | -122,90 | -160,38 | -121,38 | -121,37 (closing removed)
    // After clockwise winding check (reversed): -121,37 | -121,38 | -160,38 | -122,90 | -122,37
    QCOMPARE(coords.count(), 5);

    // Verify the coordinates are present, normalized, and in clockwise order
    QCOMPARE(coords[0].latitude(), 37.0);
    QCOMPARE(coords[0].longitude(), -121.0);
    QCOMPARE(coords[1].latitude(), 38.0);
    QCOMPARE(coords[1].longitude(), -121.0);
    QCOMPARE(coords[2].latitude(), 38.0);
    QCOMPARE(coords[2].longitude(), -160.0);  // Wrapped from 200
    QCOMPARE(coords[3].latitude(), 90.0);     // Clamped from 91
    QCOMPARE(coords[3].longitude(), -122.0);
    QCOMPARE(coords[4].latitude(), 37.0);
    QCOMPARE(coords[4].longitude(), -122.0);
}

void ShapeTest::_testKMLExportSchemaValidation()
{
    // Test that KMLSchemaValidator is properly loaded and functional
    const auto *validator = KMLSchemaValidator::instance();

    // Verify schema was loaded and enum types were extracted
    const QStringList altitudeModes = validator->validEnumValues("altitudeModeEnumType");
    QVERIFY(!altitudeModes.isEmpty());
    QVERIFY(altitudeModes.contains("absolute"));
    QVERIFY(altitudeModes.contains("clampToGround"));
    QVERIFY(altitudeModes.contains("relativeToGround"));

    // Verify element validation works
    QVERIFY(validator->isValidElement("Polygon"));
    QVERIFY(validator->isValidElement("LineString"));
    QVERIFY(validator->isValidElement("Point"));
    QVERIFY(validator->isValidElement("coordinates"));
    QVERIFY(validator->isValidElement("altitudeMode"));

    // Create a KML document and validate it
    KMLDomDocument doc("Test Export");
    QDomElement placemark = doc.addPlacemark("Test Point", true);
    QDomElement point = doc.createElement("Point");
    doc.addTextElement(point, "altitudeMode", "absolute");
    doc.addTextElement(point, "coordinates", doc.kmlCoordString(QGeoCoordinate(37.0, -122.0, 100.0)));
    placemark.appendChild(point);

    // Validate the document
    const auto result = validator->validate(doc);
    if (!result.isValid) {
        qWarning() << "KML validation errors:" << result.errors;
    }
    QVERIFY(result.isValid);
    QVERIFY(result.errors.isEmpty());

    // Test validation catches invalid altitudeMode
    const QString invalidKml = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <Placemark>
      <Point>
        <altitudeMode>invalidMode</altitudeMode>
        <coordinates>-122.0,37.0,0</coordinates>
      </Point>
    </Placemark>
  </Document>
</kml>)";

    const QTemporaryDir tmpDir;
    const QString invalidKmlPath = _writeKmlFile(tmpDir, "invalid.kml", invalidKml);
    const auto invalidResult = validator->validateFile(invalidKmlPath);
    QVERIFY(!invalidResult.isValid);
    QVERIFY(invalidResult.errors.first().contains("invalidMode"));

    // Test validation catches out-of-range coordinates
    const QString badCoordsKml = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Document>
    <Placemark>
      <Point>
        <coordinates>200.0,95.0,0</coordinates>
      </Point>
    </Placemark>
  </Document>
</kml>)";

    const QString badCoordsPath = _writeKmlFile(tmpDir, "bad_coords.kml", badCoordsKml);
    const auto badCoordsResult = validator->validateFile(badCoordsPath);
    QVERIFY(!badCoordsResult.isValid);
    QVERIFY(badCoordsResult.errors.size() >= 2);  // lat and lon both out of range
}

// ============================================================================
// GeoJSON Tests
// ============================================================================

QString ShapeTest::_writeGeoJsonFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content)
{
    const QString path = tmpDir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
    return path;
}

void ShapeTest::_testLoadPolygonFromGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polygon
    QList<QGeoCoordinate> originalVertices;
    originalVertices.append(QGeoCoordinate(37.0, -122.0));
    originalVertices.append(QGeoCoordinate(38.0, -122.0));
    originalVertices.append(QGeoCoordinate(38.0, -121.0));
    originalVertices.append(QGeoCoordinate(37.0, -121.0));

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(geoJsonFile, originalVertices, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back via ShapeFileHelper facade
    QList<QGeoCoordinate> loadedCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(geoJsonFile, loadedCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedCoords.count(), 4);

    // Verify coordinates
    QCOMPARE(loadedCoords[0].latitude(), 37.0);
    QCOMPARE(loadedCoords[0].longitude(), -122.0);
}

void ShapeTest::_testLoadPolylineFromGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polyline
    QList<QGeoCoordinate> originalCoords;
    originalCoords.append(QGeoCoordinate(37.0, -122.0));
    originalCoords.append(QGeoCoordinate(37.5, -122.5));
    originalCoords.append(QGeoCoordinate(38.0, -123.0));

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("polyline.geojson");
    QVERIFY(GeoJsonHelper::savePolylineToFile(geoJsonFile, originalCoords, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back via ShapeFileHelper facade
    QList<QGeoCoordinate> loadedCoords;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(geoJsonFile, loadedCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedCoords.count(), 3);

    QCOMPARE(loadedCoords[0].latitude(), 37.0);
    QCOMPARE(loadedCoords[0].longitude(), -122.0);
    QCOMPARE(loadedCoords[2].latitude(), 38.0);
    QCOMPARE(loadedCoords[2].longitude(), -123.0);
}

void ShapeTest::_testLoadPolygonsFromGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create two polygons
    QList<QGeoCoordinate> poly1;
    poly1.append(QGeoCoordinate(37.0, -122.0));
    poly1.append(QGeoCoordinate(38.0, -122.0));
    poly1.append(QGeoCoordinate(38.0, -121.0));
    poly1.append(QGeoCoordinate(37.0, -121.0));

    QList<QGeoCoordinate> poly2;
    poly2.append(QGeoCoordinate(35.0, -120.0));
    poly2.append(QGeoCoordinate(36.0, -120.0));
    poly2.append(QGeoCoordinate(36.0, -119.0));
    poly2.append(QGeoCoordinate(35.0, -119.0));

    QList<QList<QGeoCoordinate>> originalPolygons;
    originalPolygons.append(poly1);
    originalPolygons.append(poly2);

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("polygons.geojson");
    QVERIFY(GeoJsonHelper::savePolygonsToFile(geoJsonFile, originalPolygons, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back
    QList<QList<QGeoCoordinate>> loadedPolygons;
    QVERIFY(ShapeFileHelper::loadPolygonsFromFile(geoJsonFile, loadedPolygons, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPolygons.count(), 2);
    QCOMPARE(loadedPolygons[0].count(), 4);
    QCOMPARE(loadedPolygons[1].count(), 4);

    // Verify first polygon
    QCOMPARE(loadedPolygons[0][0].latitude(), 37.0);
    QCOMPARE(loadedPolygons[0][0].longitude(), -122.0);

    // Verify second polygon
    QCOMPARE(loadedPolygons[1][0].latitude(), 35.0);
    QCOMPARE(loadedPolygons[1][0].longitude(), -120.0);
}

void ShapeTest::_testLoadPolylinesFromGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create two polylines
    QList<QGeoCoordinate> line1;
    line1.append(QGeoCoordinate(37.0, -122.0));
    line1.append(QGeoCoordinate(37.5, -122.5));

    QList<QGeoCoordinate> line2;
    line2.append(QGeoCoordinate(35.0, -120.0));
    line2.append(QGeoCoordinate(35.5, -120.5));
    line2.append(QGeoCoordinate(36.0, -121.0));

    QList<QList<QGeoCoordinate>> originalPolylines;
    originalPolylines.append(line1);
    originalPolylines.append(line2);

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("polylines.geojson");
    QVERIFY(GeoJsonHelper::savePolylinesToFile(geoJsonFile, originalPolylines, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back
    QList<QList<QGeoCoordinate>> loadedPolylines;
    QVERIFY(ShapeFileHelper::loadPolylinesFromFile(geoJsonFile, loadedPolylines, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPolylines.count(), 2);
    QCOMPARE(loadedPolylines[0].count(), 2);
    QCOMPARE(loadedPolylines[1].count(), 3);
}

void ShapeTest::_testLoadPointsFromGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create points
    QList<QGeoCoordinate> originalPoints;
    originalPoints.append(QGeoCoordinate(37.0, -122.0));
    originalPoints.append(QGeoCoordinate(37.5, -121.5, 100.0));

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("points.geojson");
    QVERIFY(GeoJsonHelper::savePointsToFile(geoJsonFile, originalPoints, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back
    QList<QGeoCoordinate> loadedPoints;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(geoJsonFile, loadedPoints, errorString));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPoints.count(), 2);

    QCOMPARE(loadedPoints[0].latitude(), 37.0);
    QCOMPARE(loadedPoints[0].longitude(), -122.0);

    QCOMPARE(loadedPoints[1].latitude(), 37.5);
    QCOMPARE(loadedPoints[1].longitude(), -121.5);
    QCOMPARE(loadedPoints[1].altitude(), 100.0);
}

void ShapeTest::_testGeoJSONVertexFiltering()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create polygon with closely spaced vertices (< 5m apart at equator)
    // 0.00001 degrees ≈ 1.1m at equator
    QList<QGeoCoordinate> denseVertices;
    denseVertices.append(QGeoCoordinate(0.0, 0.0));
    denseVertices.append(QGeoCoordinate(0.0, 0.00001));
    denseVertices.append(QGeoCoordinate(0.0, 0.00002));
    denseVertices.append(QGeoCoordinate(0.0, 0.00003));
    denseVertices.append(QGeoCoordinate(0.001, 0.001));
    denseVertices.append(QGeoCoordinate(0.001, 0.0));

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("dense_polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(geoJsonFile, denseVertices, errorString));
    QVERIFY(errorString.isEmpty());

    // Load without filtering
    QList<QGeoCoordinate> unfilteredCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(geoJsonFile, unfilteredCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(unfilteredCoords.count(), 6);

    // Load with filtering (default 5m)
    QList<QGeoCoordinate> filteredCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(geoJsonFile, filteredCoords, errorString));
    QVERIFY(errorString.isEmpty());

    // Filtered should have fewer vertices
    QVERIFY(filteredCoords.count() < unfilteredCoords.count());
    QVERIFY(filteredCoords.count() >= 3);  // Minimum valid polygon
}

void ShapeTest::_testGeoJSONAltitudeParsing()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create polygon with altitude values
    QList<QGeoCoordinate> originalVertices;
    originalVertices.append(QGeoCoordinate(37.0, -122.0, 100.5));
    originalVertices.append(QGeoCoordinate(38.0, -122.0, 200.0));
    originalVertices.append(QGeoCoordinate(38.0, -121.0, 150.0));
    originalVertices.append(QGeoCoordinate(37.0, -121.0, 175.5));

    // Save to file
    const QString geoJsonFile = tmpDir.filePath("altitude_polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(geoJsonFile, originalVertices, errorString));
    QVERIFY(errorString.isEmpty());

    // Load back
    QList<QGeoCoordinate> loadedCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(geoJsonFile, loadedCoords, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedCoords.count(), 4);

    // Verify altitudes were preserved
    QCOMPARE(loadedCoords[0].altitude(), 100.5);
    QCOMPARE(loadedCoords[1].altitude(), 200.0);
    QCOMPARE(loadedCoords[2].altitude(), 150.0);
    QCOMPARE(loadedCoords[3].altitude(), 175.5);

    // Verify lat/lon
    QCOMPARE(loadedCoords[0].latitude(), 37.0);
    QCOMPARE(loadedCoords[0].longitude(), -122.0);
}

void ShapeTest::_testGeoJSONSaveFunctions()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Test save polygon
    QList<QGeoCoordinate> polygonVertices;
    polygonVertices.append(QGeoCoordinate(37.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -121.0));
    polygonVertices.append(QGeoCoordinate(37.0, -121.0));

    const QString savedPolygonFile = tmpDir.filePath("saved_polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(savedPolygonFile, polygonVertices, errorString));
    QVERIFY(errorString.isEmpty());

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolygon;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(savedPolygonFile, loadedPolygon, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPolygon.count(), polygonVertices.count());

    // Test save polyline
    QList<QGeoCoordinate> polylineCoords;
    polylineCoords.append(QGeoCoordinate(37.0, -122.0));
    polylineCoords.append(QGeoCoordinate(37.5, -122.5));
    polylineCoords.append(QGeoCoordinate(38.0, -123.0));

    const QString savedPolylineFile = tmpDir.filePath("saved_polyline.geojson");
    QVERIFY(GeoJsonHelper::savePolylineToFile(savedPolylineFile, polylineCoords, errorString));
    QVERIFY(errorString.isEmpty());

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolyline;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(savedPolylineFile, loadedPolyline, errorString, 0));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPolyline.count(), polylineCoords.count());

    // Test save points
    QList<QGeoCoordinate> points;
    points.append(QGeoCoordinate(37.0, -122.0, 100.0));
    points.append(QGeoCoordinate(38.0, -121.0, 200.0));

    const QString savedPointsFile = tmpDir.filePath("saved_points.geojson");
    QVERIFY(GeoJsonHelper::savePointsToFile(savedPointsFile, points, errorString));
    QVERIFY(errorString.isEmpty());

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPoints;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(savedPointsFile, loadedPoints, errorString));
    QVERIFY(errorString.isEmpty());
    QCOMPARE(loadedPoints.count(), points.count());
    QCOMPARE(loadedPoints[0].altitude(), 100.0);
}

void ShapeTest::_testGeoJSONDetermineShapeType()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Test polygon detection
    QList<QGeoCoordinate> polygonVertices;
    polygonVertices.append(QGeoCoordinate(37.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -121.0));
    polygonVertices.append(QGeoCoordinate(37.0, -121.0));

    const QString polygonFile = tmpDir.filePath("polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(polygonFile, polygonVertices, errorString));
    QCOMPARE(ShapeFileHelper::determineShapeType(polygonFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QVERIFY(errorString.isEmpty());

    // Test polyline detection
    QList<QGeoCoordinate> polylineCoords;
    polylineCoords.append(QGeoCoordinate(37.0, -122.0));
    polylineCoords.append(QGeoCoordinate(38.0, -123.0));

    const QString polylineFile = tmpDir.filePath("polyline.geojson");
    QVERIFY(GeoJsonHelper::savePolylineToFile(polylineFile, polylineCoords, errorString));
    QCOMPARE(ShapeFileHelper::determineShapeType(polylineFile, errorString), ShapeFileHelper::ShapeType::Polyline);
    QVERIFY(errorString.isEmpty());

    // Test point detection
    QList<QGeoCoordinate> points;
    points.append(QGeoCoordinate(37.0, -122.0));

    const QString pointFile = tmpDir.filePath("point.geojson");
    QVERIFY(GeoJsonHelper::savePointsToFile(pointFile, points, errorString));
    QCOMPARE(ShapeFileHelper::determineShapeType(pointFile, errorString), ShapeFileHelper::ShapeType::Point);
    QVERIFY(errorString.isEmpty());
}

void ShapeTest::_testSHPSaveFunctions()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // ========================================================================
    // Test save/load polygon round-trip
    // ========================================================================
    QList<QGeoCoordinate> polygonVertices;
    polygonVertices.append(QGeoCoordinate(37.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -121.0));
    polygonVertices.append(QGeoCoordinate(37.0, -121.0));

    const QString savedPolygonFile = tmpDir.filePath("saved_polygon.shp");
    QVERIFY(SHPFileHelper::savePolygonToFile(savedPolygonFile, polygonVertices, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify PRJ file was created
    const QString prjFile = tmpDir.filePath("saved_polygon.prj");
    QVERIFY(QFile::exists(prjFile));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolygon;
    QVERIFY(SHPFileHelper::loadPolygonFromFile(savedPolygonFile, loadedPolygon, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolygon.count(), polygonVertices.count());

    // Verify coordinates
    for (int i = 0; i < polygonVertices.count(); i++) {
        QVERIFY(qAbs(loadedPolygon[i].latitude() - polygonVertices[i].latitude()) < 0.0001);
        QVERIFY(qAbs(loadedPolygon[i].longitude() - polygonVertices[i].longitude()) < 0.0001);
    }

    // ========================================================================
    // Test save/load polyline round-trip
    // ========================================================================
    QList<QGeoCoordinate> polylineCoords;
    polylineCoords.append(QGeoCoordinate(37.0, -122.0));
    polylineCoords.append(QGeoCoordinate(37.5, -122.5));
    polylineCoords.append(QGeoCoordinate(38.0, -123.0));

    const QString savedPolylineFile = tmpDir.filePath("saved_polyline.shp");
    QVERIFY(SHPFileHelper::savePolylineToFile(savedPolylineFile, polylineCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolyline;
    QVERIFY(SHPFileHelper::loadPolylineFromFile(savedPolylineFile, loadedPolyline, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolyline.count(), polylineCoords.count());

    // ========================================================================
    // Test save/load points round-trip
    // ========================================================================
    QList<QGeoCoordinate> points;
    points.append(QGeoCoordinate(37.0, -122.0));
    points.append(QGeoCoordinate(38.0, -121.0));
    points.append(QGeoCoordinate(39.0, -120.0));

    const QString savedPointsFile = tmpDir.filePath("saved_points.shp");
    QVERIFY(SHPFileHelper::savePointsToFile(savedPointsFile, points, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPoints;
    QVERIFY(SHPFileHelper::loadPointsFromFile(savedPointsFile, loadedPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPoints.count(), points.count());

    // ========================================================================
    // Test save/load with altitude (Z values)
    // ========================================================================
    QList<QGeoCoordinate> polygonWithAlt;
    polygonWithAlt.append(QGeoCoordinate(37.0, -122.0, 100.0));
    polygonWithAlt.append(QGeoCoordinate(38.0, -122.0, 200.0));
    polygonWithAlt.append(QGeoCoordinate(38.0, -121.0, 150.0));
    polygonWithAlt.append(QGeoCoordinate(37.0, -121.0, 175.0));

    const QString savedPolygonZFile = tmpDir.filePath("saved_polygonz.shp");
    QVERIFY(SHPFileHelper::savePolygonToFile(savedPolygonZFile, polygonWithAlt, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back - altitudes should be preserved
    QList<QGeoCoordinate> loadedPolygonZ;
    QVERIFY(SHPFileHelper::loadPolygonFromFile(savedPolygonZFile, loadedPolygonZ, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolygonZ.count(), polygonWithAlt.count());

    // Verify altitudes
    QCOMPARE(loadedPolygonZ[0].altitude(), 100.0);
    QCOMPARE(loadedPolygonZ[1].altitude(), 200.0);
    QCOMPARE(loadedPolygonZ[2].altitude(), 150.0);
    QCOMPARE(loadedPolygonZ[3].altitude(), 175.0);

    // ========================================================================
    // Test multiple polygons
    // ========================================================================
    QList<QList<QGeoCoordinate>> multiPolygons;
    QList<QGeoCoordinate> poly1;
    poly1.append(QGeoCoordinate(37.0, -122.0));
    poly1.append(QGeoCoordinate(37.5, -122.0));
    poly1.append(QGeoCoordinate(37.5, -121.5));
    multiPolygons.append(poly1);

    QList<QGeoCoordinate> poly2;
    poly2.append(QGeoCoordinate(38.0, -123.0));
    poly2.append(QGeoCoordinate(38.5, -123.0));
    poly2.append(QGeoCoordinate(38.5, -122.5));
    multiPolygons.append(poly2);

    const QString savedMultiPolyFile = tmpDir.filePath("saved_multi_polygon.shp");
    QVERIFY(SHPFileHelper::savePolygonsToFile(savedMultiPolyFile, multiPolygons, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QList<QGeoCoordinate>> loadedMultiPolygons;
    QVERIFY(SHPFileHelper::loadPolygonsFromFile(savedMultiPolyFile, loadedMultiPolygons, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedMultiPolygons.count(), 2);

    // ========================================================================
    // Test determineShapeType on saved files
    // ========================================================================
    QCOMPARE(SHPFileHelper::determineShapeType(savedPolygonFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QCOMPARE(SHPFileHelper::determineShapeType(savedPolylineFile, errorString), ShapeFileHelper::ShapeType::Polyline);
    QCOMPARE(SHPFileHelper::determineShapeType(savedPointsFile, errorString), ShapeFileHelper::ShapeType::Point);
}

void ShapeTest::_testKMLSaveFunctions()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // ========================================================================
    // Test save/load polygon round-trip
    // ========================================================================
    QList<QGeoCoordinate> polygonVertices;
    polygonVertices.append(QGeoCoordinate(37.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -121.0));
    polygonVertices.append(QGeoCoordinate(37.0, -121.0));

    const QString savedPolygonFile = tmpDir.filePath("saved_polygon.kml");
    QVERIFY(KMLHelper::savePolygonToFile(savedPolygonFile, polygonVertices, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolygon;
    QVERIFY(KMLHelper::loadPolygonFromFile(savedPolygonFile, loadedPolygon, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolygon.count(), polygonVertices.count());

    // ========================================================================
    // Test save/load polyline round-trip
    // ========================================================================
    QList<QGeoCoordinate> polylineCoords;
    polylineCoords.append(QGeoCoordinate(37.0, -122.0));
    polylineCoords.append(QGeoCoordinate(37.5, -122.5));
    polylineCoords.append(QGeoCoordinate(38.0, -123.0));

    const QString savedPolylineFile = tmpDir.filePath("saved_polyline.kml");
    QVERIFY(KMLHelper::savePolylineToFile(savedPolylineFile, polylineCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolyline;
    QVERIFY(KMLHelper::loadPolylineFromFile(savedPolylineFile, loadedPolyline, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolyline.count(), polylineCoords.count());

    // ========================================================================
    // Test save/load points round-trip
    // ========================================================================
    QList<QGeoCoordinate> points;
    points.append(QGeoCoordinate(37.0, -122.0));
    points.append(QGeoCoordinate(38.0, -121.0));
    points.append(QGeoCoordinate(39.0, -120.0));

    const QString savedPointsFile = tmpDir.filePath("saved_points.kml");
    QVERIFY(KMLHelper::savePointsToFile(savedPointsFile, points, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPoints;
    QVERIFY(KMLHelper::loadPointsFromFile(savedPointsFile, loadedPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPoints.count(), points.count());

    // ========================================================================
    // Test save/load with altitude
    // ========================================================================
    QList<QGeoCoordinate> polygonWithAlt;
    polygonWithAlt.append(QGeoCoordinate(37.0, -122.0, 100.0));
    polygonWithAlt.append(QGeoCoordinate(38.0, -122.0, 200.0));
    polygonWithAlt.append(QGeoCoordinate(38.0, -121.0, 150.0));
    polygonWithAlt.append(QGeoCoordinate(37.0, -121.0, 175.0));

    const QString savedPolygonAltFile = tmpDir.filePath("saved_polygon_alt.kml");
    QVERIFY(KMLHelper::savePolygonToFile(savedPolygonAltFile, polygonWithAlt, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back - altitudes should be preserved
    QList<QGeoCoordinate> loadedPolygonAlt;
    QVERIFY(KMLHelper::loadPolygonFromFile(savedPolygonAltFile, loadedPolygonAlt, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolygonAlt.count(), polygonWithAlt.count());

    // Verify altitudes
    QCOMPARE(loadedPolygonAlt[0].altitude(), 100.0);
    QCOMPARE(loadedPolygonAlt[1].altitude(), 200.0);
    QCOMPARE(loadedPolygonAlt[2].altitude(), 150.0);
    QCOMPARE(loadedPolygonAlt[3].altitude(), 175.0);

    // ========================================================================
    // Test multiple polygons
    // ========================================================================
    QList<QList<QGeoCoordinate>> multiPolygons;
    QList<QGeoCoordinate> poly1;
    poly1.append(QGeoCoordinate(37.0, -122.0));
    poly1.append(QGeoCoordinate(37.5, -122.0));
    poly1.append(QGeoCoordinate(37.5, -121.5));
    multiPolygons.append(poly1);

    QList<QGeoCoordinate> poly2;
    poly2.append(QGeoCoordinate(38.0, -123.0));
    poly2.append(QGeoCoordinate(38.5, -123.0));
    poly2.append(QGeoCoordinate(38.5, -122.5));
    multiPolygons.append(poly2);

    const QString savedMultiPolyFile = tmpDir.filePath("saved_multi_polygon.kml");
    QVERIFY(KMLHelper::savePolygonsToFile(savedMultiPolyFile, multiPolygons, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QList<QGeoCoordinate>> loadedMultiPolygons;
    QVERIFY(KMLHelper::loadPolygonsFromFile(savedMultiPolyFile, loadedMultiPolygons, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedMultiPolygons.count(), 2);

    // ========================================================================
    // Test determineShapeType on saved files
    // ========================================================================
    QCOMPARE(KMLHelper::determineShapeType(savedPolygonFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QCOMPARE(KMLHelper::determineShapeType(savedPolylineFile, errorString), ShapeFileHelper::ShapeType::Polyline);
    QCOMPARE(KMLHelper::determineShapeType(savedPointsFile, errorString), ShapeFileHelper::ShapeType::Point);
}

// ============================================================================
// GPX Tests
// ============================================================================

QString ShapeTest::_writeGpxFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content)
{
    const QString path = tmpDir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
    return path;
}

void ShapeTest::_testLoadPolylineFromGPX()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with a route
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <rte>
    <name>Test Route</name>
    <rtept lat="37.0" lon="-122.0"><ele>100</ele></rtept>
    <rtept lat="37.5" lon="-122.5"><ele>150</ele></rtept>
    <rtept lat="38.0" lon="-123.0"><ele>200</ele></rtept>
  </rte>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "route.gpx", gpxContent);

    QList<QGeoCoordinate> coords;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(gpxFile, coords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(coords.count(), 3);
    QCOMPARE(coords[0].latitude(), 37.0);
    QCOMPARE(coords[0].longitude(), -122.0);
    QCOMPARE(coords[2].latitude(), 38.0);
}

void ShapeTest::_testLoadPolylinesFromGPX()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with multiple routes and a track
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <rte>
    <name>Route 1</name>
    <rtept lat="37.0" lon="-122.0"></rtept>
    <rtept lat="37.5" lon="-122.5"></rtept>
  </rte>
  <rte>
    <name>Route 2</name>
    <rtept lat="38.0" lon="-123.0"></rtept>
    <rtept lat="38.5" lon="-123.5"></rtept>
  </rte>
  <trk>
    <name>Track 1</name>
    <trkseg>
      <trkpt lat="39.0" lon="-124.0"></trkpt>
      <trkpt lat="39.5" lon="-124.5"></trkpt>
    </trkseg>
  </trk>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "multi.gpx", gpxContent);

    QList<QList<QGeoCoordinate>> polylines;
    QVERIFY(ShapeFileHelper::loadPolylinesFromFile(gpxFile, polylines, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(polylines.count(), 3);  // 2 routes + 1 track segment
}

void ShapeTest::_testLoadPolygonFromGPX()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with a closed route (polygon)
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <rte>
    <name>Closed Route</name>
    <rtept lat="37.0" lon="-122.0"></rtept>
    <rtept lat="38.0" lon="-122.0"></rtept>
    <rtept lat="38.0" lon="-121.0"></rtept>
    <rtept lat="37.0" lon="-121.0"></rtept>
    <rtept lat="37.0" lon="-122.0"></rtept>
  </rte>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "polygon.gpx", gpxContent);

    QList<QGeoCoordinate> vertices;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(gpxFile, vertices, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(vertices.count(), 4);  // Last point (duplicate of first) is removed
}

void ShapeTest::_testLoadPointsFromGPX()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with waypoints
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <wpt lat="37.0" lon="-122.0">
    <ele>100</ele>
    <name>WPT001</name>
  </wpt>
  <wpt lat="38.0" lon="-121.0">
    <ele>200</ele>
    <name>WPT002</name>
  </wpt>
  <wpt lat="39.0" lon="-120.0">
    <name>WPT003</name>
  </wpt>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "waypoints.gpx", gpxContent);

    QList<QGeoCoordinate> points;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(gpxFile, points, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(points.count(), 3);
    QCOMPARE(points[0].latitude(), 37.0);
    QCOMPARE(points[0].altitude(), 100.0);
    QCOMPARE(points[1].altitude(), 200.0);
    QVERIFY(std::isnan(points[2].altitude()));  // No elevation specified
}

void ShapeTest::_testGPXSaveFunctions()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // ========================================================================
    // Test save/load polygon round-trip
    // ========================================================================
    QList<QGeoCoordinate> polygonVertices;
    polygonVertices.append(QGeoCoordinate(37.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -122.0));
    polygonVertices.append(QGeoCoordinate(38.0, -121.0));
    polygonVertices.append(QGeoCoordinate(37.0, -121.0));

    const QString savedPolygonFile = tmpDir.filePath("saved_polygon.gpx");
    QVERIFY(GPXHelper::savePolygonToFile(savedPolygonFile, polygonVertices, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolygon;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(savedPolygonFile, loadedPolygon, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolygon.count(), polygonVertices.count());

    // ========================================================================
    // Test save/load polyline round-trip
    // ========================================================================
    QList<QGeoCoordinate> polylineCoords;
    polylineCoords.append(QGeoCoordinate(37.0, -122.0));
    polylineCoords.append(QGeoCoordinate(37.5, -122.5));
    polylineCoords.append(QGeoCoordinate(38.0, -123.0));

    const QString savedPolylineFile = tmpDir.filePath("saved_polyline.gpx");
    QVERIFY(GPXHelper::savePolylineToFile(savedPolylineFile, polylineCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPolyline;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(savedPolylineFile, loadedPolyline, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPolyline.count(), polylineCoords.count());

    // ========================================================================
    // Test save/load points round-trip
    // ========================================================================
    QList<QGeoCoordinate> points;
    points.append(QGeoCoordinate(37.0, -122.0, 100.0));
    points.append(QGeoCoordinate(38.0, -121.0, 200.0));

    const QString savedPointsFile = tmpDir.filePath("saved_points.gpx");
    QVERIFY(GPXHelper::savePointsToFile(savedPointsFile, points, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back
    QList<QGeoCoordinate> loadedPoints;
    QVERIFY(ShapeFileHelper::loadPointsFromFile(savedPointsFile, loadedPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedPoints.count(), points.count());

    // Verify altitudes
    QCOMPARE(loadedPoints[0].altitude(), 100.0);
    QCOMPARE(loadedPoints[1].altitude(), 200.0);

    // ========================================================================
    // Test save track
    // ========================================================================
    QList<QGeoCoordinate> trackCoords;
    trackCoords.append(QGeoCoordinate(37.0, -122.0, 50.0));
    trackCoords.append(QGeoCoordinate(37.1, -122.1, 55.0));
    trackCoords.append(QGeoCoordinate(37.2, -122.2, 60.0));

    const QString savedTrackFile = tmpDir.filePath("saved_track.gpx");
    QVERIFY(GPXHelper::saveTrackToFile(savedTrackFile, trackCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify by loading it back as polyline
    QList<QGeoCoordinate> loadedTrack;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(savedTrackFile, loadedTrack, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(loadedTrack.count(), trackCoords.count());

    // ========================================================================
    // Test determineShapeType on saved files
    // ========================================================================
    QCOMPARE(GPXHelper::determineShapeType(savedPolygonFile, errorString), ShapeFileHelper::ShapeType::Polygon);
    QCOMPARE(GPXHelper::determineShapeType(savedPolylineFile, errorString), ShapeFileHelper::ShapeType::Polyline);
    QCOMPARE(GPXHelper::determineShapeType(savedPointsFile, errorString), ShapeFileHelper::ShapeType::Point);
}

void ShapeTest::_testGPXTrackParsing()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with multiple track segments
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <trk>
    <name>Multi-segment Track</name>
    <trkseg>
      <trkpt lat="37.0" lon="-122.0"></trkpt>
      <trkpt lat="37.1" lon="-122.1"></trkpt>
    </trkseg>
    <trkseg>
      <trkpt lat="38.0" lon="-123.0"></trkpt>
      <trkpt lat="38.1" lon="-123.1"></trkpt>
    </trkseg>
  </trk>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "multiseg.gpx", gpxContent);

    QList<QList<QGeoCoordinate>> polylines;
    QVERIFY(GPXHelper::loadPolylinesFromFile(gpxFile, polylines, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(polylines.count(), 2);  // Each track segment becomes a polyline
    QCOMPARE(polylines[0].count(), 2);
    QCOMPARE(polylines[1].count(), 2);
}

void ShapeTest::_testGPXAltitudeParsing()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // GPX with various altitude scenarios
    const QString gpxContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<gpx version="1.1" creator="QGC Test">
  <wpt lat="37.0" lon="-122.0">
    <ele>100.5</ele>
  </wpt>
  <wpt lat="38.0" lon="-121.0">
    <ele>-50.25</ele>
  </wpt>
  <wpt lat="39.0" lon="-120.0">
  </wpt>
  <rte>
    <rtept lat="40.0" lon="-119.0"><ele>500</ele></rtept>
    <rtept lat="41.0" lon="-118.0"></rtept>
  </rte>
</gpx>)";

    const QString gpxFile = _writeGpxFile(tmpDir, "altitude.gpx", gpxContent);

    QList<QGeoCoordinate> points;
    QVERIFY(GPXHelper::loadPointsFromFile(gpxFile, points, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(points.count(), 3);
    QCOMPARE(points[0].altitude(), 100.5);
    QCOMPARE(points[1].altitude(), -50.25);  // Negative altitude (below sea level)
    QVERIFY(std::isnan(points[2].altitude()));  // No altitude

    // Test route with mixed altitudes
    QList<QGeoCoordinate> routeCoords;
    QVERIFY(GPXHelper::loadPolylineFromFile(gpxFile, routeCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));
    QCOMPARE(routeCoords.count(), 2);
    QCOMPARE(routeCoords[0].altitude(), 500.0);
    QVERIFY(std::isnan(routeCoords[1].altitude()));
}

// ============================================================================
// Round-trip tests (save then load to verify data integrity)
// ============================================================================

void ShapeTest::_testRoundTripPolygonKMLToSHP()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polygon coordinates
    QList<QGeoCoordinate> originalCoords;
    originalCoords.append(QGeoCoordinate(37.0, -122.0, 100.0));
    originalCoords.append(QGeoCoordinate(37.1, -122.0, 110.0));
    originalCoords.append(QGeoCoordinate(37.1, -122.1, 120.0));
    originalCoords.append(QGeoCoordinate(37.0, -122.1, 130.0));
    originalCoords.append(QGeoCoordinate(37.0, -122.0, 100.0));  // Closed polygon

    // Save to KML
    const QString kmlFile = tmpDir.filePath("polygon.kml");
    QVERIFY(KMLHelper::savePolygonToFile(kmlFile, originalCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from KML
    QList<QGeoCoordinate> kmlCoords;
    QVERIFY(KMLHelper::loadPolygonFromFile(kmlFile, kmlCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Save to SHP
    const QString shpFile = tmpDir.filePath("polygon.shp");
    QVERIFY(SHPFileHelper::savePolygonToFile(shpFile, kmlCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from SHP
    QList<QGeoCoordinate> shpCoords;
    QVERIFY(SHPFileHelper::loadPolygonFromFile(shpFile, shpCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify coordinates match (within tolerance for floating point)
    // Note: SHP may not preserve altitude, so only compare lat/lon
    QCOMPARE(shpCoords.count(), kmlCoords.count());
    for (int i = 0; i < shpCoords.count(); ++i) {
        QVERIFY2(qAbs(shpCoords[i].latitude() - kmlCoords[i].latitude()) < 0.0001,
                 qPrintable(QString("Latitude mismatch at index %1: %2 vs %3")
                            .arg(i).arg(shpCoords[i].latitude()).arg(kmlCoords[i].latitude())));
        QVERIFY2(qAbs(shpCoords[i].longitude() - kmlCoords[i].longitude()) < 0.0001,
                 qPrintable(QString("Longitude mismatch at index %1: %2 vs %3")
                            .arg(i).arg(shpCoords[i].longitude()).arg(kmlCoords[i].longitude())));
    }
}

void ShapeTest::_testRoundTripPolygonKMLToGeoJSON()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polygon coordinates
    QList<QGeoCoordinate> originalCoords;
    originalCoords.append(QGeoCoordinate(47.0, -122.5, 50.0));
    originalCoords.append(QGeoCoordinate(47.2, -122.5, 55.0));
    originalCoords.append(QGeoCoordinate(47.2, -122.8, 60.0));
    originalCoords.append(QGeoCoordinate(47.0, -122.8, 65.0));
    originalCoords.append(QGeoCoordinate(47.0, -122.5, 50.0));  // Closed polygon

    // Save to KML
    const QString kmlFile = tmpDir.filePath("polygon.kml");
    QVERIFY(KMLHelper::savePolygonToFile(kmlFile, originalCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from KML
    QList<QGeoCoordinate> kmlCoords;
    QVERIFY(KMLHelper::loadPolygonFromFile(kmlFile, kmlCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Save to GeoJSON
    const QString geoJsonFile = tmpDir.filePath("polygon.geojson");
    QVERIFY(GeoJsonHelper::savePolygonToFile(geoJsonFile, kmlCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from GeoJSON
    QList<QGeoCoordinate> geoJsonCoords;
    QVERIFY(GeoJsonHelper::loadPolygonFromFile(geoJsonFile, geoJsonCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify coordinates match (GeoJSON preserves altitude)
    QCOMPARE(geoJsonCoords.count(), kmlCoords.count());
    for (int i = 0; i < geoJsonCoords.count(); ++i) {
        QVERIFY2(qAbs(geoJsonCoords[i].latitude() - kmlCoords[i].latitude()) < 0.0001,
                 qPrintable(QString("Latitude mismatch at index %1").arg(i)));
        QVERIFY2(qAbs(geoJsonCoords[i].longitude() - kmlCoords[i].longitude()) < 0.0001,
                 qPrintable(QString("Longitude mismatch at index %1").arg(i)));
        if (!std::isnan(kmlCoords[i].altitude())) {
            QVERIFY2(qAbs(geoJsonCoords[i].altitude() - kmlCoords[i].altitude()) < 0.01,
                     qPrintable(QString("Altitude mismatch at index %1").arg(i)));
        }
    }
}

void ShapeTest::_testRoundTripPolygonKMLToGPX()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polygon coordinates
    QList<QGeoCoordinate> originalCoords;
    originalCoords.append(QGeoCoordinate(40.0, -105.0, 1600.0));
    originalCoords.append(QGeoCoordinate(40.1, -105.0, 1650.0));
    originalCoords.append(QGeoCoordinate(40.1, -105.1, 1700.0));
    originalCoords.append(QGeoCoordinate(40.0, -105.1, 1750.0));
    originalCoords.append(QGeoCoordinate(40.0, -105.0, 1600.0));  // Closed polygon

    // Save to KML
    const QString kmlFile = tmpDir.filePath("polygon.kml");
    QVERIFY(KMLHelper::savePolygonToFile(kmlFile, originalCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from KML
    QList<QGeoCoordinate> kmlCoords;
    QVERIFY(KMLHelper::loadPolygonFromFile(kmlFile, kmlCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Save to GPX
    const QString gpxFile = tmpDir.filePath("polygon.gpx");
    QVERIFY(GPXHelper::savePolygonToFile(gpxFile, kmlCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Load from GPX
    QList<QGeoCoordinate> gpxCoords;
    QVERIFY(GPXHelper::loadPolygonFromFile(gpxFile, gpxCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify coordinates match (GPX preserves altitude)
    QCOMPARE(gpxCoords.count(), kmlCoords.count());
    for (int i = 0; i < gpxCoords.count(); ++i) {
        QVERIFY2(qAbs(gpxCoords[i].latitude() - kmlCoords[i].latitude()) < 0.0001,
                 qPrintable(QString("Latitude mismatch at index %1").arg(i)));
        QVERIFY2(qAbs(gpxCoords[i].longitude() - kmlCoords[i].longitude()) < 0.0001,
                 qPrintable(QString("Longitude mismatch at index %1").arg(i)));
        if (!std::isnan(kmlCoords[i].altitude())) {
            QVERIFY2(qAbs(gpxCoords[i].altitude() - kmlCoords[i].altitude()) < 0.01,
                     qPrintable(QString("Altitude mismatch at index %1").arg(i)));
        }
    }
}

void ShapeTest::_testRoundTripPolylineAllFormats()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test polyline coordinates (flight path)
    QList<QGeoCoordinate> originalCoords;
    originalCoords.append(QGeoCoordinate(33.0, -117.0, 500.0));
    originalCoords.append(QGeoCoordinate(33.1, -117.1, 550.0));
    originalCoords.append(QGeoCoordinate(33.2, -117.0, 600.0));
    originalCoords.append(QGeoCoordinate(33.3, -116.9, 650.0));

    // Test KML -> GeoJSON -> GPX -> KML round-trip
    const QString kmlFile1 = tmpDir.filePath("polyline1.kml");
    QVERIFY(KMLHelper::savePolylineToFile(kmlFile1, originalCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> kmlCoords;
    QVERIFY(KMLHelper::loadPolylineFromFile(kmlFile1, kmlCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString geoJsonFile = tmpDir.filePath("polyline.geojson");
    QVERIFY(GeoJsonHelper::savePolylineToFile(geoJsonFile, kmlCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> geoJsonCoords;
    QVERIFY(GeoJsonHelper::loadPolylineFromFile(geoJsonFile, geoJsonCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString gpxFile = tmpDir.filePath("polyline.gpx");
    QVERIFY(GPXHelper::savePolylineToFile(gpxFile, geoJsonCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> gpxCoords;
    QVERIFY(GPXHelper::loadPolylineFromFile(gpxFile, gpxCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString kmlFile2 = tmpDir.filePath("polyline2.kml");
    QVERIFY(KMLHelper::savePolylineToFile(kmlFile2, gpxCoords, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> finalCoords;
    QVERIFY(KMLHelper::loadPolylineFromFile(kmlFile2, finalCoords, errorString, 0));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify final coords match original
    QCOMPARE(finalCoords.count(), originalCoords.count());
    for (int i = 0; i < finalCoords.count(); ++i) {
        QVERIFY2(qAbs(finalCoords[i].latitude() - originalCoords[i].latitude()) < 0.0001,
                 qPrintable(QString("Latitude mismatch at index %1: expected %2, got %3")
                            .arg(i).arg(originalCoords[i].latitude()).arg(finalCoords[i].latitude())));
        QVERIFY2(qAbs(finalCoords[i].longitude() - originalCoords[i].longitude()) < 0.0001,
                 qPrintable(QString("Longitude mismatch at index %1: expected %2, got %3")
                            .arg(i).arg(originalCoords[i].longitude()).arg(finalCoords[i].longitude())));
        if (!std::isnan(originalCoords[i].altitude())) {
            QVERIFY2(qAbs(finalCoords[i].altitude() - originalCoords[i].altitude()) < 0.01,
                     qPrintable(QString("Altitude mismatch at index %1: expected %2, got %3")
                                .arg(i).arg(originalCoords[i].altitude()).arg(finalCoords[i].altitude())));
        }
    }
}

void ShapeTest::_testRoundTripPointsAllFormats()
{
    const QTemporaryDir tmpDir;
    QString errorString;

    // Create test waypoints
    QList<QGeoCoordinate> originalPoints;
    originalPoints.append(QGeoCoordinate(45.0, -110.0, 1000.0));
    originalPoints.append(QGeoCoordinate(45.5, -110.5, 1100.0));
    originalPoints.append(QGeoCoordinate(46.0, -111.0, 1200.0));

    // Test KML -> GeoJSON -> GPX -> KML round-trip
    const QString kmlFile1 = tmpDir.filePath("points1.kml");
    QVERIFY(KMLHelper::savePointsToFile(kmlFile1, originalPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> kmlPoints;
    QVERIFY(KMLHelper::loadPointsFromFile(kmlFile1, kmlPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString geoJsonFile = tmpDir.filePath("points.geojson");
    QVERIFY(GeoJsonHelper::savePointsToFile(geoJsonFile, kmlPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> geoJsonPoints;
    QVERIFY(GeoJsonHelper::loadPointsFromFile(geoJsonFile, geoJsonPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString gpxFile = tmpDir.filePath("points.gpx");
    QVERIFY(GPXHelper::savePointsToFile(gpxFile, geoJsonPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> gpxPoints;
    QVERIFY(GPXHelper::loadPointsFromFile(gpxFile, gpxPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    const QString kmlFile2 = tmpDir.filePath("points2.kml");
    QVERIFY(KMLHelper::savePointsToFile(kmlFile2, gpxPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    QList<QGeoCoordinate> finalPoints;
    QVERIFY(KMLHelper::loadPointsFromFile(kmlFile2, finalPoints, errorString));
    QVERIFY2(errorString.isEmpty(), qPrintable(errorString));

    // Verify final points match original
    QCOMPARE(finalPoints.count(), originalPoints.count());
    for (int i = 0; i < finalPoints.count(); ++i) {
        QVERIFY2(qAbs(finalPoints[i].latitude() - originalPoints[i].latitude()) < 0.0001,
                 qPrintable(QString("Latitude mismatch at point %1: expected %2, got %3")
                            .arg(i).arg(originalPoints[i].latitude()).arg(finalPoints[i].latitude())));
        QVERIFY2(qAbs(finalPoints[i].longitude() - originalPoints[i].longitude()) < 0.0001,
                 qPrintable(QString("Longitude mismatch at point %1: expected %2, got %3")
                            .arg(i).arg(originalPoints[i].longitude()).arg(finalPoints[i].longitude())));
        if (!std::isnan(originalPoints[i].altitude())) {
            QVERIFY2(qAbs(finalPoints[i].altitude() - originalPoints[i].altitude()) < 0.01,
                     qPrintable(QString("Altitude mismatch at point %1: expected %2, got %3")
                                .arg(i).arg(originalPoints[i].altitude()).arg(finalPoints[i].altitude())));
        }
    }
}

void ShapeTest::_testSupportsFeatureAPI()
{
    using Feature = ShapeFileHelper::Feature;

    // KML supports most features
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::Polygons));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::PolygonsWithHoles));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::Polylines));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::Points));
    QVERIFY(!ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::Tracks));  // KML doesn't have GPX-style tracks
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::Altitude));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".kml", Feature::MultipleEntities));

    // GPX supports tracks but not polygons with holes
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::Polygons));
    QVERIFY(!ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::PolygonsWithHoles));  // GPX doesn't support holes
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::Polylines));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::Points));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::Tracks));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".gpx", Feature::Altitude));

    // SHP supports all basic features
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".shp", Feature::Polygons));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".shp", Feature::PolygonsWithHoles));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".shp", Feature::Polylines));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".shp", Feature::Points));
    QVERIFY(!ShapeFileHelper::supportsFeatureByExtension(".shp", Feature::Tracks));

    // GeoJSON supports all basic features
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::Polygons));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::PolygonsWithHoles));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::Polylines));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::Points));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::Altitude));
    QVERIFY(ShapeFileHelper::supportsFeatureByExtension(".geojson", Feature::MultipleEntities));

    // Unknown extension should return false for all features
    QVERIFY(!ShapeFileHelper::supportsFeatureByExtension(".xyz", Feature::Polygons));
    QVERIFY(!ShapeFileHelper::supportsFeatureByExtension(".xyz", Feature::Tracks));

    // Also test with file path
    QVERIFY(ShapeFileHelper::supportsFeature("/some/path/file.kml", Feature::Polygons));
    QVERIFY(ShapeFileHelper::supportsFeature("/some/path/file.gpx", Feature::Tracks));
    QVERIFY(!ShapeFileHelper::supportsFeature("/some/path/file.kml", Feature::Tracks));
}

void ShapeTest::_testSaveInvalidCoordinates()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create coordinates with an invalid value (latitude > 90)
    QList<QGeoCoordinate> invalidPolygon;
    invalidPolygon << QGeoCoordinate(100.0, 0.0);  // Invalid latitude
    invalidPolygon << QGeoCoordinate(0.0, 1.0);
    invalidPolygon << QGeoCoordinate(0.0, -1.0);

    QString errorString;

    // Test that save functions reject invalid coordinates for all formats
    const QString kmlFile = tmpDir.filePath("invalid.kml");
    QVERIFY(!ShapeFileHelper::savePolygonToFile(kmlFile, invalidPolygon, errorString));
    QVERIFY(!errorString.isEmpty());
    qDebug() << "KML save rejected with:" << errorString;

    errorString.clear();
    const QString geoJsonFile = tmpDir.filePath("invalid.geojson");
    QVERIFY(!ShapeFileHelper::savePolygonToFile(geoJsonFile, invalidPolygon, errorString));
    QVERIFY(!errorString.isEmpty());
    qDebug() << "GeoJSON save rejected with:" << errorString;

    errorString.clear();
    const QString gpxFile = tmpDir.filePath("invalid.gpx");
    QVERIFY(!ShapeFileHelper::savePolygonToFile(gpxFile, invalidPolygon, errorString));
    QVERIFY(!errorString.isEmpty());
    qDebug() << "GPX save rejected with:" << errorString;

    errorString.clear();
    const QString shpFile = tmpDir.filePath("invalid.shp");
    QVERIFY(!ShapeFileHelper::savePolygonToFile(shpFile, invalidPolygon, errorString));
    QVERIFY(!errorString.isEmpty());
    qDebug() << "SHP save rejected with:" << errorString;

    // Test with invalid longitude
    QList<QGeoCoordinate> invalidLongitude;
    invalidLongitude << QGeoCoordinate(0.0, 200.0);  // Invalid longitude
    invalidLongitude << QGeoCoordinate(1.0, 0.0);
    invalidLongitude << QGeoCoordinate(-1.0, 0.0);

    errorString.clear();
    QVERIFY(!ShapeFileHelper::savePolygonToFile(kmlFile, invalidLongitude, errorString));
    QVERIFY(!errorString.isEmpty());
    qDebug() << "Invalid longitude rejected with:" << errorString;
}

// ============================================================================
// OpenAir Parser Tests
// ============================================================================

void ShapeTest::_testOpenAirParsePolygon()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create a simple OpenAir file with a polygon
    const QString content = R"(
AC R
AN Test Restricted Area
AH 5000FT MSL
AL GND
DP 47:00:00N 008:00:00E
DP 47:00:00N 009:00:00E
DP 46:00:00N 009:00:00E
DP 46:00:00N 008:00:00E
)";

    const QString filePath = tmpDir.filePath("test.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content.toUtf8());
    file.close();

    OpenAirParser::ParseResult result = OpenAirParser::parseFile(filePath);
    QVERIFY(result.success);
    QCOMPARE(result.airspaces.count(), 1);
    QCOMPARE(result.airspaces[0].name, QStringLiteral("Test Restricted Area"));
    QVERIFY(result.airspaces[0].boundary.count() >= 4);
}

void ShapeTest::_testOpenAirParseCircle()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create an OpenAir file with a circle
    const QString content = R"(
AC D
AN Test Circle
AH FL100
AL 2000FT MSL
V X=47:30:00N 008:30:00E
DC 5
)";

    const QString filePath = tmpDir.filePath("circle.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content.toUtf8());
    file.close();

    OpenAirParser::ParseResult result = OpenAirParser::parseFile(filePath);
    QVERIFY(result.success);
    QCOMPARE(result.airspaces.count(), 1);
    // Circle should generate multiple points (72 by default)
    QVERIFY(result.airspaces[0].boundary.count() >= 36);
}

void ShapeTest::_testOpenAirParseArc()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create an OpenAir file with an arc
    const QString content = R"(
AC C
AN Test Arc Area
AH 3000FT MSL
AL GND
DP 47:00:00N 008:00:00E
V X=47:00:00N 008:30:00E
V D=+
DA 10,0,90
DP 47:00:00N 009:00:00E
)";

    const QString filePath = tmpDir.filePath("arc.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(content.toUtf8());
    file.close();

    OpenAirParser::ParseResult result = OpenAirParser::parseFile(filePath);
    QVERIFY(result.success);
    QCOMPARE(result.airspaces.count(), 1);
    QVERIFY(result.airspaces[0].boundary.count() >= 3);
}

void ShapeTest::_testOpenAirParseCoordinate()
{
    QGeoCoordinate coord;
    QString errorString;

    // Test DMS format
    QVERIFY(OpenAirParser::parseCoordinate(QStringLiteral("47:30:00N 008:30:00E"), coord));
    QCOMPARE(qRound(coord.latitude() * 100), 4750);
    QCOMPARE(qRound(coord.longitude() * 100), 850);

    // Test decimal format
    QVERIFY(OpenAirParser::parseCoordinate(QStringLiteral("47.5N 8.5E"), coord));
    QCOMPARE(qRound(coord.latitude() * 10), 475);
    QCOMPARE(qRound(coord.longitude() * 10), 85);

    // Test southern/western coordinates
    QVERIFY(OpenAirParser::parseCoordinate(QStringLiteral("33:45:00S 070:40:00W"), coord));
    QVERIFY(coord.latitude() < 0);
    QVERIFY(coord.longitude() < 0);
}

void ShapeTest::_testOpenAirParseAltitude()
{
    // Test various altitude formats
    OpenAirParser::Altitude alt;

    alt = OpenAirParser::parseAltitude(QStringLiteral("5000FT MSL"));
    QCOMPARE(alt.reference, OpenAirParser::AltitudeReference::MSL);
    QCOMPARE(qRound(alt.value), 5000);

    alt = OpenAirParser::parseAltitude(QStringLiteral("FL100"));
    QCOMPARE(alt.reference, OpenAirParser::AltitudeReference::FL);
    QCOMPARE(qRound(alt.value), 100);

    alt = OpenAirParser::parseAltitude(QStringLiteral("GND"));
    QCOMPARE(alt.reference, OpenAirParser::AltitudeReference::SFC);
    QCOMPARE(qRound(alt.value), 0);

    alt = OpenAirParser::parseAltitude(QStringLiteral("2500FT AGL"));
    QCOMPARE(alt.reference, OpenAirParser::AltitudeReference::AGL);
    QCOMPARE(qRound(alt.value), 2500);
}

// ============================================================================
// WKB (Well-Known Binary) Tests
// ============================================================================

void ShapeTest::_testWKBParsePoint()
{
    // Create a WKB point (little endian)
    QByteArray wkb;
    QDataStream stream(&wkb, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint8(1);  // Little endian
    stream << quint32(1); // wkbPoint
    stream << double(8.5);  // X (longitude)
    stream << double(47.5); // Y (latitude)

    QGeoCoordinate coord;
    QString errorString;
    QVERIFY(WKBHelper::parsePoint(wkb, coord, errorString));
    QCOMPARE(qRound(coord.longitude() * 10), 85);
    QCOMPARE(qRound(coord.latitude() * 10), 475);
}

void ShapeTest::_testWKBParseLineString()
{
    // Create a WKB linestring
    QByteArray wkb;
    QDataStream stream(&wkb, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint8(1);  // Little endian
    stream << quint32(2); // wkbLineString
    stream << quint32(3); // 3 points
    stream << double(8.0) << double(47.0);
    stream << double(8.5) << double(47.5);
    stream << double(9.0) << double(48.0);

    QList<QGeoCoordinate> coords;
    QString errorString;
    QVERIFY(WKBHelper::parseLineString(wkb, coords, errorString));
    QCOMPARE(coords.count(), 3);
}

void ShapeTest::_testWKBParsePolygon()
{
    // Create a WKB polygon (closed ring)
    QByteArray wkb;
    QDataStream stream(&wkb, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint8(1);  // Little endian
    stream << quint32(3); // wkbPolygon
    stream << quint32(1); // 1 ring
    stream << quint32(5); // 5 points (closed)
    stream << double(8.0) << double(47.0);
    stream << double(9.0) << double(47.0);
    stream << double(9.0) << double(48.0);
    stream << double(8.0) << double(48.0);
    stream << double(8.0) << double(47.0); // Closing point

    QList<QGeoCoordinate> coords;
    QString errorString;
    QVERIFY(WKBHelper::parsePolygon(wkb, coords, errorString));
    QVERIFY(coords.count() >= 4);
}

void ShapeTest::_testWKBGenerate()
{
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);

    QByteArray wkb = WKBHelper::toWKBPolygon(polygon, false);
    QVERIFY(!wkb.isEmpty());

    // Parse it back
    QList<QGeoCoordinate> parsed;
    QString errorString;
    QVERIFY(WKBHelper::parsePolygon(wkb, parsed, errorString));
    QCOMPARE(parsed.count(), polygon.count());
}

void ShapeTest::_testWKBRoundTrip()
{
    // Test point round-trip
    QGeoCoordinate originalPoint(47.5, 8.5, 500.0);
    QByteArray pointWkb = WKBHelper::toWKBPoint(originalPoint, true);

    QGeoCoordinate parsedPoint;
    QString errorString;
    QVERIFY(WKBHelper::parsePoint(pointWkb, parsedPoint, errorString));
    QCOMPARE(qRound(parsedPoint.latitude() * 100), qRound(originalPoint.latitude() * 100));
    QCOMPARE(qRound(parsedPoint.longitude() * 100), qRound(originalPoint.longitude() * 100));
}

// ============================================================================
// GeoPackage Tests
// ============================================================================

void ShapeTest::_testGeoPackageCreate()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString gpkgFile = tmpDir.filePath("test.gpkg");
    QString errorString;

    QVERIFY(GeoPackageHelper::createGeoPackage(gpkgFile, errorString));
    QVERIFY(QFile::exists(gpkgFile));
    QVERIFY(GeoPackageHelper::isValidGeoPackage(gpkgFile));
}

void ShapeTest::_testGeoPackageSavePoints()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString gpkgFile = tmpDir.filePath("points.gpkg");
    QString errorString;

    QList<QGeoCoordinate> points;
    points << QGeoCoordinate(47.0, 8.0, 100.0);
    points << QGeoCoordinate(47.5, 8.5, 200.0);
    points << QGeoCoordinate(48.0, 9.0, 300.0);

    QVERIFY(GeoPackageHelper::savePoints(gpkgFile, points, QStringLiteral("test_points"), errorString));
    QVERIFY(QFile::exists(gpkgFile));
}

void ShapeTest::_testGeoPackageSavePolygons()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString gpkgFile = tmpDir.filePath("polygons.gpkg");
    QString errorString;

    QList<QList<QGeoCoordinate>> polygons;
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);
    polygons.append(polygon);

    QVERIFY(GeoPackageHelper::savePolygons(gpkgFile, polygons, QStringLiteral("test_polygons"), errorString));
    QVERIFY(QFile::exists(gpkgFile));
}

void ShapeTest::_testGeoPackageLoadFeatures()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString gpkgFile = tmpDir.filePath("features.gpkg");
    QString errorString;

    // Create and save some polygons
    QList<QList<QGeoCoordinate>> polygons;
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);
    polygons.append(polygon);

    QVERIFY(GeoPackageHelper::savePolygons(gpkgFile, polygons, QStringLiteral("areas"), errorString));

    // Load them back
    GeoPackageHelper::LoadResult result = GeoPackageHelper::loadAllFeatures(gpkgFile);
    QVERIFY(result.success);
    QCOMPARE(result.polygons.count(), 1);
}

void ShapeTest::_testGeoPackageRoundTrip()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString gpkgFile = tmpDir.filePath("roundtrip.gpkg");
    QString errorString;

    // Create test data
    QList<QGeoCoordinate> originalPoints;
    originalPoints << QGeoCoordinate(47.1, 8.1);
    originalPoints << QGeoCoordinate(47.2, 8.2);

    QList<QList<QGeoCoordinate>> originalPolygons;
    QList<QGeoCoordinate> poly;
    poly << QGeoCoordinate(46.0, 7.0);
    poly << QGeoCoordinate(46.0, 8.0);
    poly << QGeoCoordinate(47.0, 8.0);
    poly << QGeoCoordinate(47.0, 7.0);
    originalPolygons.append(poly);

    // Save
    QVERIFY(GeoPackageHelper::savePoints(gpkgFile, originalPoints, QStringLiteral("waypoints"), errorString));
    QVERIFY(GeoPackageHelper::savePolygons(gpkgFile, originalPolygons, QStringLiteral("areas"), errorString));

    // Load and verify
    GeoPackageHelper::LoadResult result = GeoPackageHelper::loadAllFeatures(gpkgFile);
    QVERIFY(result.success);
    QCOMPARE(result.points.count(), originalPoints.count());
    QCOMPARE(result.polygons.count(), originalPolygons.count());
}

// ============================================================================
// GeoFormatRegistry Tests
// ============================================================================

void ShapeTest::_testGeoFormatRegistrySupportedFormats()
{
    QList<GeoFormatRegistry::FormatInfo> formats = GeoFormatRegistry::supportedFormats();
    QVERIFY(formats.count() >= 8); // At least KML, KMZ, GeoJSON, GPX, SHP, WKT, OpenAir, GeoPackage

    // Check that native formats are included
    QList<GeoFormatRegistry::FormatInfo> nativeFormats = GeoFormatRegistry::nativeFormats();
    QVERIFY(nativeFormats.count() >= 8);

    // Verify format info structure
    for (const auto& format : nativeFormats) {
        QVERIFY(!format.name.isEmpty());
        QVERIFY(!format.extensions.isEmpty());
        QVERIFY(format.isNative);
    }
}

void ShapeTest::_testGeoFormatRegistryLoadFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Create a test KML file
    const QString kmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
<Placemark>
<Polygon><outerBoundaryIs><LinearRing><coordinates>
8,47,0 9,47,0 9,48,0 8,48,0 8,47,0
</coordinates></LinearRing></outerBoundaryIs></Polygon>
</Placemark>
</Document>
</kml>)";

    const QString kmlFile = tmpDir.filePath("test.kml");
    QFile file(kmlFile);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write(kmlContent.toUtf8());
    file.close();

    // Load using registry
    GeoFormatRegistry::LoadResult result = GeoFormatRegistry::loadFile(kmlFile);
    QVERIFY(result.success);
    QCOMPARE(result.formatUsed, QStringLiteral("KML"));
    QVERIFY(result.totalFeatures() > 0);
}

void ShapeTest::_testGeoFormatRegistryFileFilters()
{
    QString readFilter = GeoFormatRegistry::readFileFilter();
    QVERIFY(!readFilter.isEmpty());
    QVERIFY(readFilter.contains(QStringLiteral("*.kml")));
    QVERIFY(readFilter.contains(QStringLiteral("*.geojson")));
    QVERIFY(readFilter.contains(QStringLiteral("*.gpx")));
    QVERIFY(readFilter.contains(QStringLiteral("*.shp")));

    QString writeFilter = GeoFormatRegistry::writeFileFilter();
    QVERIFY(!writeFilter.isEmpty());
}

void ShapeTest::_testGeoFormatRegistrySaveFunctions()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    // Test data
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);

    QList<QGeoCoordinate> polyline;
    polyline << QGeoCoordinate(47.0, 8.0);
    polyline << QGeoCoordinate(47.5, 8.5);
    polyline << QGeoCoordinate(48.0, 9.0);

    QList<QGeoCoordinate> points;
    points << QGeoCoordinate(47.1, 8.1);
    points << QGeoCoordinate(47.2, 8.2);

    // Test save to KML
    const QString kmlFile = tmpDir.filePath("registry_test.kml");
    GeoFormatRegistry::SaveResult kmlResult = GeoFormatRegistry::savePolygon(kmlFile, polygon);
    QVERIFY2(kmlResult.success, qPrintable(kmlResult.errorString));
    QVERIFY(QFile::exists(kmlFile));

    // Test save to GeoJSON
    const QString geojsonFile = tmpDir.filePath("registry_test.geojson");
    GeoFormatRegistry::SaveResult geojsonResult = GeoFormatRegistry::savePolygon(geojsonFile, polygon);
    QVERIFY2(geojsonResult.success, qPrintable(geojsonResult.errorString));
    QVERIFY(QFile::exists(geojsonFile));

    // Test save to GPX (polyline)
    const QString gpxFile = tmpDir.filePath("registry_test.gpx");
    GeoFormatRegistry::SaveResult gpxResult = GeoFormatRegistry::savePolyline(gpxFile, polyline);
    QVERIFY2(gpxResult.success, qPrintable(gpxResult.errorString));
    QVERIFY(QFile::exists(gpxFile));

    // Test save to WKT
    const QString wktFile = tmpDir.filePath("registry_test.wkt");
    GeoFormatRegistry::SaveResult wktResult = GeoFormatRegistry::savePolygon(wktFile, polygon);
    QVERIFY2(wktResult.success, qPrintable(wktResult.errorString));
    QVERIFY(QFile::exists(wktFile));

    // Test save points to WKT
    const QString wktPointsFile = tmpDir.filePath("registry_points.wkt");
    GeoFormatRegistry::SaveResult wktPointsResult = GeoFormatRegistry::savePoints(wktPointsFile, points);
    QVERIFY2(wktPointsResult.success, qPrintable(wktPointsResult.errorString));
    QVERIFY(QFile::exists(wktPointsFile));

    // Test save multiple polygons
    QList<QList<QGeoCoordinate>> polygons;
    polygons.append(polygon);
    const QString multiFile = tmpDir.filePath("registry_multi.geojson");
    GeoFormatRegistry::SaveResult multiResult = GeoFormatRegistry::savePolygons(multiFile, polygons);
    QVERIFY2(multiResult.success, qPrintable(multiResult.errorString));
}

void ShapeTest::_testGeoFormatRegistryCapabilityValidation()
{
    // Test that all registered formats have valid capability declarations
    QList<GeoFormatRegistry::ValidationResult> results = GeoFormatRegistry::validateCapabilities();

    // Verify validation returns results for all formats
    QVERIFY(results.count() > 0);

    // Check that all native formats are valid (no capability mismatches)
    bool allValid = GeoFormatRegistry::allCapabilitiesValid();
    if (!allValid) {
        for (const auto& result : results) {
            if (!result.valid) {
                qDebug() << "Format" << result.formatName << "has issues:" << result.issues;
            }
        }
    }
    QVERIFY2(allValid, "Some formats have capability mismatches");
}

// ============================================================================
// WKT (Well-Known Text) Tests
// ============================================================================

QString ShapeTest::_writeWktFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content)
{
    const QString path = tmpDir.filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
    }
    return path;
}

void ShapeTest::_testWKTParsePoint()
{
    QGeoCoordinate coord;
    QString errorString;

    // Test basic POINT
    QVERIFY(WKTHelper::parsePoint(QStringLiteral("POINT (8.5 47.5)"), coord, errorString));
    QCOMPARE(qRound(coord.longitude() * 10), 85);
    QCOMPARE(qRound(coord.latitude() * 10), 475);

    // Test POINT Z (with altitude)
    QVERIFY(WKTHelper::parsePoint(QStringLiteral("POINT Z (8.5 47.5 100)"), coord, errorString));
    QCOMPARE(qRound(coord.longitude() * 10), 85);
    QCOMPARE(qRound(coord.latitude() * 10), 475);
    QCOMPARE(qRound(coord.altitude()), 100);

    // Test invalid point
    QVERIFY(!WKTHelper::parsePoint(QStringLiteral("POINT ()"), coord, errorString));
}

void ShapeTest::_testWKTParseLineString()
{
    QList<QGeoCoordinate> coords;
    QString errorString;

    // Test basic LINESTRING
    QVERIFY(WKTHelper::parseLineString(
        QStringLiteral("LINESTRING (8 47, 8.5 47.5, 9 48)"), coords, errorString));
    QCOMPARE(coords.count(), 3);
    QCOMPARE(qRound(coords[0].longitude()), 8);
    QCOMPARE(qRound(coords[0].latitude()), 47);
    QCOMPARE(qRound(coords[2].longitude()), 9);
    QCOMPARE(qRound(coords[2].latitude()), 48);

    // Test LINESTRING Z
    coords.clear();
    QVERIFY(WKTHelper::parseLineString(
        QStringLiteral("LINESTRING Z (8 47 100, 9 48 200)"), coords, errorString));
    QCOMPARE(coords.count(), 2);
    QCOMPARE(qRound(coords[0].altitude()), 100);
    QCOMPARE(qRound(coords[1].altitude()), 200);
}

void ShapeTest::_testWKTParsePolygon()
{
    QList<QGeoCoordinate> vertices;
    QString errorString;

    // Test basic POLYGON (ring is auto-closed in WKT)
    QVERIFY(WKTHelper::parsePolygon(
        QStringLiteral("POLYGON ((8 47, 9 47, 9 48, 8 48, 8 47))"), vertices, errorString));
    QVERIFY(vertices.count() >= 4);

    // Test POLYGON with holes (outer ring should be extracted)
    vertices.clear();
    QVERIFY(WKTHelper::parsePolygon(
        QStringLiteral("POLYGON ((8 47, 9 47, 9 48, 8 48, 8 47), (8.2 47.2, 8.8 47.2, 8.8 47.8, 8.2 47.8, 8.2 47.2))"),
        vertices, errorString));
    QVERIFY(vertices.count() >= 4);
}

void ShapeTest::_testWKTGenerate()
{
    // Test POINT generation
    QGeoCoordinate point(47.5, 8.5, 100.0);
    QString pointWkt = WKTHelper::toWKTPoint(point, false);
    QVERIFY(pointWkt.startsWith(QStringLiteral("POINT")));
    QVERIFY(pointWkt.contains(QStringLiteral("8.5")));
    QVERIFY(pointWkt.contains(QStringLiteral("47.5")));

    // Test POINT Z generation
    QString pointZWkt = WKTHelper::toWKTPoint(point, true);
    QVERIFY(pointZWkt.startsWith(QStringLiteral("POINT Z")));
    QVERIFY(pointZWkt.contains(QStringLiteral("100")));

    // Test LINESTRING generation
    QList<QGeoCoordinate> line;
    line << QGeoCoordinate(47.0, 8.0);
    line << QGeoCoordinate(47.5, 8.5);
    line << QGeoCoordinate(48.0, 9.0);
    QString lineWkt = WKTHelper::toWKTLineString(line, false);
    QVERIFY(lineWkt.startsWith(QStringLiteral("LINESTRING")));

    // Test POLYGON generation
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);
    QString polygonWkt = WKTHelper::toWKTPolygon(polygon, false);
    QVERIFY(polygonWkt.startsWith(QStringLiteral("POLYGON")));
}

void ShapeTest::_testWKTRoundTrip()
{
    QString errorString;

    // Point round-trip
    QGeoCoordinate originalPoint(47.5, 8.5, 100.0);
    QString pointWkt = WKTHelper::toWKTPoint(originalPoint, true);
    QGeoCoordinate parsedPoint;
    QVERIFY(WKTHelper::parsePoint(pointWkt, parsedPoint, errorString));
    QCOMPARE(qRound(parsedPoint.latitude() * 100), qRound(originalPoint.latitude() * 100));
    QCOMPARE(qRound(parsedPoint.longitude() * 100), qRound(originalPoint.longitude() * 100));

    // LineString round-trip
    QList<QGeoCoordinate> originalLine;
    originalLine << QGeoCoordinate(47.0, 8.0);
    originalLine << QGeoCoordinate(47.5, 8.5);
    originalLine << QGeoCoordinate(48.0, 9.0);
    QString lineWkt = WKTHelper::toWKTLineString(originalLine, false);
    QList<QGeoCoordinate> parsedLine;
    QVERIFY(WKTHelper::parseLineString(lineWkt, parsedLine, errorString));
    QCOMPARE(parsedLine.count(), originalLine.count());

    // Polygon round-trip
    QList<QGeoCoordinate> originalPolygon;
    originalPolygon << QGeoCoordinate(47.0, 8.0);
    originalPolygon << QGeoCoordinate(47.0, 9.0);
    originalPolygon << QGeoCoordinate(48.0, 9.0);
    originalPolygon << QGeoCoordinate(48.0, 8.0);
    QString polygonWkt = WKTHelper::toWKTPolygon(originalPolygon, false);
    QList<QGeoCoordinate> parsedPolygon;
    QVERIFY(WKTHelper::parsePolygon(polygonWkt, parsedPolygon, errorString));
    QCOMPARE(parsedPolygon.count(), originalPolygon.count());
}

void ShapeTest::_testWKTLoadFromFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString errorString;

    // Test loading point from file
    const QString pointWkt = _writeWktFile(tmpDir, "point.wkt", "POINT (8.5 47.5)");
    QGeoCoordinate point;
    QVERIFY(WKTHelper::loadPointFromFile(pointWkt, point, errorString));
    QCOMPARE(qRound(point.longitude() * 10), 85);
    QCOMPARE(qRound(point.latitude() * 10), 475);

    // Test loading points from MULTIPOINT file
    const QString multiPointWkt = _writeWktFile(tmpDir, "multipoint.wkt",
        "MULTIPOINT ((8 47), (8.5 47.5), (9 48))");
    QList<QGeoCoordinate> points;
    QVERIFY2(WKTHelper::loadPointsFromFile(multiPointWkt, points, errorString),
             qPrintable(QStringLiteral("loadPointsFromFile failed: %1").arg(errorString)));
    QCOMPARE(points.count(), 3);

    // Test loading polyline from file
    const QString lineWkt = _writeWktFile(tmpDir, "line.wkt",
        "LINESTRING (8 47, 8.5 47.5, 9 48)");
    QList<QGeoCoordinate> polyline;
    QVERIFY(WKTHelper::loadPolylineFromFile(lineWkt, polyline, errorString));
    QCOMPARE(polyline.count(), 3);

    // Test loading polygon from file
    const QString polygonWkt = _writeWktFile(tmpDir, "polygon.wkt",
        "POLYGON ((8 47, 9 47, 9 48, 8 48, 8 47))");
    QList<QGeoCoordinate> polygon;
    QVERIFY(WKTHelper::loadPolygonFromFile(polygonWkt, polygon, errorString));
    QVERIFY(polygon.count() >= 4);

    // Test loading multiple polygons from MULTIPOLYGON file
    const QString multiPolygonWkt = _writeWktFile(tmpDir, "multipolygon.wkt",
        "MULTIPOLYGON (((8 47, 9 47, 9 48, 8 48, 8 47)), ((10 50, 11 50, 11 51, 10 51, 10 50)))");
    QList<QList<QGeoCoordinate>> polygons;
    QVERIFY(WKTHelper::loadPolygonsFromFile(multiPolygonWkt, polygons, errorString));
    QCOMPARE(polygons.count(), 2);
}

void ShapeTest::_testWKTSaveToFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QString errorString;

    // Test saving point to file
    QGeoCoordinate point(47.5, 8.5, 100.0);
    const QString pointFile = tmpDir.filePath("save_point.wkt");
    QVERIFY(WKTHelper::savePointToFile(pointFile, point, errorString));
    QVERIFY(QFile::exists(pointFile));

    // Verify by loading back
    QGeoCoordinate loadedPoint;
    QVERIFY(WKTHelper::loadPointFromFile(pointFile, loadedPoint, errorString));
    QCOMPARE(qRound(loadedPoint.latitude() * 10), qRound(point.latitude() * 10));

    // Test saving points to file
    QList<QGeoCoordinate> points;
    points << QGeoCoordinate(47.1, 8.1);
    points << QGeoCoordinate(47.2, 8.2);
    const QString pointsFile = tmpDir.filePath("save_points.wkt");
    QVERIFY(WKTHelper::savePointsToFile(pointsFile, points, errorString));
    QVERIFY(QFile::exists(pointsFile));

    // Test saving polyline to file
    QList<QGeoCoordinate> polyline;
    polyline << QGeoCoordinate(47.0, 8.0);
    polyline << QGeoCoordinate(47.5, 8.5);
    polyline << QGeoCoordinate(48.0, 9.0);
    const QString polylineFile = tmpDir.filePath("save_polyline.wkt");
    QVERIFY(WKTHelper::savePolylineToFile(polylineFile, polyline, errorString));
    QVERIFY(QFile::exists(polylineFile));

    // Verify by loading back
    QList<QGeoCoordinate> loadedPolyline;
    QVERIFY(WKTHelper::loadPolylineFromFile(polylineFile, loadedPolyline, errorString));
    QCOMPARE(loadedPolyline.count(), polyline.count());

    // Test saving polygon to file
    QList<QGeoCoordinate> polygon;
    polygon << QGeoCoordinate(47.0, 8.0);
    polygon << QGeoCoordinate(47.0, 9.0);
    polygon << QGeoCoordinate(48.0, 9.0);
    polygon << QGeoCoordinate(48.0, 8.0);
    const QString polygonFile = tmpDir.filePath("save_polygon.wkt");
    QVERIFY(WKTHelper::savePolygonToFile(polygonFile, polygon, errorString));
    QVERIFY(QFile::exists(polygonFile));

    // Verify by loading back
    QList<QGeoCoordinate> loadedPolygon;
    QVERIFY(WKTHelper::loadPolygonFromFile(polygonFile, loadedPolygon, errorString));
    QCOMPARE(loadedPolygon.count(), polygon.count());

    // Test saving multiple polygons to file
    QList<QList<QGeoCoordinate>> polygons;
    polygons.append(polygon);
    const QString polygonsFile = tmpDir.filePath("save_polygons.wkt");
    QVERIFY(WKTHelper::savePolygonsToFile(polygonsFile, polygons, errorString));
    QVERIFY(QFile::exists(polygonsFile));

    // Verify by loading back
    QList<QList<QGeoCoordinate>> loadedPolygons;
    QVERIFY(WKTHelper::loadPolygonsFromFile(polygonsFile, loadedPolygons, errorString));
    QCOMPARE(loadedPolygons.count(), polygons.count());
}
