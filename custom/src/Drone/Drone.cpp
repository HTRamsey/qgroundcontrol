#include "Drone.h"
#include "DroneData.h"
#include "GPUData.h"
#include "GCSData.h"
#include <QtCore/QtMinMax>
#include <QtCore/QLoggingCategory>

static Q_LOGGING_CATEGORY(Log, "zat.comms.drone")

Drone::Drone(QObject* parent): QObject(parent)
{
    connect(this, &Drone::sendTask, MavLib::mavlib(), &MavLib::addTask, Qt::AutoConnection);
    qCDebug(Log) << Q_FUNC_INFO << this;
}

Drone::~Drone()
{
    qCDebug(Log) << Q_FUNC_INFO << this;
}

bool Drone::_isValidAltitude(float altitude)
{
    const float maxAlt = (GPUData::gpudata()->tetherLength() > 0) ? GPUData::gpudata()->tetherLength() : DroneData::dronedata()->fenceMaxAlt();
    if(altitude > maxAlt * 1.02f) return false;

    constexpr float minAlt = DroneData::minGuidedAlt();
    if(altitude < minAlt * .98f) return false;

    return true;
}

bool Drone::_readyForArm()
{
    if(DroneData::dronedata()->armed()) return false;
    if(!DroneData::dronedata()->highPowerOn()) return false;
    // requestArmAuthorization()
    return true;
}

void Drone::_arm(bool arm)
{
    if(!_readyForArm()) return;
    qCDebug(Log, "ARM");
    emit sendTask(new ArmTask(arm));
}

bool Drone::_readyForGuided()
{
    if(DroneData::dronedata()->guidedMode()) return false;
    return true;
}

void Drone::_guided()
{
    if(!_readyForGuided()) return;
    qCDebug(Log, "GUIDED");
    emit sendTask(new GuidedTask());
}

bool Drone::readyForRtl()
{
    if(!DroneData::dronedata()->inAir()) return false;
    if(DroneData::dronedata()->landing()) return false;
    if(DroneData::dronedata()->rtlMode()) return false;
    return true;
}

void Drone::rtl()
{
    if(!readyForRtl()) return;
    qCDebug(Log, "RTL");
    emit sendTask(new ReturnToLaunchTask());
}

bool Drone::readyForLand()
{
    if(!DroneData::dronedata()->inAir()) return false;
    if(DroneData::dronedata()->landing()) return false;
    if(DroneData::dronedata()->landMode()) return false;
    return true;
}

void Drone::land()
{
    if(!readyForLand()) return;
    qCDebug(Log, "LAND");
    emit sendTask(new LandTask());
}

bool Drone::readyForBrake()
{
    if(!DroneData::dronedata()->flying()) return false;
    if(DroneData::dronedata()->brakeMode()) return false;
    return true;
}

void Drone::brake()
{
    if(!readyForBrake()) return;
    qCDebug(Log, "BRAKE");
    emit sendTask(new BrakeTask());
}

bool Drone::readyForTakeoff()
{
    if(DroneData::dronedata()->armed()) return false;
    if(!DroneData::dronedata()->readyToFly()) return false;
    if(DroneData::dronedata()->takingOff()) return false;
    if(!DroneData::dronedata()->coordinate().isValid()) return false;
    if(DroneData::dronedata()->coordinate().type() != QGeoCoordinate::CoordinateType::Coordinate3D) return false;
    return true;
}

void Drone::takeoff(float altitude)
{
    if(!readyForTakeoff()) return;

    if(!_isValidAltitude(altitude)) return;

    _guided();

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    m_count = 0;
    timer->callOnTimeout([this, timer, altitude]()
    {
        if(m_count++ == 5)
        {
            timer->deleteLater();
        }
        else if(DroneData::dronedata()->armed())
        {
            qCDebug(Log, "TAKEOFF");
            emit sendTask(new TakeoffTask(altitude));
            timer->deleteLater();
        }
        else if(DroneData::dronedata()->guidedMode())
        {
            _arm(true);
            timer->start();
        }
        else
        {
            timer->deleteLater();
        }
    });
    timer->start();
}

bool Drone::readyForChangeAltitude()
{
    if(!DroneData::dronedata()->inAir()) return false;
    return true;
}

