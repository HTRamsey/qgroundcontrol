#include "KmzPlanExporter.h"
#include "KMLPlanDomDocument.h"
#include "MissionController.h"
#include "PlanExporter.h"
#include "QGCZip.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTextStream>

KmzPlanExporter* KmzPlanExporter::s_instance = nullptr;

KmzPlanExporter::KmzPlanExporter(QObject* parent)
    : PlanExporter(parent)
{
}

KmzPlanExporter* KmzPlanExporter::instance()
{
    if (!s_instance) {
        s_instance = new KmzPlanExporter();
        PlanExporter::registerExporter(s_instance->fileExtension(), s_instance);
    }
    return s_instance;
}

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

    // Create KMZ (zip) file with doc.kml inside using QGCZip
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        errorString = tr("Failed to create temporary directory");
        return false;
    }

    // Write doc.kml to temp directory (standard name for KMZ main file)
    const QString kmlPath = tempDir.filePath(QStringLiteral("doc.kml"));
    QFile kmlFile(kmlPath);
    if (!kmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorString = tr("Cannot create temporary KML file: %1").arg(kmlFile.errorString());
        return false;
    }
    kmlFile.write(kmlContent.toUtf8());
    kmlFile.close();

    // Zip the temp directory to create the KMZ file
    if (!QGCZip::zipDirectory(tempDir.path(), filename)) {
        errorString = tr("Failed to create KMZ archive");
        return false;
    }

    return true;
}
