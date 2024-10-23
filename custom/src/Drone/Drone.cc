#include "Drone.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QtMinMax>
#include <QtCore/QLoggingCategory>

QGC_LOGGING_CATEGORY(DroneLog, "qgc.custom.drone.drone")

Drone::Drone(DroneFactGroup *droneFactGroup, QObject *parent)
    : QObject(parent)
    , _droneFactGroup(droneFactGroup)
{
    // qCDebug(DroneLog) << Q_FUNC_INFO << this;
}

Drone::~Drone()
{
    // qCDebug(DroneLog) << Q_FUNC_INFO << this;
}

void Drone::_setServo(Vehicle *vehicle, uint8_t ch, uint16_t pwm)
{
    vehicle->sendMavCommand(vehicle->defaultComponentId(), MAV_CMD_DO_SET_SERVO, true, ch, pwm);
}

bool Drone::readyForAdjustLights(Vehicle *vehicle)
{
    if (!_droneFactGroup->spotlightEnabled()->rawValue().toBool()) return false;
    // if (!vehicle->flying()) return false;
    // if (vehicle->vehicleFactGroup()->altitudeRelative()->rawValue() < (vehicle->minimumTakeoffAltitudeMeters() - .5)) return false;
    return true;
}

void Drone::adjustLights(Vehicle *vehicle, uint8_t intensity)
{
    if (!readyForAdjustLights(vehicle)) return;

    const uint16_t pwm = 1000 + (intensity * 9);
    uint8_t count = 1;
    for (const uint8_t port: _droneFactGroup->spotlightValues().keys()) {
        QTimer* const porttimer = new QTimer(this);
        porttimer->setInterval(50 * count);
        porttimer->setSingleShot(true);
        (void) porttimer->callOnTimeout([this, vehicle, porttimer, pwm, port]() {
            _setServo(vehicle, port, pwm);
            porttimer->deleteLater();
        });
        porttimer->start();
        count++;
    }

    /*uint8_t current = qBound(0U, _droneFactGroup->spotlightStatus()->rawValue().toUInt(), 100U);
    int8_t diff;
    do {
        diff = static_cast<int8_t>(current) - static_cast<int8_t>(intensity);
        if(diff > 5) {
            current -= 5;
            _lightTargets.append(current);
        } else if(diff < -5) {
            current += 5;
            (void) _lightTargets.append(current);
        } else {
            // current = intensity;
            (void) _lightTargets.append(intensity);
            break;
        }
    } while (diff != 0);

    QTimer* const timer = new QTimer(this);
    timer->setInterval(100);
    timer->setSingleShot(true);
    (void) timer->callOnTimeout([this, vehicle, timer]() {
        // if (!_lightTargets.isEmpty()) {
            const uint16_t pwm = 1000 + (_lightTargets.takeFirst() * 9);
            for (const uint8_t port: _droneFactGroup->spotlightValues().keys()) {
                QTimer* const porttimer = new QTimer(this);
                porttimer->setInterval(50);
                porttimer->setSingleShot(true);
                (void) porttimer->callOnTimeout([this, vehicle, porttimer, pwm, port]() {
                    _setServo(vehicle, port, pwm);
                    porttimer->deleteLater();
                });
                porttimer->start();
            }
            timer->start();
        } else {
            timer->deleteLater();
        }
    });
    timer->start();*/
}

bool Drone::readyForToggleBeacon()
{
    if (!_droneFactGroup->beaconEnabled()->rawValue().toBool()) return false;

    return true;
}

void Drone::toggleBeacon(Vehicle *vehicle)
{
    if (!readyForToggleBeacon()) return;

    _setServo(vehicle, _droneFactGroup->beaconPort(), _droneFactGroup->beaconStatus()->rawValue().toBool() ? _servoOff : _servoOn);
}

bool Drone::readyForToggleRemoteId()
{
    if (!_droneFactGroup->remoteIdEnabled()->rawValue().toBool()) return false;

    return true;
}

void Drone::toggleRemoteId(Vehicle *vehicle)
{
    if (!readyForToggleRemoteId()) return;

    _setServo(vehicle, _droneFactGroup->remoteIdPort(), _droneFactGroup->remoteIdStatus()->rawValue().toBool() ? _servoOff : _servoOn);
}

bool Drone::readyForToggleNavigationLight()
{
    if (!_droneFactGroup->navigationLightEnabled()->rawValue().toBool()) return false;

    return true;
}

void Drone::toggleNavigationLight(Vehicle *vehicle)
{
    if (!readyForToggleNavigationLight()) return;

    _setServo(vehicle, _droneFactGroup->navigationLightPort(), _droneFactGroup->navigationLightStatus()->rawValue().toBool() ? _servoOff : _servoOn);
}

bool Drone::readyForSetAntiCollisionLightState()
{
    if (!_droneFactGroup->antiCollisionLightEnabled()->rawValue().toBool()) return false;

    return true;
}

void Drone::setAntiCollisionLightState(Vehicle *vehicle, DroneFactGroup::AntiCollisionLightState state)
{
    if (!readyForSetAntiCollisionLightState()) return;

    const uint8_t port = _droneFactGroup->antiCollisionLightPort();

    QTimer* const timer = new QTimer(this);
    timer->setSingleShot(true);
    switch(state)
    {
    case DroneFactGroup::AntiCollisionLightState::OFF:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(3500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->deleteLater();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::VISIBLE_SHORT:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(3500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->deleteLater();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::VISIBLE_LONG:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, vehicle, timer]() {
                setAntiCollisionLightState(vehicle, DroneFactGroup::AntiCollisionLightState::VISIBLE_CONSTANT);
                timer->deleteLater();
            });
            timer->start();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::VISIBLE_CONSTANT:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, vehicle, timer]() {
                setAntiCollisionLightState(vehicle, DroneFactGroup::AntiCollisionLightState::IR_SHORT);
                timer->deleteLater();
            });
            timer->start();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::IR_SHORT:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->deleteLater();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::IR_LONG:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(500);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, vehicle, timer]() {
                setAntiCollisionLightState(vehicle, DroneFactGroup::AntiCollisionLightState::IR_CONSTANT);
                timer->deleteLater();
            });
            timer->start();
        });
        timer->start();
        break;
    case DroneFactGroup::AntiCollisionLightState::IR_CONSTANT:
        _setServo(vehicle, port, _servoOn);
        timer->setInterval(100);
        (void) timer->callOnTimeout([this, vehicle, timer, port]() {
            _setServo(vehicle, port, _servoOff);
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, vehicle, timer]() {
                setAntiCollisionLightState(vehicle, DroneFactGroup::AntiCollisionLightState::OFF);
                timer->deleteLater();
            });
            timer->start();
        });
        timer->start();
        break;
    default:
        break;
    }

    _droneFactGroup->antiCollisionLightState()->setRawValue(state);
}
