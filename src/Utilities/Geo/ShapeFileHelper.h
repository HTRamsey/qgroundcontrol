#pragma once

#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPolygon>
#include <QtQmlIntegration/QtQmlIntegration>

Q_DECLARE_LOGGING_CATEGORY(ShapeFileHelperLog)

/// Routines for loading polygons or polylines from KML, SHP, GeoJSON, or GPX files.
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