void Drone::changeAltitude(float altitude)
{
    if(!readyForChangeAltitude()) return;

    if(!_isValidAltitude(altitude)) return;

    const float diff = altitude - DroneData::dronedata()->coordinate().altitude();

    _guided();

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    timer->callOnTimeout([this, timer, diff]()
    {
        if(DroneData::dronedata()->guidedMode())
        {
            qCDebug(Log, "ALTITUDE");
            emit sendTask(new ChangeAltitudeTask(DroneData::sysID(), DroneData::compID(), diff));
            timer->deleteLater();
        }
        else
        {
            timer->deleteLater();
        }
    });
    timer->start();
}

bool Drone::readyForReposition()
{
    if(!DroneData::dronedata()->inAir()) return false;
    if(DroneData::dronedata()->followMode()) return false;
    // if(relative_alt < minGuidedAlt() - .5) return false
    return true;
}

void Drone::reposition(const QGeoCoordinate& gotoCoord)
{
    if(!readyForReposition()) return;

    const QGeoCoordinate currentCoord = DroneData::dronedata()->coordinate();
    const QGeoCoordinate targetCoord = QGeoCoordinate(gotoCoord.latitude(), gotoCoord.longitude(), currentCoord.altitude());

    if(!currentCoord.isValid() || !targetCoord.isValid()) return;
    if(currentCoord.type() != QGeoCoordinate::CoordinateType::Coordinate3D || targetCoord.type() != QGeoCoordinate::CoordinateType::Coordinate3D) return;
    if(!DroneData::dronedata()->geofence().contains(targetCoord)) return;
    if(currentCoord.distanceTo(targetCoord) > (DroneData::dronedata()->geofence().radius() * 2)) return;
    // TODO: check distance between point1 and point2 to get angled distance (hypotenuse)

    _guided();

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    timer->callOnTimeout([this, timer, targetCoord]()
    {
        if(DroneData::dronedata()->guidedMode())
        {
            qCDebug(Log, "REPOSITION");
            emit sendTask(new RepositionTask(targetCoord.latitude(), targetCoord.longitude(), targetCoord.altitude()));
            timer->deleteLater();
        }
        else
        {
            timer->deleteLater();
        }
    });
    timer->start();
}

bool Drone::readyForChangeHeading()
{
    if(!DroneData::dronedata()->inAir()) return false;
    if(DroneData::dronedata()->followMode()) return false;
    // if(relative_alt < minGuidedAlt() - .5) return false
    return true;
}

void Drone::changeHeading(const QGeoCoordinate& headingCoord)
{
    if(!readyForChangeHeading()) return;

    _guided();

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(true);
    timer->callOnTimeout([this, timer, headingCoord]()
    {
        if(DroneData::dronedata()->guidedMode())
        {
            const float degrees = DroneData::dronedata()->coordinate().azimuthTo(headingCoord);
            const float currentHeading = DroneData::dronedata()->heading();
            float diff = degrees - currentHeading;
            if(diff < -180)
            {
                diff += 360;
            }
            if(diff > 180)
            {
                diff -= 360;
            }

            constexpr bool relative = true;
            const float maxYawRate = DroneData::dronedata()->maxYawRate();
            const int8_t direction = (relative && diff > 0) ? 1 : -1;

            qCDebug(Log, "HEADING");
            emit sendTask(new ChangeHeadingTask(qAbs(diff), maxYawRate, direction, relative));
            timer->deleteLater();
        }
        else
        {
            timer->deleteLater();
        }
    });
    timer->start();
}

