#include "GCSFactGroup.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(GCSFactGroupLog, "qgc.custom.factgroups.gcsfactgroup")

GCSFactGroup::GCSFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/GCSFact.json", parent)
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

GCSFactGroup::~GCSFactGroup()
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}
