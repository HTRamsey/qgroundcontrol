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

    uint32_t pipMode() const { return 0; }
    uint32_t zoom() const { return 1; }

private:
    void _handleStatusText(Vehicle *vehicle, const mavlink_message_t &message);

    const QString _ipAddress = QStringLiteral("192.168.1.4");
    const QString _streamUrl = QStringLiteral("rtsp://%1:554/stream0");

    static constexpr uint16_t _port = 14580;
    // static constexpr mavlink_system_t _mavsys = {1, };
    static constexpr const char *_ip = "192.168.1.4";
};
