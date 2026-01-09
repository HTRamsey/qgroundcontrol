#include "KmzPlanExporter.h"
#include "KMLPlanDomDocument.h"
#include "MissionController.h"
#include "PlanExporter.h"

#include <QtCore/QFile>
#include <private/qzipwriter_p.h>

KmzPlanExporter::KmzPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

IMPLEMENT_PLAN_EXPORTER_SINGLETON(KmzPlanExporter)

bool KmzPlanExporter::exportToFile(const QString& filename,
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

    // Create KML content
    KMLPlanDomDocument planKML;
    planKML.addMission(missionController->controllerVehicle(), visualItems, rgMissionItems);
    const QString kmlContent = planKML.toString();

    deleteParent->deleteLater();

    // Create KMZ (zip) file with doc.kml inside
    QZipWriter zipWriter(filename);
    if (zipWriter.status() != QZipWriter::NoError) {
        errorString = tr("Failed to create KMZ file");
        return false;
    }

    zipWriter.addFile(QStringLiteral("doc.kml"), kmlContent.toUtf8());

    if (zipWriter.status() != QZipWriter::NoError) {
        errorString = tr("Failed to write KML data to KMZ");
        return false;
    }

    zipWriter.close();
    return true;
}
