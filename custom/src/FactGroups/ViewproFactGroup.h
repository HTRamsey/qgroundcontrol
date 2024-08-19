#pragma once

#include <QtCore/QLoggingCategory>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(ViewproFactGroupLog)

class ViewproFactGroup : public FactGroup
{
    Q_OBJECT

public:
    ViewproFactGroup(QObject *parent = nullptr);
    ~ViewproFactGroup();

    void handleMessage(Vehicle *vehicle, mavlink_message_t &message) final;

private:
    void _handleStatusText(Vehicle *vehicle, const mavlink_message_t &message);

    const QString _ipAddress = QStringLiteral("192.168.1.4");
    const QString _streamUrl = QStringLiteral("rtsp://%1:554/stream0");
};
