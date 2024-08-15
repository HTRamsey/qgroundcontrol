#pragma once

#include "APMAutoPilotPlugin.h"

class Vehicle;

class CustomAutoPilotPlugin : public APMAutoPilotPlugin
{
    Q_OBJECT

public:
    CustomAutoPilotPlugin(Vehicle *vehicle, QObject *parent);
};
