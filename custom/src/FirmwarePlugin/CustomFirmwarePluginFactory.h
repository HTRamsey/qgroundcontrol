#pragma once

#include <QtCore/QLoggingCategory>

#include "FirmwarePluginFactory.h"
#include "QGCMAVLink.h"

class FirmwarePlugin;
class CustomFirmwarePlugin;

Q_DECLARE_LOGGING_CATEGORY(CustomFirmwarePluginFactoryLog)

class CustomFirmwarePluginFactory : public FirmwarePluginFactory
{
    Q_OBJECT

public:
    CustomFirmwarePluginFactory(QObject *parent = nullptr);
    ~CustomFirmwarePluginFactory();
    QList<QGCMAVLink::FirmwareClass_t> supportedFirmwareClasses() const final;
    QList<QGCMAVLink::VehicleClass_t> supportedVehicleClasses() const final;
    FirmwarePlugin *firmwarePluginForAutopilot(MAV_AUTOPILOT autopilotType, MAV_TYPE vehicleType) final;

private:
    CustomFirmwarePlugin *_pluginInstance = nullptr;
};

extern CustomFirmwarePluginFactory CustomFirmwarePluginFactoryImp;
