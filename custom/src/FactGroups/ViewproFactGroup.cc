#include "ViewproFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ViewproFactGroupLog, "qgc.custom.factgroups.viewprofactgroup")

ViewproFactGroup::ViewproFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/ViewproFact.json", parent)
{

}

ViewproFactGroup::~ViewproFactGroup()
{

}

void ViewproFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    default:
        return;
    }

    _setTelemetryAvailable(true);
}

void ViewproFactGroup::_handleMsg(mavlink_message_t &message)
{

}
