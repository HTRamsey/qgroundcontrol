#pragma once

#include "UnitTest.h"

class QTemporaryDir;

class ShapeTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testLoadPolylineFromSHP();
    void _testLoadPolylineFromKML();
    void _testLoadPolygonFromSHP();
    void _testLoadPolygonFromKML();
    void _testLoadPointsFromSHP();
    void _testLoadPointsFromKML();
    void _testLoadPolygonsFromSHP();
    void _testLoadPolylinesFromSHP();
    void _testLoadPolygonsFromKML();
    void _testLoadPolylinesFromKML();
    void _testGetEntityCount();
    void _testDetermineShapeType();
    void _testUnsupportedProjectionError();
    void _testLoadFromQtResource();
    void _testVertexFiltering();
    void _testKMLVertexFiltering();
    void _testKMLAltitudeParsing();
    void _testKMLCoordinateValidation();
    void _testKMLExportSchemaValidation();

    // GeoJSON tests
    void _testLoadPolygonFromGeoJSON();
    void _testLoadPolylineFromGeoJSON();
    void _testLoadPolygonsFromGeoJSON();
    void _testLoadPolylinesFromGeoJSON();
    void _testLoadPointsFromGeoJSON();
    void _testGeoJSONVertexFiltering();
    void _testGeoJSONAltitudeParsing();
    void _testGeoJSONSaveFunctions();
    void _testGeoJSONDetermineShapeType();

    // SHP save tests
    void _testSHPSaveFunctions();

    // KML save tests
    void _testKMLSaveFunctions();

    // GPX tests
    void _testLoadPolylineFromGPX();
    void _testLoadPolylinesFromGPX();
    void _testLoadPolygonFromGPX();
    void _testLoadPointsFromGPX();
    void _testGPXSaveFunctions();
    void _testGPXTrackParsing();
    void _testGPXAltitudeParsing();

private:
    static QString _copyRes(const QTemporaryDir &tmpDir, const QString &name);
    static void _writePrjFile(const QString &path, const QString &content);
    static QString _writeKmlFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content);
    static QString _writeGeoJsonFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content);
    static QString _writeGpxFile(const QTemporaryDir &tmpDir, const QString &name, const QString &content);
};
