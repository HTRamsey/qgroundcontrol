#include "PlanImporter.h"
#include "CsvPlanImporter.h"
#include "GeoFormatPlanImporter.h"
#include "GeoFormatRegistry.h"
#include "GeoJsonPlanImporter.h"
#include "GeoPackagePlanImporter.h"
#include "GpxPlanImporter.h"
#include "KmlPlanImporter.h"
#include "KmzPlanImporter.h"
#include "OpenAirPlanImporter.h"
#include "ShpPlanImporter.h"
#include "WktPlanImporter.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFileInfo>

QGC_LOGGING_CATEGORY(PlanImporterLog, "PlanManager.PlanImporter")

QHash<QString, PlanImporter*> PlanImporter::s_importers;
bool PlanImporter::s_initialized = false;

void PlanImporter::registerImporter(const QString& extension, PlanImporter* importer)
{
    const QString lowerExt = extension.toLower();
    if (s_importers.contains(lowerExt)) {
        qCWarning(PlanImporterLog) << "Replacing existing importer for extension:" << lowerExt;
    }
    s_importers.insert(lowerExt, importer);
    qCDebug(PlanImporterLog) << "Registered importer for extension:" << lowerExt;
}

PlanImporter* PlanImporter::importerForExtension(const QString& extension)
{
    if (!s_initialized) {
        initializeImporters();
    }

    const QString lowerExt = extension.toLower();
    PlanImporter* importer = s_importers.value(lowerExt, nullptr);

    // If no specific importer registered, fallback to GeoFormatPlanImporter
    // if GeoFormatRegistry supports the format
    if (!importer && GeoFormatRegistry::isSupported(lowerExt)) {
        qCDebug(PlanImporterLog) << "Using GeoFormatPlanImporter fallback for extension:" << lowerExt;
        importer = GeoFormatPlanImporter::instance();
    }

    return importer;
}

PlanImporter* PlanImporter::importerForFile(const QString& filename)
{
    const QFileInfo fileInfo(filename);
    return importerForExtension(fileInfo.suffix());
}

QStringList PlanImporter::registeredExtensions()
{
    if (!s_initialized) {
        initializeImporters();
    }
    return s_importers.keys();
}

QStringList PlanImporter::fileDialogFilters()
{
    if (!s_initialized) {
        initializeImporters();
    }

    QStringList filters;
    for (auto it = s_importers.constBegin(); it != s_importers.constEnd(); ++it) {
        filters.append(it.value()->fileFilter());
    }
    return filters;
}

void PlanImporter::initializeImporters()
{
    if (s_initialized) {
        return;
    }
    s_initialized = true;

    // Initialize built-in importers (they register themselves)
    CsvPlanImporter::instance();
    GeoJsonPlanImporter::instance();
    GeoPackagePlanImporter::instance();
    GpxPlanImporter::instance();
    KmlPlanImporter::instance();
    KmzPlanImporter::instance();
    OpenAirPlanImporter::instance();
    ShpPlanImporter::instance();
    WktPlanImporter::instance();

    qCDebug(PlanImporterLog) << "PlanImporter system initialized with" << s_importers.count() << "importers";
}
