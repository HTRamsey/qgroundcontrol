#include "WktPlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "WktPlanDocument.h"

WktPlanExporter::WktPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

IMPLEMENT_PLAN_EXPORTER_SINGLETON(WktPlanExporter)

bool WktPlanExporter::exportToFile(const QString& filename,
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

    WktPlanDocument planWkt;
    planWkt.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    deleteParent->deleteLater();

    return planWkt.saveToFile(filename, errorString);
}
