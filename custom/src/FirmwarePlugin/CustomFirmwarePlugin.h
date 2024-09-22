#pragma once

#include <QtCore/QLoggingCategory>

#include "ArduCopterFirmwarePlugin.h"

class AutoPilotPlugin;
class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(CustomFirmwarePluginLog)

class CustomFirmwarePlugin : public ArduCopterFirmwarePlugin
{
    Q_OBJECT

public:
    CustomFirmwarePlugin(QObject *parent = nullptr);
    ~CustomFirmwarePlugin();

    AutoPilotPlugin *autopilotPlugin(Vehicle *vehicle) final;

    bool supportsSmartRTL() const final { return false; }
    // double minimumTakeoffAltitudeMeters(Vehicle *vehicle) final;
    const QVariantList &toolIndicators(const Vehicle *vehicle) final;
    const QVariantList &modeIndicators(const Vehicle *vehicle) final;
    // void adjustOutgoingMavlinkMessageThreadSafe(Vehicle* vehicle, LinkInterface* outgoingLink, mavlink_message_t* message) final;
};
