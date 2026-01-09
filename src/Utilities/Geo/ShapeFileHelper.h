#pragma once

#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPolygon>
#include <QtQmlIntegration/QtQmlIntegration>

Q_DECLARE_LOGGING_CATEGORY(ShapeFileHelperLog)

/// @file ShapeFileHelper.h
/// @brief Unified API for loading and saving geographic shapes (polygons, polylines, points)
///
/// ShapeFileHelper provides format-agnostic I/O for geographic data, automatically selecting
/// the appropriate helper based on file extension:
///
/// | Format  | Extension | Load | Save | Polygons | Polylines | Points | Holes | Altitude |
/// |---------|-----------|------|------|----------|-----------|--------|-------|----------|
/// | KML     | .kml      | ✓    | ✓    | ✓        | ✓         | ✓      | ✓     | ✓        |
/// | SHP     | .shp      | ✓    | ✓    | ✓        | ✓         | ✓      | ✓     | ✓        |
/// | GeoJSON | .geojson  | ✓    | ✓    | ✓        | ✓         | ✓      | ✓     | ✓        |
/// | GPX     | .gpx      | ✓    | ✓    | ✓        | ✓         | ✓      | ✗     | ✓        |
///
/// @section usage Basic Usage
/// @code
/// QList<QGeoCoordinate> vertices;
/// QString error;
/// // Load from any supported format
/// if (!ShapeFileHelper::loadPolygonFromFile("survey.kml", vertices, error)) {
///     qWarning() << error;
/// }
/// // Save to any supported format (format determined by extension)
/// if (!ShapeFileHelper::savePolygonToFile("survey.shp", vertices, error)) {
///     qWarning() << error;
/// }
/// @endcode
///
/// @section filtering Vertex Filtering
/// Load functions support automatic filtering of vertices closer than a threshold distance.
/// This helps simplify overly dense shapes from GPS tracks or detailed surveys:
/// @code
/// // Filter vertices closer than 10 meters apart
/// ShapeFileHelper::loadPolygonFromFile("track.gpx", vertices, error, 10.0);
/// // Disable filtering (0 = keep all vertices)
/// ShapeFileHelper::loadPolygonFromFile("precise.kml", vertices, error, 0.0);
/// @endcode
///
/// @section validation Pre-save Validation
/// All save functions validate coordinates before writing:
/// - Latitude must be in [-90, 90]
/// - Longitude must be in [-180, 180]
/// - Invalid coordinates cause save to fail with descriptive error
///
/// @see KMLHelper for KML-specific operations and schema validation
/// @see SHPFileHelper for Shapefile-specific operations
/// @see GeoJsonHelper for GeoJSON-specific operations
/// @see GPXHelper for GPX-specific operations
/// @see GeoUtilities for coordinate validation and normalization
class ShapeFileHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList fileDialogKMLFilters     READ fileDialogKMLFilters     CONSTANT) ///< File filter list for KML file dialogs
    Q_PROPERTY(QStringList fileDialogFilters        READ fileDialogFilters        CONSTANT) ///< File filter list for shape file dialogs (KML, SHP, GeoJSON, GPX)

