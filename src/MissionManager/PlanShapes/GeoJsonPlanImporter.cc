#include "GeoJsonPlanImporter.h"
#include "GeoJsonHelper.h"

GeoJsonPlanImporter::GeoJsonPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
    registerImporter(QStringLiteral("geojson"), this);
    registerImporter(QStringLiteral("json"), this);
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(GeoJsonPlanImporter)

PlanImportResult GeoJsonPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    QString errorString;

    GeoJsonHelper::loadPointsFromFile(filename, result.waypoints, errorString);

    QList<QList<QGeoCoordinate>> polylines;
    if (GeoJsonHelper::loadPolylinesFromFile(filename, polylines, errorString, true)) {
        for (const auto& polyline : polylines) {
            result.trackPoints.append(polyline);
        }
    }

    GeoJsonHelper::loadPolygonsFromFile(filename, result.polygons, errorString, true);

    result.success = (result.itemCount() > 0);
    if (result.success) {
        result.formatDescription = tr("GeoJSON (%1 features)").arg(result.itemCount());
    } else {
        result.errorString = errorString.isEmpty() ? tr("No features found in GeoJSON file") : errorString;
    }

    return result;
}
