#include "ViewproFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ViewproFactGroupLog, "qgc.custom.factgroups.viewprofactgroup")

ViewproFactGroup::ViewproFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/ViewproFact.json", parent)
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

ViewproFactGroup::~ViewproFactGroup()
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

void ViewproFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_STATUSTEXT:
        _handleStatusText(vehicle, message);
        break;
    default:
        return;
    }

    _setTelemetryAvailable(true);
}

void ViewproFactGroup::_handleStatusText(Vehicle *vehicle, const mavlink_message_t &message)
{
    mavlink_statustext_t data;
    mavlink_msg_statustext_decode(&message, &data);

    const QString text = QString::fromStdString(data.text);
    if (text.contains("Payload", Qt::CaseInsensitive)) {
        const QStringList parts = text.split(": ");
        QString payload = parts[1];
        static const QRegularExpression regexp("[()]");
        payload = payload.remove(regexp);
        if (payload.contains("Gimbal", Qt::CaseInsensitive)) {
            const QStringList params = payload.remove(QChar(' ')).split(",");
            const QString gimbalType = params.at(1);
            const QString gimbalModel = params.at(2);
            if (gimbalType.contains("Viewpro", Qt::CaseInsensitive)) {
                /* if (!_viewproEnabledFact.rawValue().toBool()) {
                    _viewproEnabledFact.setRawValue(true);
                } */
            }
        }
    }
}
