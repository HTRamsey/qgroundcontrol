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
};
