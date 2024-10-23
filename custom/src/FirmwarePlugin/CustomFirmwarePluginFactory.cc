#include "CustomFirmwarePluginFactory.h"
#include "CustomFirmwarePlugin.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(CustomFirmwarePluginFactoryLog, "qgc.custom.firmwareplugin.customfirmwarepluginfactory")

CustomFirmwarePluginFactory CustomFirmwarePluginFactoryImp;

CustomFirmwarePluginFactory::CustomFirmwarePluginFactory(QObject *parent)
    : FirmwarePluginFactory()
{
    Q_UNUSED(parent);

    // qCDebug(CustomFirmwarePluginFactoryLog) << Q_FUNC_INFO << this;
}

CustomFirmwarePluginFactory::~CustomFirmwarePluginFactory()
{
    // qCDebug(CustomFirmwarePluginFactoryLog) << Q_FUNC_INFO << this;
}

QList<QGCMAVLink::FirmwareClass_t> CustomFirmwarePluginFactory::supportedFirmwareClasses() const
{
    QList<QGCMAVLink::FirmwareClass_t> firmwareClasses;
    firmwareClasses.append(QGCMAVLink::FirmwareClassArduPilot);
    return firmwareClasses;
}

QList<QGCMAVLink::VehicleClass_t> CustomFirmwarePluginFactory::supportedVehicleClasses() const
{
    QList<QGCMAVLink::VehicleClass_t> vehicleClasses;
    vehicleClasses.append(QGCMAVLink::VehicleClassMultiRotor);
    return vehicleClasses;
}

FirmwarePlugin *CustomFirmwarePluginFactory::firmwarePluginForAutopilot(MAV_AUTOPILOT autopilotType, MAV_TYPE vehicleType)
{
    if (autopilotType == MAV_AUTOPILOT_ARDUPILOTMEGA) {
        switch (vehicleType) {
        case MAV_TYPE_QUADROTOR:
        case MAV_TYPE_HEXAROTOR:
        case MAV_TYPE_OCTOROTOR:
        case MAV_TYPE_TRICOPTER:
        case MAV_TYPE_COAXIAL:
        case MAV_TYPE_HELICOPTER:
            if (!_pluginInstance) {
                _pluginInstance = new CustomFirmwarePlugin;
            }
            return _pluginInstance;
        default:
            break;
        }
    }

    return nullptr;
}
