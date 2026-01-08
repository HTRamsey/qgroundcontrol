#include "ShapeFileHelper.h"
#include "GeoJsonHelper.h"
#include "GPXHelper.h"
#include "KMLHelper.h"
#include "SHPFileHelper.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ShapeFileHelperLog, "Utilities.ShapeFileHelper")

namespace {
    constexpr const char *_errorPrefix = QT_TR_NOOP("Shape file load failed. %1");
    constexpr const char *_saveErrorPrefix = QT_TR_NOOP("Shape file save failed. %1");
}

ShapeFileHelper::ShapeFileType ShapeFileHelper::_getShapeFileType(const QString &file, QString &errorString)
{
    errorString.clear();

    if (file.endsWith(kmlFileExtension, Qt::CaseInsensitive)) {
        return ShapeFileType::KML;
    } else if (file.endsWith(shpFileExtension, Qt::CaseInsensitive)) {
        return ShapeFileType::SHP;
    } else if (file.endsWith(geojsonFileExtension, Qt::CaseInsensitive)) {
        return ShapeFileType::GeoJson;
    } else if (file.endsWith(gpxFileExtension, Qt::CaseInsensitive)) {
        return ShapeFileType::GPX;
    } else {
        errorString = QString(_errorPrefix).arg(tr("Unsupported file type. Only %1, %2, %3, and %4 are supported.")
            .arg(kmlFileExtension, shpFileExtension, geojsonFileExtension, gpxFileExtension));
    }

    return ShapeFileType::None;
}

void ShapeFileHelper::filterVertices(QList<QGeoCoordinate> &vertices, double filterMeters, int minVertices)
{
    if (filterMeters <= 0 || vertices.count() <= minVertices) {
        return;
    }

    int i = 0;
    while (i < (vertices.count() - 1)) {
        if ((vertices.count() > minVertices) && (vertices[i].distanceTo(vertices[i + 1]) < filterMeters)) {
            vertices.removeAt(i + 1);
        } else {
            i++;
        }
    }
}

ShapeFileHelper::ShapeType ShapeFileHelper::determineShapeType(const QString &file, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::determineShapeType(file, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::determineShapeType(file, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::determineShapeType(file, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::determineShapeType(file, errorString);
    case ShapeFileType::None:
    default:
        return ShapeType::Error;
    }
}

bool ShapeFileHelper::loadPolygonFromFile(const QString &file, QList<QGeoCoordinate> &vertices, QString &errorString, double filterMeters)
{
    errorString.clear();
    vertices.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolygonFromFile(file, vertices, errorString, filterMeters);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolygonFromFile(file, vertices, errorString, filterMeters);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolygonFromFile(file, vertices, errorString, filterMeters);
    case ShapeFileType::GPX:
        return GPXHelper::loadPolygonFromFile(file, vertices, errorString, filterMeters);
    case ShapeFileType::None:
    default:
        return false;
    }
}

bool ShapeFileHelper::loadPolylineFromFile(const QString &file, QList<QGeoCoordinate> &coords, QString &errorString, double filterMeters)
{
    errorString.clear();
    coords.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolylineFromFile(file, coords, errorString, filterMeters);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolylineFromFile(file, coords, errorString, filterMeters);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolylineFromFile(file, coords, errorString, filterMeters);
    case ShapeFileType::GPX:
        return GPXHelper::loadPolylineFromFile(file, coords, errorString, filterMeters);
    case ShapeFileType::None:
    default:
        return false;
    }
}

int ShapeFileHelper::getEntityCount(const QString &file, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::getEntityCount(file, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::getEntityCount(file, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::getEntityCount(file, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::getEntityCount(file, errorString);
    case ShapeFileType::None:
    default:
        return 0;
    }
}

bool ShapeFileHelper::loadPolygonsFromFile(const QString &file, QList<QList<QGeoCoordinate>> &polygons, QString &errorString, double filterMeters)
{
    errorString.clear();
    polygons.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolygonsFromFile(file, polygons, errorString, filterMeters);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolygonsFromFile(file, polygons, errorString, filterMeters);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolygonsFromFile(file, polygons, errorString, filterMeters);
    case ShapeFileType::GPX:
        return GPXHelper::loadPolygonsFromFile(file, polygons, errorString, filterMeters);
    case ShapeFileType::None:
    default:
        return false;
    }
}

bool ShapeFileHelper::loadPolylinesFromFile(const QString &file, QList<QList<QGeoCoordinate>> &polylines, QString &errorString, double filterMeters)
{
    errorString.clear();
    polylines.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolylinesFromFile(file, polylines, errorString, filterMeters);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolylinesFromFile(file, polylines, errorString, filterMeters);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolylinesFromFile(file, polylines, errorString, filterMeters);
    case ShapeFileType::GPX:
        return GPXHelper::loadPolylinesFromFile(file, polylines, errorString, filterMeters);
    case ShapeFileType::None:
    default:
        return false;
    }
}

