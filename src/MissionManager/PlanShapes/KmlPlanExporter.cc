#include "KmlPlanExporter.h"
#include "PlanExporter.h"
#include "MissionController.h"
#include "KMLPlanDomDocument.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>

KmlPlanExporter::KmlPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

IMPLEMENT_PLAN_EXPORTER_SINGLETON(KmlPlanExporter)

bool KmlPlanExporter::exportToFile(const QString& filename,
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

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        deleteParent->deleteLater();
        errorString = tr("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    KMLPlanDomDocument planKML;
    planKML.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);

    QTextStream stream(&file);
    stream << planKML.toString();
    file.close();

    deleteParent->deleteLater();
    return true;
}
