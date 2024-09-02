#pragma once

#include "ArduCopterFirmwarePlugin.h"

class AutoPilotPlugin;
class Vehicle;

class CustomFirmwarePlugin : public ArduCopterFirmwarePlugin
{
    Q_OBJECT

public:
    CustomFirmwarePlugin();

    AutoPilotPlugin *autopilotPlugin(Vehicle *vehicle) final;

    bool supportsSmartRTL() const final { return false; }
    // double minimumTakeoffAltitudeMeters(Vehicle *vehicle) final;
    // const QVariantList &toolIndicators(const Vehicle *vehicle) final;
    // const QVariantList &modeIndicators(const Vehicle *vehicle) final;
    // void adjustOutgoingMavlinkMessageThreadSafe(Vehicle* vehicle, LinkInterface* outgoingLink, mavlink_message_t* message) final;
};
