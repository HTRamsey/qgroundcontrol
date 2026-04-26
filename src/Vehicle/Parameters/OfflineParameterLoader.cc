#include "OfflineParameterLoader.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

#include "CompInfoParam.h"
#include "ComponentInformationManager.h"
#include "Fact.h"
#include "FirmwarePlugin.h"
#include "ParameterManager.h"
#include "ParameterMavlinkCodec.h"
#include "ParameterTextLine.h"
#include "QGCLoggingCategory.h"
#include "Vehicle.h"

QGC_LOGGING_CATEGORY(OfflineParameterLoaderLog, "Vehicle.Parameters.OfflineParameterLoader")

void OfflineParameterLoader::load(ParameterManager* manager)
{
    Vehicle* const vehicle = manager->vehicle();
    const QString paramFilename = vehicle->firmwarePlugin()->offlineEditingParamFile(vehicle);
    if (paramFilename.isEmpty()) {
        return;
    }

    QFile paramFile(paramFilename);
    if (!paramFile.open(QFile::ReadOnly)) {
        qCWarning(OfflineParameterLoaderLog) << "Unable to open offline editing params file" << paramFilename;
        return;
    }

    QTextStream paramStream(&paramFile);
    int lastOfflineComponentId = -1;
    int registeredCount = 0;
    while (!paramStream.atEnd()) {
        const QString line = paramStream.readLine();
        const auto parsed = ParameterTextLine::parseLine(line);
        if (!parsed) {
            if (!line.startsWith(QLatin1Char('#')) && !line.isEmpty()) {
                qCWarning(OfflineParameterLoaderLog) << "Malformed offline param line:" << line;
            }
            continue;
        }

        // Note: parsed->rawValue is intentionally discarded; the offline-editing path uses
        // metadata defaults, not the values from the .params source file.
        const FactMetaData::ValueType_t factType = ParameterMavlinkCodec::mavTypeToFactType(parsed->type);
        FactMetaData* const factMetaData = vehicle->compInfoManager()
                                               ->compInfoParam(parsed->componentId)
                                               ->factMetaDataForName(parsed->name, factType);
        if (!factMetaData) {
            qCWarning(OfflineParameterLoaderLog) << "No metadata for offline param" << parsed->componentId
                                                 << parsed->name << "- skipping";
            continue;
        }

        auto* const fact = new Fact(parsed->componentId, parsed->name, factType, manager);
        fact->setMetaData(factMetaData);

        if (!manager->registerFact(ParameterManager::anyComponentId, parsed->name, fact)) {
            qCWarning(OfflineParameterLoaderLog) << "Duplicate offline param" << parsed->name << "- skipping";
            delete fact;
            continue;
        }
        ++registeredCount;
        lastOfflineComponentId = parsed->componentId;
    }

    if (lastOfflineComponentId != -1) {
        // Set once after the loop instead of on every row. Last-write-wins matches the prior
        // per-row behaviour for typical files where every row carries the same component id.
        vehicle->setOfflineEditingDefaultComponentId(lastOfflineComponentId);
    }

    if (registeredCount == 0) {
        qCWarning(OfflineParameterLoaderLog) << "No facts registered from" << paramFilename
                                             << "- leaving parametersReady=false";
        return;
    }

    manager->setParametersReady(true);
    manager->resetCacheDiagnostics();
}
