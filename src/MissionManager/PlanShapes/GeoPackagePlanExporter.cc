#include "GeoPackagePlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "GeoPackagePlanDocument.h"

GeoPackagePlanExporter::GeoPackagePlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

IMPLEMENT_PLAN_EXPORTER_SINGLETON(GeoPackagePlanExporter)

bool GeoPackagePlanExporter::exportToFile(const QString& filename,
                                           MissionController* missionController,
                                           QString& errorString)
{
    if (!missionController) {
        errorString = tr("No mission controller provided");
        return false;
    }

    QmlObjectListModel* visualItems = missionController->visualItems();
    if (!visualItems || visualItems->count() == 0) {
        errorString = tr("No mission items to export");
        return false;
    }

    QObject* deleteParent = new QObject();
    QList<MissionItem*> rgMissionItems;

    if (!missionController->convertToMissionItems(rgMissionItems, deleteParent)) {
        deleteParent->deleteLater();
        errorString = tr("Failed to convert mission items");
        return false;
    }

    // Create GeoPackage document with full mission structure
    GeoPackagePlanDocument planGpkg;
    planGpkg.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    deleteParent->deleteLater();

    // Export to GeoPackage file
    return planGpkg.exportToFile(filename, errorString);
}
