#pragma once

#include <QtCore/QLoggingCategory>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(GCSFactGroupLog)

class GCSFactGroup : public FactGroup
{
    Q_OBJECT

public:
    GCSFactGroup(QObject *parent = nullptr);
    ~GCSFactGroup();

private:
    const QString _ipAddress = QStringLiteral("192.168.1.9");

    static constexpr uint16_t _port = 14570;
    static constexpr mavlink_system_t _mavsys = {3, MAV_COMP_ID_MISSIONPLANNER};
    static constexpr const char *_ip = "192.168.1.9";
};
