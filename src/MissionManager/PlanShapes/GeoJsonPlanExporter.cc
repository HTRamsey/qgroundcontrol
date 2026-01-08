#include "GeoJsonPlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "GeoJsonPlanDocument.h"

#include <QtCore/QFile>

GeoJsonPlanExporter* GeoJsonPlanExporter::s_instance = nullptr;

GeoJsonPlanExporter::GeoJsonPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

GeoJsonPlanExporter* GeoJsonPlanExporter::instance()
{
    if (!s_instance) {
        s_instance = new GeoJsonPlanExporter();
        PlanExporter::registerExporter(s_instance->fileExtension(), s_instance);
    }
    return s_instance;
}

bool GeoJsonPlanExporter::exportToFile(const QString& filename,
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

    // Create GeoJSON document with full mission structure
    GeoJsonPlanDocument planGeoJson;
    planGeoJson.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    deleteParent->deleteLater();

    // Write to file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorString = tr("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    file.write(planGeoJson.toJson());
    file.close();

    return true;
}
