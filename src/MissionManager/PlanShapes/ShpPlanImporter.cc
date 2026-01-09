#include "ShpPlanImporter.h"
#include "SHPFileHelper.h"

ShpPlanImporter::ShpPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
    registerImporter(QStringLiteral("shp"), this);
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(ShpPlanImporter)

PlanImportResult ShpPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    QString errorString;

    SHPFileHelper::loadPointsFromFile(filename, result.waypoints, errorString);

    QList<QList<QGeoCoordinate>> polylines;
    if (SHPFileHelper::loadPolylinesFromFile(filename, polylines, errorString, true)) {
        for (const auto& polyline : polylines) {
            result.trackPoints.append(polyline);
        }
    }

    SHPFileHelper::loadPolygonsFromFile(filename, result.polygons, errorString, true);

    result.success = (result.itemCount() > 0);
    if (result.success) {
        result.formatDescription = tr("Shapefile (%1 features)").arg(result.itemCount());
    } else {
        result.errorString = errorString.isEmpty() ? tr("No features found in Shapefile") : errorString;
    }

    return result;
}
