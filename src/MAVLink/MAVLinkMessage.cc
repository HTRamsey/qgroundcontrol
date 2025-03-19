/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "MAVLinkMessage.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(MAVLinkMessageLog, "qgc.mavlink.mavlinkmessage")

QGCMAVLinkMessage::QGCMAVLinkMessage(const mavlink_message_t &message, QObject *parent)
    : QObject(parent)
    , _message(message)

{
    // qCDebug(LogDownloadControllerLog) << Q_FUNC_INFO << this;

    const mavlink_message_info_t *const msgInfo = mavlink_get_message_info(&message);
    if (!msgInfo) {
        qCWarning(MAVLinkMessageLog) << QStringLiteral("QGCMAVLinkMessage NULL msgInfo msgid(%1)").arg(message.msgid);
        return;
    }

    _name = QString(msgInfo->name);
    qCDebug(MAVLinkMessageLog) << "New Message:" << _name;
}

QGCMAVLinkMessage::~QGCMAVLinkMessage()
{
    // qCDebug(LogDownloadControllerLog) << Q_FUNC_INFO << this;
}

void QGCMAVLinkMessage::updateFreq()
{
    const quint64 msgCount = _count - _lastCount;
    const qreal lastRateHz = _actualRateHz;
    _actualRateHz = (0.2 * _actualRateHz) + (0.8 * msgCount);
    _lastCount = _count;
    if (_actualRateHz != lastRateHz) {
        emit actualRateHzChanged();
    }
}

void QGCMAVLinkMessage::setTargetRateHz(int32_t rate)
{
    if (rate != _targetRateHz) {
        _targetRateHz = rate;
        emit targetRateHzChanged();
    }
}

void QGCMAVLinkMessage::update(const mavlink_message_t &message)
{
    _count++;
    _message = message;

    emit countChanged();
}
