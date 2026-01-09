#include "KmzPlanImporter.h"
#include "KMLHelper.h"

KmzPlanImporter::KmzPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
    registerImporter(QStringLiteral("kmz"), this);
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(KmzPlanImporter)

PlanImportResult KmzPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;
    QString errorString;

    // KMLHelper handles KMZ files transparently (decompresses internally)
    KMLHelper::loadPointsFromFile(filename, result.waypoints, errorString);

    QList<QList<QGeoCoordinate>> polylines;
    if (KMLHelper::loadPolylinesFromFile(filename, polylines, errorString, true)) {
        for (const auto& polyline : polylines) {
            result.trackPoints.append(polyline);
        }
    }

    KMLHelper::loadPolygonsFromFile(filename, result.polygons, errorString, true);

    result.success = (result.itemCount() > 0);
    if (result.success) {
        result.formatDescription = tr("KMZ (%1 features)").arg(result.itemCount());
    } else {
        result.errorString = errorString.isEmpty() ? tr("No features found in KMZ file") : errorString;
    }

    return result;
}
