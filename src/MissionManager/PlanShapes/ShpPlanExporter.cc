#include "ShpPlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "ShpPlanDocument.h"

ShpPlanExporter::ShpPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

IMPLEMENT_PLAN_EXPORTER_SINGLETON(ShpPlanExporter)

bool ShpPlanExporter::exportToFile(const QString& filename,
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

    // Create Shapefile document set with full mission structure
    ShpPlanDocument planShp;
    planShp.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    deleteParent->deleteLater();

    // Export creates multiple files: _waypoints.shp, _path.shp, _areas.shp
    if (!planShp.exportToFiles(filename, errorString)) {
        return false;
    }

    _lastCreatedFiles = planShp.createdFiles();
    return true;
}

QStringList ShpPlanExporter::lastCreatedFiles() const
{
    return _lastCreatedFiles;
}