bool Drone::readyForAdjustLights()
{
    if(!DroneData::dronedata()->lightsEnabled()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    // if(relative_alt < minGuidedAlt() - .5) return false
    return true;
}

void Drone::adjustLights(uint8_t intensity)
{
    if(!readyForAdjustLights()) return;

    uint8_t current = qBound(0U, DroneData::dronedata()->lightsStatus(), 100U);
    int8_t diff;
    do {
        diff = static_cast<int8_t>(current) - static_cast<int8_t>(intensity);
        if(diff > 5)
        {
            current -= 5;
            m_lightTargets.append(current);
        }
        else if(diff < -5)
        {
            current += 5;
            m_lightTargets.append(current);
        }
        else
        {
            // current = intensity;
            m_lightTargets.append(intensity);
            break;
        }
    } while (diff != 0);

    QTimer* timer = new QTimer(this);
    timer->setInterval(25);
    timer->setSingleShot(true);
    (void) timer->callOnTimeout([this, timer]()
    {
        if(!m_lightTargets.isEmpty())
        {
            const uint16_t pwm = 1000 + (m_lightTargets.takeFirst() * 9);
            for(const uint8_t port: DroneData::dronedata()->lights().keys())
            {
                emit sendTask(new SetServoTask(port, pwm));
            }
            timer->start();
        }
        else
        {
            timer->deleteLater();
        }
    });
    timer->start();
}

bool Drone::readyForToggleBeacon()
{
    if(!DroneData::dronedata()->beaconEnabled()) return false;
    return true;
}

void Drone::toggleBeacon()
{
    if(!readyForToggleBeacon()) return;

    emit sendTask(new SetServoTask(DroneData::dronedata()->beaconPort(), DroneData::dronedata()->beaconStatus() ? 1100U : 1900U));
}

bool Drone::readyForToggleRemoteID()
{
    if(!DroneData::dronedata()->remoteIDEnabled()) return false;
    return true;
}

void Drone::toggleRemoteID()
{
    if(!readyForToggleRemoteID()) return;

    emit sendTask(new SetServoTask(DroneData::dronedata()->remoteIDPort(), DroneData::dronedata()->remoteIDStatus() ? 1100U : 1900U));
}

bool Drone::readyForToggleNavigationLights()
{
    if(!DroneData::dronedata()->navigationLightsEnabled()) return false;
    return true;
}

void Drone::toggleNavigationLights()
{
    if(!readyForToggleNavigationLights()) return;

    emit sendTask(new SetServoTask(DroneData::dronedata()->navigationLightsPort(), DroneData::dronedata()->navigationLightsStatus() ? 1100U : 1900U));
}

bool Drone::readyForSetAntiCollisionLightState()
{
    if(!DroneData::dronedata()->antiCollisionLightEnabled()) return false;
    return true;
}

void Drone::setAntiCollisionLightState(DroneData::AntiCollisionLightState state)
{
    if(!readyForSetAntiCollisionLightState()) return;

    const uint8_t port = DroneData::dronedata()->antiCollisionLightPort();
    constexpr uint16_t off = 1100;
    constexpr uint16_t on = 1900;

    QTimer* const timer = new QTimer(this);
    timer->setSingleShot(true);
    switch(state)
    {
        using enum DroneData::AntiCollisionLightState;
        case OFF:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(3500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->deleteLater();
            });
            timer->start();
            break;

        case VISIBLE_SHORT:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(3500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->deleteLater();
            });
            timer->start();
            break;

        case VISIBLE_LONG:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->setInterval(500);
                (void) timer->callOnTimeout([this, timer]()
                {
                    setAntiCollisionLightState(VISIBLE_CONSTANT);
                    timer->deleteLater();
                });
                timer->start();
            });
            timer->start();
            break;

        case VISIBLE_CONSTANT:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->setInterval(500);
                (void) timer->callOnTimeout([this, timer]()
                {
                    setAntiCollisionLightState(IR_SHORT);
                    timer->deleteLater();
                });
                timer->start();
            });
            timer->start();
            break;

        case IR_SHORT:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->deleteLater();
            });
            timer->start();
            break;

        case IR_LONG:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(500);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->setInterval(500);
                (void) timer->callOnTimeout([this, timer]()
                {
                    setAntiCollisionLightState(IR_CONSTANT);
                    timer->deleteLater();
                });
                timer->start();
            });
            timer->start();
            break;

        case IR_CONSTANT:
            emit sendTask(new SetServoTask(port, on));
            timer->setInterval(100);
            (void) timer->callOnTimeout([this, timer, port]()
            {
                emit sendTask(new SetServoTask(port, off));
                timer->setInterval(500);
                (void) timer->callOnTimeout([this, timer]()
                {
                    setAntiCollisionLightState(OFF);
                    timer->deleteLater();
                });
                timer->start();
            });
            timer->start();
            break;

        default:
            break;
    }

    DroneData::dronedata()->setAntiCollisionLightState(state);
}