public:
    static QStringList fileDialogKMLFilters();
    static QStringList fileDialogFilters();

    enum class ShapeType {
        Polygon,
        Polyline,
        Point,
        Error
    };

    /// Features that may or may not be supported by a file format
    enum class Feature {
        Polygons,           ///< Basic polygon support
        PolygonsWithHoles,  ///< Polygons with interior holes
        Polylines,          ///< Basic polyline/path support
        Points,             ///< Point/waypoint support
        Tracks,             ///< GPX-style tracks (ordered with timestamps)
        Altitude,           ///< 3D coordinates with altitude
        MultipleEntities    ///< Multiple geometries in one file
    };

    /// Check if a file format supports a specific feature
    /// @param file File path (used to determine format from extension)
    /// @param feature The feature to check
    /// @return true if the format supports the feature
    static bool supportsFeature(const QString &file, Feature feature);

    /// Check if a file format supports a specific feature (by extension)
    /// @param extension File extension including dot (e.g., ".kml")
    /// @param feature The feature to check
    /// @return true if the format supports the feature
    static bool supportsFeatureByExtension(const QString &extension, Feature feature);

    /// Default distance threshold for filtering nearby vertices (meters)
    static constexpr double kDefaultVertexFilterMeters = 5.0;

    static ShapeType determineShapeType(const QString &file, QString &errorString);

    /// Get the number of geometry entities in the file
    static int getEntityCount(const QString &file, QString &errorString);

    /// Load first polygon entity (convenience wrapper)
    /// @param filterMeters Filter vertices closer than this distance (0 to disable)
    static bool loadPolygonFromFile(const QString &file, QList<QGeoCoordinate> &vertices, QString &errorString,
                                    double filterMeters = kDefaultVertexFilterMeters);

    /// Load all polygon entities
    /// @param filterMeters Filter vertices closer than this distance (0 to disable)
    static bool loadPolygonsFromFile(const QString &file, QList<QList<QGeoCoordinate>> &polygons, QString &errorString,
                                     double filterMeters = kDefaultVertexFilterMeters);

    /// Load first polygon with holes (QGeoPolygon preserves hole information)
    static bool loadPolygonWithHolesFromFile(const QString &file, QGeoPolygon &polygon, QString &errorString);

    /// Load all polygons with holes
    static bool loadPolygonsWithHolesFromFile(const QString &file, QList<QGeoPolygon> &polygons, QString &errorString);

    /// Load first polyline entity (convenience wrapper)
    /// @param filterMeters Filter vertices closer than this distance (0 to disable)
    static bool loadPolylineFromFile(const QString &file, QList<QGeoCoordinate> &coords, QString &errorString,
                                     double filterMeters = kDefaultVertexFilterMeters);

    /// Load all polyline entities
    /// @param filterMeters Filter vertices closer than this distance (0 to disable)
    static bool loadPolylinesFromFile(const QString &file, QList<QList<QGeoCoordinate>> &polylines, QString &errorString,
                                      double filterMeters = kDefaultVertexFilterMeters);

    /// Load point entities
    static bool loadPointsFromFile(const QString &file, QList<QGeoCoordinate> &points, QString &errorString);

    // ========================================================================
    // Save functions
    // ========================================================================

    /// Save a single polygon to file
    static bool savePolygonToFile(const QString &file, const QList<QGeoCoordinate> &vertices, QString &errorString);

    /// Save multiple polygons to file
    static bool savePolygonsToFile(const QString &file, const QList<QList<QGeoCoordinate>> &polygons, QString &errorString);

    /// Save a polygon with holes to file
    static bool savePolygonWithHolesToFile(const QString &file, const QGeoPolygon &polygon, QString &errorString);

    /// Save multiple polygons with holes to file
    static bool savePolygonsWithHolesToFile(const QString &file, const QList<QGeoPolygon> &polygons, QString &errorString);

    /// Save a single polyline to file
    static bool savePolylineToFile(const QString &file, const QList<QGeoCoordinate> &coords, QString &errorString);

    /// Save multiple polylines to file
    static bool savePolylinesToFile(const QString &file, const QList<QList<QGeoCoordinate>> &polylines, QString &errorString);

    /// Save points to file
    static bool savePointsToFile(const QString &file, const QList<QGeoCoordinate> &points, QString &errorString);

    /// Save a track to GPX file (GPX-specific: tracks are ordered point sequences with timestamps)
    /// Note: Only supported for GPX format. Other formats use savePolylineToFile.
    static bool saveTrackToFile(const QString &file, const QList<QGeoCoordinate> &coords, QString &errorString);

    /// Save multiple tracks to GPX file
    /// Note: Only supported for GPX format. Other formats use savePolylinesToFile.
    static bool saveTracksToFile(const QString &file, const QList<QList<QGeoCoordinate>> &tracks, QString &errorString);

    // ========================================================================
    // Utilities
    // ========================================================================

    /// Filter vertices that are closer than filterMeters apart
    /// @param vertices Coordinate list to filter in-place
    /// @param filterMeters Remove vertices closer than this distance (0 to disable)
    /// @param minVertices Minimum number of vertices to keep (e.g., 3 for polygon, 2 for polyline)
    static void filterVertices(QList<QGeoCoordinate> &vertices, double filterMeters, int minVertices);

    static constexpr const char *kmlFileExtension = ".kml";
    static constexpr const char *shpFileExtension = ".shp";
    static constexpr const char *geojsonFileExtension = ".geojson";
    static constexpr const char *gpxFileExtension = ".gpx";

private:
    enum class ShapeFileType {
        None,
        KML,
        SHP,
        GeoJson,
        GPX
    };
    static ShapeFileType _getShapeFileType(const QString &file, QString &errorString);
};
