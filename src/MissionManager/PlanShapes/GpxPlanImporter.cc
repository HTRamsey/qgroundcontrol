#include "GpxPlanImporter.h"
#include "GPXHelper.h"
#include "PlanImporter.h"

GpxPlanImporter::GpxPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(GpxPlanImporter)

PlanImportResult GpxPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    result.formatDescription = formatName();

    QString errorString;

    // Load waypoints
    QList<QGeoCoordinate> points;
    if (GPXHelper::loadPointsFromFile(filename, points, errorString)) {
        result.waypoints = points;
    }

    // Load tracks as polylines
    QList<QList<QGeoCoordinate>> polylines;
    if (GPXHelper::loadPolylinesFromFile(filename, polylines, errorString, true)) {
        for (const auto& polyline : polylines) {
            result.trackPoints.append(polyline);
        }
    }

    // Load routes as well
    QList<QList<QGeoCoordinate>> routes;
    if (GPXHelper::loadPolylinesFromFile(filename, routes, errorString, false)) {
        for (const auto& route : routes) {
            result.trackPoints.append(route);
        }
    }

    if (result.waypoints.isEmpty() && result.trackPoints.isEmpty()) {
        result.errorString = tr("No waypoints or tracks found in GPX file");
        return result;
    }

    result.success = true;
    return result;
}
