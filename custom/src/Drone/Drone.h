#pragma once

#include <QtCore/QLoggingCategory>

#include "DroneFactGroup.h"

class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(DroneLog)

class Drone : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_SINGLETON

public:
    explicit Drone(DroneFactGroup *droneFactGroup, QObject *parent = nullptr);
    ~Drone();

    bool readyForAdjustLights(Vehicle *vehicle);
    Q_INVOKABLE void adjustLights(Vehicle *vehicle, uint8_t intensity);
    bool readyForToggleBeacon();
    Q_INVOKABLE void toggleBeacon(Vehicle *vehicle);
    bool readyForToggleRemoteId();
    Q_INVOKABLE void toggleRemoteId(Vehicle *vehicle);
    bool readyForToggleNavigationLight();
    Q_INVOKABLE void toggleNavigationLight(Vehicle *vehicle);
    bool readyForSetAntiCollisionLightState();
    Q_INVOKABLE void setAntiCollisionLightState(Vehicle *vehicle, DroneFactGroup::AntiCollisionLightState state);

private:
    void _setServo(Vehicle *vehicle, uint8_t ch, uint16_t pwm);

    DroneFactGroup *_droneFactGroup = nullptr;
    QList<int8_t> _lightTargets;

    static constexpr uint16_t _servoOn = 1900;
    static constexpr uint16_t _servoOff = 1100;
};
