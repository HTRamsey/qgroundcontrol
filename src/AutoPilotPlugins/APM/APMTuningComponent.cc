/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "APMTuningComponent.h"
#include "Vehicle.h"

APMTuningComponent::APMTuningComponent(Vehicle *vehicle, AutoPilotPlugin *autopilot, QObject *parent)
    : VehicleComponent(vehicle, autopilot, AutoPilotPlugin::UnknownVehicleComponent, parent)
{

}

QUrl APMTuningComponent::setupSource() const
{
    QString qmlFile;

    switch (_vehicle->vehicleType()) {
    case MAV_TYPE_QUADROTOR:
    case MAV_TYPE_COAXIAL:
    case MAV_TYPE_HELICOPTER:
    case MAV_TYPE_HEXAROTOR:
    case MAV_TYPE_OCTOROTOR:
    case MAV_TYPE_TRICOPTER:
        qmlFile = QStringLiteral("qrc:/qml/APMTuningComponentCopter.qml");
        break;
    case MAV_TYPE_SUBMARINE:
        qmlFile = QStringLiteral("qrc:/qml/APMTuningComponentSub.qml");
        break;
    default:
        // No tuning panel
        break;
    }

    return QUrl::fromUserInput(qmlFile);
}
