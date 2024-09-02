#pragma once

#include "APMAutoPilotPlugin.h"

class Vehicle;
class APMFollowComponent;

class CustomAutoPilotPlugin : public APMAutoPilotPlugin
{
    Q_OBJECT

public:
    CustomAutoPilotPlugin(Vehicle *vehicle, QObject *parent);
    ~CustomAutoPilotPlugin();

    const QVariantList &vehicleComponents() final;
    // QString prerequisiteSetup(VehicleComponent *component) const final;

private:
    APMFollowComponent *_followComponent = nullptr;
    QVariantList _components;
};
