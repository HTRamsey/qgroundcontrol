#include "ParameterCacheDiagnostics.h"

#include <QtCore/QCoreApplication>

#include <cstring>

#include "AppMessages.h"
#include "FactMetaData.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ParameterCacheDiagnosticsLog, "Vehicle.Parameters.ParameterCacheDiagnostics")

bool ParameterCacheDiagnostics::isEnabled()
{
    return ParameterCacheDiagnosticsLog().isDebugEnabled();
}

void ParameterCacheDiagnostics::noteCrcMismatch(int componentId, const ParameterCache::CacheMap& cacheParams)
{
    _crcMismatchCompIds.insert(componentId);
    _cacheParamsByComp[componentId] = cacheParams;
    QSet<QString>& unseen = _unseenByComp[componentId];
    unseen.clear();
    unseen.reserve(cacheParams.size());
    for (auto it = cacheParams.cbegin(); it != cacheParams.cend(); ++it) {
        unseen.insert(it.key());
    }
    QGC::showAppMessage(QCoreApplication::translate("ParameterCacheDiagnostics",
                                                    "Parameter cache CRC match failed"));
}

void ParameterCacheDiagnostics::noteParamValue(int componentId, const QString& parameterName,
                                               const QVariant& vehicleValue)
{
    if (!_crcMismatchCompIds.contains(componentId)) {
        return;
    }
    if (!_cacheParamsByComp[componentId].contains(parameterName)) {
        qCDebug(ParameterCacheDiagnosticsLog) << "Parameter missing from cache" << parameterName;
        return;
    }
    const ParameterCache::ParamTypeVal& cacheVal = _cacheParamsByComp[componentId][parameterName];
    const size_t dataSize = FactMetaData::typeToSize(cacheVal.type);
    if (memcmp(cacheVal.value.constData(), vehicleValue.constData(), dataSize) != 0) {
        qCDebug(ParameterCacheDiagnosticsLog)
            << "Cache/Vehicle values differ name:cache:actual" << parameterName << vehicleValue << cacheVal.value;
    }
    _unseenByComp[componentId].remove(parameterName);
}

void ParameterCacheDiagnostics::clear()
{
    _crcMismatchCompIds.clear();
    _cacheParamsByComp.clear();
    _unseenByComp.clear();
}

void ParameterCacheDiagnostics::logUnseen() const
{
    for (const int cacheCompId : _crcMismatchCompIds) {
        const auto it = _unseenByComp.constFind(cacheCompId);
        if (it == _unseenByComp.constEnd()) {
            continue;
        }
        for (const QString& paramName : std::as_const(*it)) {
            qCDebug(ParameterCacheDiagnosticsLog)
                << "Parameter in cache but not on vehicle componentId:Name" << cacheCompId << paramName;
        }
    }
}
