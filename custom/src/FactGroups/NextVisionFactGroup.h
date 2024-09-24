#pragma once

#include <QtCore/QLoggingCategory>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(NextVisionFactGroupLog)

class NextVisionFactGroup : public FactGroup
{
    Q_OBJECT

public:
    NextVisionFactGroup(QObject *parent = nullptr);
    ~NextVisionFactGroup();

    void handleMessage(Vehicle *vehicle, mavlink_message_t &message) final;

private:
    void _handleStatusText(Vehicle *vehicle, const mavlink_message_t &message);

    const QString _ipAddress = QStringLiteral("192.168.1.4");
    const QString _streamUrl = QStringLiteral("rtsp://%1:554/video");

    static constexpr uint16_t _port = 14580;
    // static constexpr mavlink_system_t _mavsys = {1, MAV_COMP_ID_AUTOPILOT1};
    static constexpr const char *_ip = "192.168.1.4";
};
