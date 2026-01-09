#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(GeoFormatRegistryLog)

/// @file GeoFormatRegistry.h
/// @brief Unified registry for geographic file format support
///
/// GeoFormatRegistry provides a unified interface for loading geographic data
/// from various file formats. It uses native parsers where available and
/// optionally delegates to GDAL for exotic formats.
///
/// Native format support (no external dependencies):
/// - KML/KMZ (Keyhole Markup Language)
/// - GeoJSON
/// - GPX (GPS Exchange Format)
/// - Shapefile (SHP)
/// - WKT (Well-Known Text)
/// - CSV with coordinates
/// - OpenAir (Airspace)
/// - GeoPackage (GPKG)
///
/// Optional GDAL support (runtime detection):
/// - GML, AIXM, and dozens of other formats
///
/// @see ShapeFileHelper, KMLHelper, GPXHelper, GeoJsonHelper, etc.

namespace GeoFormatRegistry
{
    /// Format capabilities
    enum Capability {
        CanReadPoints = 0x01,
        CanReadPolylines = 0x02,
        CanReadPolygons = 0x04,
        CanWritePoints = 0x08,
        CanWritePolylines = 0x10,
        CanWritePolygons = 0x20,
        CanReadAirspace = 0x40,

        CanRead = CanReadPoints | CanReadPolylines | CanReadPolygons,
        CanWrite = CanWritePoints | CanWritePolylines | CanWritePolygons,
        CanReadWrite = CanRead | CanWrite
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    /// Information about a supported format
    struct FormatInfo {
        QString name;             ///< Human-readable name
        QString description;      ///< Brief description
        QStringList extensions;   ///< File extensions (without dot)
        Capabilities capabilities;
        bool isNative = true;     ///< True if handled by native code, false if GDAL
    };

    /// Result of loading geographic data
    struct LoadResult {
        bool success = false;
        QString errorString;
        QString formatUsed;

        QList<QGeoCoordinate> points;
        QList<QList<QGeoCoordinate>> polylines;
        QList<QList<QGeoCoordinate>> polygons;

        int totalFeatures() const {
            return points.count() + polylines.count() + polygons.count();
        }
    };

    // ========================================================================
    // Registry Information
    // ========================================================================

    /// Get list of all supported formats
    QList<FormatInfo> supportedFormats();

    /// Get list of native (non-GDAL) formats
    QList<FormatInfo> nativeFormats();

    /// Check if a format is supported
    /// @param extension File extension (without dot)
    /// @return true if format is supported
    bool isSupported(const QString &extension);

    /// Get format info for an extension
    /// @param extension File extension
    /// @return Format info, or invalid info if not supported
    FormatInfo formatInfo(const QString &extension);

    /// Get file filter string for all readable formats
    /// @return Filter string for file dialogs (e.g., "All Geo Files (*.kml *.shp ...)")
    QString readFileFilter();

    /// Get file filter string for all writable formats
    QString writeFileFilter();

    // ========================================================================
    // Loading Functions
    // ========================================================================

    /// Load geographic data from any supported format
    /// @param filePath Path to file
    /// @return Load result with data or error
    LoadResult loadFile(const QString &filePath);

    /// Load points from any supported format
    /// @param filePath Path to file
    /// @param[out] points Loaded points
    /// @param[out] errorString Error description if operation fails
    /// @return true on success
    bool loadPoints(const QString &filePath, QList<QGeoCoordinate> &points, QString &errorString);

    /// Load polylines from any supported format
    bool loadPolylines(const QString &filePath, QList<QList<QGeoCoordinate>> &polylines, QString &errorString);

    /// Load polygons from any supported format
    bool loadPolygons(const QString &filePath, QList<QList<QGeoCoordinate>> &polygons, QString &errorString);

    // ========================================================================
    // Save Functions
    // ========================================================================

    /// Result of saving geographic data
    struct SaveResult {
        bool success = false;
        QString errorString;
        QString formatUsed;
    };

    /// Save a point to a file (format determined by extension)
    /// @param filePath Output file path
    /// @param point Point to save
    /// @return Save result
    SaveResult savePoint(const QString &filePath, const QGeoCoordinate &point);

    /// Save multiple points to a file
    /// @param filePath Output file path
    /// @param points Points to save
    /// @return Save result
    SaveResult savePoints(const QString &filePath, const QList<QGeoCoordinate> &points);

    /// Save a polyline to a file
    /// @param filePath Output file path
    /// @param polyline Polyline coordinates
    /// @return Save result
    SaveResult savePolyline(const QString &filePath, const QList<QGeoCoordinate> &polyline);

    /// Save multiple polylines to a file
    /// @param filePath Output file path
    /// @param polylines List of polylines
    /// @return Save result
    SaveResult savePolylines(const QString &filePath, const QList<QList<QGeoCoordinate>> &polylines);

    /// Save a polygon to a file
    /// @param filePath Output file path
    /// @param polygon Polygon vertices
    /// @return Save result
    SaveResult savePolygon(const QString &filePath, const QList<QGeoCoordinate> &polygon);

    /// Save multiple polygons to a file
    /// @param filePath Output file path
    /// @param polygons List of polygon vertex lists
    /// @return Save result
    SaveResult savePolygons(const QString &filePath, const QList<QList<QGeoCoordinate>> &polygons);

    // ========================================================================
    // Validation
    // ========================================================================

    /// Validation result for a single format
    struct ValidationResult {
        QString formatName;
        bool valid = true;
        QStringList issues;
    };

    /// Validate that all registered formats implement their declared capabilities
    /// @return List of validation results, one per format
    QList<ValidationResult> validateCapabilities();

    /// Check if all formats pass validation
    /// @return true if all formats are valid
    bool allCapabilitiesValid();

    // ========================================================================
    // GDAL Integration
    // ========================================================================

    /// Check if GDAL is available at runtime
    /// @return true if GDAL/OGR can be used
    bool isGDALAvailable();

    /// Get GDAL version string (empty if not available)
    QString gdalVersion();

    /// Load file using GDAL (if available)
    /// @param filePath Path to file
    /// @return Load result
    LoadResult loadWithGDAL(const QString &filePath);

    /// Get additional formats available via GDAL
    QStringList gdalFormats();
}

Q_DECLARE_OPERATORS_FOR_FLAGS(GeoFormatRegistry::Capabilities)
