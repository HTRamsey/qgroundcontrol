#include "GpxPlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "GpxPlanDocument.h"

GpxPlanExporter* GpxPlanExporter::s_instance = nullptr;

GpxPlanExporter::GpxPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

GpxPlanExporter* GpxPlanExporter::instance()
{
    if (!s_instance) {
        s_instance = new GpxPlanExporter();
        PlanExporter::registerExporter(s_instance->fileExtension(), s_instance);
    }
    return s_instance;
}

bool GpxPlanExporter::exportToFile(const QString& filename,
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

    GpxPlanDocument planGpx;
    planGpx.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    deleteParent->deleteLater();

    return planGpx.saveToFile(filename, errorString);
}
