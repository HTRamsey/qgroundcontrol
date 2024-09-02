#include "CustomFirmwarePlugin.h"
#include "CustomAutoPilotPlugin.h"
#include "Vehicle.h"

CustomFirmwarePlugin::CustomFirmwarePlugin()
{

}

AutoPilotPlugin* CustomFirmwarePlugin::autopilotPlugin(Vehicle *vehicle)
{
    return new CustomAutoPilotPlugin(vehicle, vehicle);
}

#if 0
const QVariantList &CustomFirmwarePlugin::toolIndicators(const Vehicle *vehicle)
{
    static QVariantList toolIndicatorList;

    if (toolIndicatorList.isEmpty()) {
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/qml/QGroundControl/Controls/FlightModeIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/MessageIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GPSIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/TelemetryRSSIIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/RCRSSIIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/qml/QGroundControl/Controls/BatteryIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/RemoteIDIndicator.qml")),
        // QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/GimbalIndicator.qml")),
        toolIndicatorList = FirmwarePlugin::toolIndicators(vehicle);

        for (int i = 0; i < toolIndicatorList.size(); i++) {
            if (toolIndicatorList.at(i).toUrl().toString().contains("FlightModeIndicator.qml")) {
                toolIndicatorList[i] = QVariant::fromValue(QUrl::fromUserInput("qrc:/APM/Indicators/APMFlightModeIndicator.qml"));
                break;
            }
        }

        for (int i = 0; i < toolIndicatorList.size(); i++) {
            if (toolIndicatorList.at(i).toUrl().toString().contains("BatteryIndicator.qml")) {
                toolIndicatorList[i] = QVariant::fromValue(QUrl::fromUserInput("qrc:/APM/Indicators/APMBatteryIndicator.qml"));
                break;
            }
        }

        (void) toolIndicatorList.append(QVariant::fromValue(QUrl::fromUserInput("qrc:/toolbar/APMSupportForwardingIndicator.qml")));
    }

    return toolIndicatorList;
}
#endif
