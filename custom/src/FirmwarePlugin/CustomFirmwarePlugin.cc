#include "CustomFirmwarePlugin.h"
#include "CustomAutoPilotPlugin.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(CustomFirmwarePluginLog, "qgc.custom.firmwareplugin.customfirmwareplugin")

CustomFirmwarePlugin::CustomFirmwarePlugin(QObject *parent)
    : ArduCopterFirmwarePlugin()
{
    Q_UNUSED(parent);

    // qCDebug(CustomFirmwarePluginLog) << Q_FUNC_INFO << this;
}

CustomFirmwarePlugin::~CustomFirmwarePlugin()
{
    // qCDebug(CustomFirmwarePluginLog) << Q_FUNC_INFO << this;
}

AutoPilotPlugin *CustomFirmwarePlugin::autopilotPlugin(Vehicle *vehicle)
{
    return new CustomAutoPilotPlugin(vehicle, vehicle);
}

const QVariantList &CustomFirmwarePlugin::toolIndicators(const Vehicle *vehicle)
{
    static const QVariantList toolIndicatorList = QVariantList({
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/VehicleGPSIndicator.qml")),
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/RemoteIDIndicator.qml")),
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GimbalIndicator.qml")),
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/MessageIndicator.qml")),
    });

    return toolIndicatorList;
}

const QVariantList &CustomFirmwarePlugin::modeIndicators(const Vehicle *vehicle)
{
    Q_UNUSED(vehicle);

    static const QVariantList modeIndicatorList = QVariantList({
        QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/LinkIndicator.qml")),
    });

    return modeIndicatorList;
}