bool ShapeFileHelper::loadPointsFromFile(const QString &file, QList<QGeoCoordinate> &points, QString &errorString)
{
    errorString.clear();
    points.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPointsFromFile(file, points, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPointsFromFile(file, points, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPointsFromFile(file, points, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::loadPointsFromFile(file, points, errorString);
    case ShapeFileType::None:
    default:
        return false;
    }
}

bool ShapeFileHelper::loadPolygonWithHolesFromFile(const QString &file, QGeoPolygon &polygon, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolygonWithHolesFromFile(file, polygon, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolygonWithHolesFromFile(file, polygon, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolygonWithHolesFromFile(file, polygon, errorString);
    case ShapeFileType::GPX:
        errorString = QString(_errorPrefix).arg(tr("GPX format does not support polygons with holes"));
        return false;
    case ShapeFileType::None:
    default:
        return false;
    }
}

bool ShapeFileHelper::loadPolygonsWithHolesFromFile(const QString &file, QList<QGeoPolygon> &polygons, QString &errorString)
{
    errorString.clear();
    polygons.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::loadPolygonsWithHolesFromFile(file, polygons, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::loadPolygonsWithHolesFromFile(file, polygons, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::loadPolygonsWithHolesFromFile(file, polygons, errorString);
    case ShapeFileType::GPX:
        errorString = QString(_errorPrefix).arg(tr("GPX format does not support polygons with holes"));
        return false;
    case ShapeFileType::None:
    default:
        return false;
    }
}

// ============================================================================
// Save functions
// ============================================================================

bool ShapeFileHelper::savePolygonToFile(const QString &file, const QList<QGeoCoordinate> &vertices, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolygonToFile(file, vertices, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolygonToFile(file, vertices, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolygonToFile(file, vertices, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::savePolygonToFile(file, vertices, errorString);
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePolygonsToFile(const QString &file, const QList<QList<QGeoCoordinate>> &polygons, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolygonsToFile(file, polygons, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolygonsToFile(file, polygons, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolygonsToFile(file, polygons, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::savePolygonsToFile(file, polygons, errorString);
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePolygonWithHolesToFile(const QString &file, const QGeoPolygon &polygon, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolygonWithHolesToFile(file, polygon, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolygonWithHolesToFile(file, polygon, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolygonWithHolesToFile(file, polygon, errorString);
    case ShapeFileType::GPX:
        errorString = QString(_saveErrorPrefix).arg(tr("GPX format does not support polygons with holes"));
        return false;
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePolygonsWithHolesToFile(const QString &file, const QList<QGeoPolygon> &polygons, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolygonsWithHolesToFile(file, polygons, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolygonsWithHolesToFile(file, polygons, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolygonsWithHolesToFile(file, polygons, errorString);
    case ShapeFileType::GPX:
        errorString = QString(_saveErrorPrefix).arg(tr("GPX format does not support polygons with holes"));
        return false;
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePolylineToFile(const QString &file, const QList<QGeoCoordinate> &coords, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolylineToFile(file, coords, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolylineToFile(file, coords, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolylineToFile(file, coords, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::savePolylineToFile(file, coords, errorString);
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePolylinesToFile(const QString &file, const QList<QList<QGeoCoordinate>> &polylines, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePolylinesToFile(file, polylines, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePolylinesToFile(file, polylines, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePolylinesToFile(file, polylines, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::savePolylinesToFile(file, polylines, errorString);
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

bool ShapeFileHelper::savePointsToFile(const QString &file, const QList<QGeoCoordinate> &points, QString &errorString)
{
    errorString.clear();

    switch (_getShapeFileType(file, errorString)) {
    case ShapeFileType::KML:
        return KMLHelper::savePointsToFile(file, points, errorString);
    case ShapeFileType::SHP:
        return SHPFileHelper::savePointsToFile(file, points, errorString);
    case ShapeFileType::GeoJson:
        return GeoJsonHelper::savePointsToFile(file, points, errorString);
    case ShapeFileType::GPX:
        return GPXHelper::savePointsToFile(file, points, errorString);
    case ShapeFileType::None:
    default:
        errorString = QString(_saveErrorPrefix).arg(tr("Unsupported file type"));
        return false;
    }
}

QStringList ShapeFileHelper::fileDialogKMLFilters()
{
    static const QStringList filters = QStringList(tr("KML Files (*%1)").arg(kmlFileExtension));
    return filters;
}

QStringList ShapeFileHelper::fileDialogFilters()
{
    static const QStringList filters = QStringList(tr("Shape Files (*%1 *%2 *%3 *%4)")
        .arg(kmlFileExtension, shpFileExtension, geojsonFileExtension, gpxFileExtension));
    return filters;
}
