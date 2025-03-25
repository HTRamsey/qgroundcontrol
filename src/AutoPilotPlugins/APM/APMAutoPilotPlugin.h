/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#pragma once

#include "AutoPilotPlugin.h"

class APMAirframeComponent;
class APMFlightModesComponent;
class APMRadioComponent;
class APMTuningComponent;
class APMSafetyComponent;
class APMSensorsComponent;
class APMPowerComponent;
class APMMotorComponent;
class APMCameraComponent;
class APMLightsComponent;
class APMSubFrameComponent;
class ESP8266Component;
class APMHeliComponent;
class APMRemoteSupportComponent;
class APMFollowComponent;
class Vehicle;

class APMAutoPilotPlugin : public AutoPilotPlugin
{
    Q_OBJECT

public:
    explicit APMAutoPilotPlugin(Vehicle *vehicle, QObject *parent = nullptr);
    ~APMAutoPilotPlugin();

    const QVariantList &vehicleComponents() override;
    QString prerequisiteSetup(VehicleComponent *component) const override;

protected:
    bool _incorrectParameterVersion = false; ///< true: parameter version incorrect, setup not allowed
    APMAirframeComponent *_airframeComponent = nullptr;
    APMCameraComponent *_cameraComponent = nullptr;
    APMLightsComponent *_lightsComponent = nullptr;
    APMSubFrameComponent *_subFrameComponent = nullptr;
    APMFlightModesComponent *_flightModesComponent = nullptr;
    APMPowerComponent *_powerComponent = nullptr;
    APMMotorComponent *_motorComponent = nullptr;
    APMRadioComponent *_radioComponent = nullptr;
    APMSafetyComponent *_safetyComponent = nullptr;
    APMSensorsComponent *_sensorsComponent = nullptr;
    APMTuningComponent *_tuningComponent = nullptr;
    ESP8266Component *_esp8266Component = nullptr;
    APMHeliComponent *_heliComponent = nullptr;
    APMRemoteSupportComponent *_apmRemoteSupportComponent = nullptr;
    APMFollowComponent *_followComponent = nullptr;

#if !defined(QGC_NO_SERIAL_LINK) && !defined(Q_OS_ANDROID)
private slots:
    void _checkForBadCubeBlack();
#endif

private:
    QVariantList _components;
};
