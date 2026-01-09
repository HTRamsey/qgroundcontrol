#pragma once

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(GeoPackageHelperLog)

/// @file GeoPackageHelper.h
/// @brief GeoPackage (.gpkg) file support using Qt's SQLite
///
/// GeoPackage is an OGC standard SQLite-based format for geospatial data.
/// This implementation uses Qt's built-in SQLite support, avoiding GDAL dependency.
///
/// Features supported:
/// - Reading feature tables (points, lines, polygons)
/// - Writing feature tables
/// - Spatial reference system handling (WGS84 default)
/// - GeoPackage Binary geometry encoding
///
/// @see http://www.geopackage.org/spec/
/// @see WKBHelper for geometry encoding

namespace GeoPackageHelper
{
    /// Information about a feature table
    struct TableInfo {
        QString tableName;
        QString geometryColumn;
        QString geometryType;   ///< POINT, LINESTRING, POLYGON, etc.
        int srid = 4326;        ///< Spatial Reference ID
        int featureCount = 0;
    };

    /// Result of loading features
    struct LoadResult {
        bool success = false;
        QString errorString;

        QList<QGeoCoordinate> points;
        QList<QList<QGeoCoordinate>> polylines;
        QList<QList<QGeoCoordinate>> polygons;

        int totalFeatures() const {
            return points.count() + polylines.count() + polygons.count();
        }
    };

    // ========================================================================
    // Reading Functions
    // ========================================================================

    /// Check if a file is a valid GeoPackage
    /// @param filePath Path to file
    /// @return true if valid GeoPackage
    bool isValidGeoPackage(const QString &filePath);

    /// List all feature tables in a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param[out] tables List of table info
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool listTables(const QString &filePath, QList<TableInfo> &tables, QString &errorString);

    /// Load all features from a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @return Load result with geometries or error
    LoadResult loadAllFeatures(const QString &filePath);

    /// Load features from a specific table
    /// @param filePath Path to GeoPackage file
    /// @param tableName Name of feature table
    /// @return Load result with geometries or error
    LoadResult loadTable(const QString &filePath, const QString &tableName);

    /// Load points from a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param[out] points Loaded points
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool loadPoints(const QString &filePath, QList<QGeoCoordinate> &points, QString &errorString);

    /// Load polylines from a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param[out] polylines Loaded polylines
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool loadPolylines(const QString &filePath, QList<QList<QGeoCoordinate>> &polylines, QString &errorString);

    /// Load polygons from a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param[out] polygons Loaded polygons
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool loadPolygons(const QString &filePath, QList<QList<QGeoCoordinate>> &polygons, QString &errorString);

    // ========================================================================
    // Writing Functions
    // ========================================================================

    /// Create a new GeoPackage file with standard tables
    /// @param filePath Path for new file
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool createGeoPackage(const QString &filePath, QString &errorString);

    /// Save points to a GeoPackage
    /// @param filePath Path to GeoPackage file (created if doesn't exist)
    /// @param points Points to save
    /// @param tableName Table name (default: "points")
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool savePoints(const QString &filePath, const QList<QGeoCoordinate> &points,
                    const QString &tableName, QString &errorString);

    /// Save polylines to a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param polylines Polylines to save
    /// @param tableName Table name (default: "lines")
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool savePolylines(const QString &filePath, const QList<QList<QGeoCoordinate>> &polylines,
                       const QString &tableName, QString &errorString);

    /// Save polygons to a GeoPackage
    /// @param filePath Path to GeoPackage file
    /// @param polygons Polygons to save
    /// @param tableName Table name (default: "polygons")
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool savePolygons(const QString &filePath, const QList<QList<QGeoCoordinate>> &polygons,
                      const QString &tableName, QString &errorString);

    // ========================================================================
    // Utility Functions
    // ========================================================================

    /// Get GeoPackage application ID (should be "GPKG")
    /// @param filePath Path to file
    /// @return Application ID string, or empty if not a GeoPackage
    QString getApplicationId(const QString &filePath);

    /// Get GeoPackage version
    /// @param filePath Path to file
    /// @return Version number (e.g., 10201 for 1.2.1)
    int getVersion(const QString &filePath);
}
