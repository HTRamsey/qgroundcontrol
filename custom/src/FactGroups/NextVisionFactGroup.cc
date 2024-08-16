#include "NextVisionFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(NextVisionFactGroupLog, "qgc.custom.factgroups.nextvisionfactgroup")

NextVisionFactGroup::NextVisionFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/Vehicle/NextVisionFact.json", parent)
{

}

NextVisionFactGroup::~NextVisionFactGroup()
{

}

void NextVisionFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    default:
        return;
    }

    _setTelemetryAvailable(true);
}

void NextVisionFactGroup::_handleMsg(mavlink_message_t &message)
{

}
