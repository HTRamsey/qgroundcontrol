#include "OpenAirPlanImporter.h"
#include "OpenAirParser.h"
#include "QGCLoggingCategory.h"

OpenAirPlanImporter::OpenAirPlanImporter(QObject* parent)
    : PlanImporter(parent)
{
    registerImporter(QStringLiteral("txt"), this);
    registerImporter(QStringLiteral("air"), this);
}

IMPLEMENT_PLAN_IMPORTER_SINGLETON(OpenAirPlanImporter)

PlanImportResult OpenAirPlanImporter::importFromFile(const QString& filename)
{
    PlanImportResult result;

    OpenAirParser::ParseResult parseResult = OpenAirParser::parseFile(filename);

    if (!parseResult.success) {
        result.success = false;
        result.errorString = parseResult.errorString;
        return result;
    }

    if (parseResult.airspaces.isEmpty()) {
        result.success = false;
        result.errorString = tr("No airspaces found in file");
        return result;
    }

    for (const OpenAirParser::Airspace& airspace : parseResult.airspaces) {
        if (!airspace.boundary.isEmpty()) {
            result.polygons.append(airspace.boundary);
            result.waypointNames.append(airspace.name);
        }
    }

    result.success = !result.polygons.isEmpty();
    result.formatDescription = tr("OpenAir Airspace (%1 airspaces)").arg(result.polygons.count());

    if (!result.success) {
        result.errorString = tr("No valid airspace boundaries found");
    }

    qCDebug(PlanImporterLog) << "Imported" << result.polygons.count() << "airspace polygons from OpenAir file";

    return result;
}
