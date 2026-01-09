#include "KmlPlanImporter.h"
#include "KMLHelper.h"
#include "PlanImporter.h"

KmlPlanImporter::KmlPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(KmlPlanImporter)

PlanImportResult KmlPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    result.formatDescription = formatName();

    QString errorString;

    // Load points (placemarks)
    QList<QGeoCoordinate> points;
    if (KMLHelper::loadPointsFromFile(filename, points, errorString)) {
        result.waypoints = points;
    }

    // Load polylines (LineStrings)
    QList<QList<QGeoCoordinate>> polylines;
    if (KMLHelper::loadPolylinesFromFile(filename, polylines, errorString, true)) {
        for (const auto& polyline : polylines) {
            result.trackPoints.append(polyline);
        }
    }

    // Load polygons
    QList<QList<QGeoCoordinate>> polygons;
    if (KMLHelper::loadPolygonsFromFile(filename, polygons, errorString, true)) {
        result.polygons = polygons;
    }

    if (result.waypoints.isEmpty() && result.trackPoints.isEmpty() && result.polygons.isEmpty()) {
        result.errorString = tr("No points, paths, or polygons found in KML file");
        return result;
    }

    result.success = true;
    return result;
}
