#include "GeoPackagePlanImporter.h"
#include "GeoPackageHelper.h"

GeoPackagePlanImporter::GeoPackagePlanImporter(QObject* parent)
    : PlanImporter(parent)
{
    registerImporter(QStringLiteral("gpkg"), this);
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(GeoPackagePlanImporter)

PlanImportResult GeoPackagePlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;

    GeoPackageHelper::LoadResult gpkgResult = GeoPackageHelper::loadAllFeatures(filename);

    if (!gpkgResult.success) {
        result.success = false;
        result.errorString = gpkgResult.errorString;
        return result;
    }

    result.waypoints = gpkgResult.points;

    for (const auto& polyline : gpkgResult.polylines) {
        result.trackPoints.append(polyline);
    }

    result.polygons = gpkgResult.polygons;

    result.success = (result.itemCount() > 0);
    if (result.success) {
        result.formatDescription = tr("GeoPackage (%1 features)").arg(result.itemCount());
    } else {
        result.errorString = tr("No features found in GeoPackage");
    }

    return result;
}
