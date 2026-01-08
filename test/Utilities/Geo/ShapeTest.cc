#include "ShapeTest.h"
#include "GeoJsonHelper.h"
#include "GPXHelper.h"
#include "KMLHelper.h"
#include "ShapeFileHelper.h"
#include "SHPFileHelper.h"
#include "KMLDomDocument.h"
#include "KMLSchemaValidator.h"

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

    // Expect warnings for invalid coordinates
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Latitude out of range.*91"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Longitude out of range.*200"));

    QString errorString;
    QList<QGeoCoordinate> coords;
    // Should succeed but skip invalid coordinates
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(kmlFile, coords, errorString, 0));
    QVERIFY(errorString.isEmpty());

    // Should have 3 valid coordinates (the two invalid ones skipped, plus duplicate closing removed)
    // Valid: -122,37 | -121,38 | -121,37
    // Invalid: -122,91 (lat>90) | 200,38 (lon>180)
    QCOMPARE(coords.count(), 3);

    // Verify the valid coordinates are present
    QCOMPARE(coords[0].latitude(), 37.0);
    QCOMPARE(coords[0].longitude(), -122.0);
    QCOMPARE(coords[1].latitude(), 38.0);
    QCOMPARE(coords[1].longitude(), -121.0);
    QCOMPARE(coords[2].latitude(), 37.0);
    QCOMPARE(coords[2].longitude(), -121.0);
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