bool Drone::readyForReboot()
{
    if(DroneData::dronedata()->armed()) return false;
    if(!DroneData::dronedata()->onGround()) return false;
    return true;
}

void Drone::reboot()
{
    if(!readyForReboot()) return;
    qCDebug(Log, "REBOOT");
    emit sendTask(new RebootTask());
}

bool Drone::readyForFollow()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    if(DroneData::dronedata()->followMode()) return false;
    return true;
}

void Drone::follow()
{
    if(!readyForFollow()) return;

    emit sendTask(new FollowTask());
}

bool Drone::readyForSetFollowTarget()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    return true;
}

void Drone::setFollowTarget(uint8_t target)
{
    if(!readyForSetFollowTarget()) return;

    emit sendTask(new SetFollowTargetTask(target));
    getParam("FOLL_SYSID", MAV_PARAM_TYPE_INT16, -1);
}

bool Drone::readyForFollowLand()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    return true;
}

void Drone::followLand()
{
    if(!readyForFollowLand()) return;

    // QVector3D?
    const QGeoCoordinate follow_land_target = DroneData::dronedata()->followLandLocation();
    setFollowOffsets(follow_land_target);
    setFollowAltitude(follow_land_target.altitude());
    land();
}

bool Drone::readyForSetFollowOffsets()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    return true;
}

void Drone::setFollowOffsets(const QGeoCoordinate& targetCoord)
{
    if(!readyForSetFollowOffsets()) return;

    if(!targetCoord.isValid()) return;

    QGeoCoordinate sourceCoord;
    if(DroneData::dronedata()->followSysId() == GCSData::sysID())
    {
        sourceCoord = GCSData::gcsdata()->position().coordinate();
    }
    else if(DroneData::dronedata()->followSysId() == GPUData::sysID())
    {
        sourceCoord = GPUData::gpudata()->position().coordinate();
    }
    else
    {
        return;
    }

    if(!sourceCoord.isValid()) return;

    const double azimuth = sourceCoord.azimuthTo(targetCoord);
    const double distance = sourceCoord.distanceTo(targetCoord);

    const double opposite = qSin(azimuth) * distance;
    const double adjacent = qCos(azimuth) * distance;

    // TODO: Use relative coords
    setParam("FOLL_OFS_X", MAV_PARAM_TYPE_REAL32, opposite);
    setParam("FOLL_OFS_Y", MAV_PARAM_TYPE_REAL32, adjacent);
}

bool Drone::readyForSetFollowAltitude()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    return true;
}

void Drone::setFollowAltitude(float altitude)
{
    if(!readyForSetFollowAltitude()) return;

    if(!_isValidAltitude(altitude)) return;

    setParam("FOLL_OFS_Z", MAV_PARAM_TYPE_REAL32, -altitude);
}

bool Drone::readyForSetFollowLandOffsets()
{
    if(!DroneData::dronedata()->followEnable()) return false;
    if(!DroneData::dronedata()->inAir()) return false;
    return true;
}

void Drone::setFollowLandOffsets(const QGeoCoordinate& target)
{
    if(!readyForSetFollowLandOffsets()) return;

    DroneData::dronedata()->setFollowLandLocation(target);
}

void Drone::setParam(const QString &param_id, uint8_t param_type, QVariant value)
{
    emit sendTask(
        new SetParamTask(
            static_cast<uint8_t>(DroneData::sysID()),
            static_cast<uint8_t>(DroneData::compID()),
            static_cast<const QString>(param_id),
            static_cast<MAV_PARAM_TYPE>(param_type),
            static_cast<QVariant>(value)
        )
    );
}

void Drone::getParam(const QString &param_id, uint8_t param_type, int16_t param_index)
{
    emit sendTask(
        new GetParamTask(
            static_cast<uint8_t>(DroneData::sysID()),
            static_cast<uint8_t>(DroneData::compID()),
            static_cast<const QString>(param_id),
            static_cast<MAV_PARAM_TYPE>(param_type),
            static_cast<int16_t>(param_index)
        )
    );
}
