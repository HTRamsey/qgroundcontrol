#include "GeoFormatPlanImporter.h"
#include "GeoFormatRegistry.h"

#include <QtCore/QFileInfo>

GeoFormatPlanImporter::GeoFormatPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(GeoFormatPlanImporter)

bool GeoFormatPlanImporter::canImport(const QString& filename)
{
    QFileInfo fileInfo(filename);
    return GeoFormatRegistry::isSupported(fileInfo.suffix());
}

PlanImportResult GeoFormatPlanImporter::importFile(const QString& filename)
{
    return instance()->importFromFile(filename);
}

PlanImportResult GeoFormatPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;

    GeoFormatRegistry::LoadResult loadResult = GeoFormatRegistry::loadFile(filename);

    if (!loadResult.success) {
        result.errorString = loadResult.errorString;
        return result;
    }

    result.formatDescription = loadResult.formatUsed;

    // Convert LoadResult to PlanImportResult
    // Points become waypoints
    result.waypoints = loadResult.points;

    // Polylines are flattened into track points
    for (const auto& polyline : loadResult.polylines) {
        result.trackPoints.append(polyline);
    }

    // Polygons are preserved as-is
    result.polygons = loadResult.polygons;

    if (result.waypoints.isEmpty() && result.trackPoints.isEmpty() && result.polygons.isEmpty()) {
        result.errorString = tr("No geographic features found in file");
        return result;
    }

    result.success = true;
    return result;
}

QString GeoFormatPlanImporter::fileFilter() const
{
    return GeoFormatRegistry::readFileFilter();
}
