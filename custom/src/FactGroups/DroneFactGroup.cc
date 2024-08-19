#include "DroneFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(DroneFactGroupLog, "qgc.custom.factgroups.dronefactgroup")

DroneFactGroup::DroneFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/DroneFact.json", parent)
{
    _addFact(&_spotlightEnabledFact, _spotlightEnabledFactName);
    _addFact(&_spotlightStatusFact, _spotlightStatusFactName);
    _addFact(&_beaconEnabledFact, _beaconEnabledFactName);
    _addFact(&_beaconStatusFact, _beaconStatusFactName);
    _addFact(&_remoteIdEnabledFact, _remoteIdEnabledFactName);
    _addFact(&_remoteIdStatusFact, _remoteIdStatusFactName);
    _addFact(&_navigationLightEnabledFact, _navigationLightEnabledFactName);
    _addFact(&_navigationLightStatusFact, _navigationLightStatusFactName);
    _addFact(&_antiCollisionLightEnabledFact, _antiCollisionLightEnabledFactName);
    _addFact(&_antiCollisionLightStatusFact, _antiCollisionLightStatusFactName);

    // qCDebug(DroneFactGroupLog) << Q_FUNC_INFO << this;
}

DroneFactGroup::~DroneFactGroup()
{
    // qCDebug(DroneFactGroupLog) << Q_FUNC_INFO << this;
}

void DroneFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_STATUSTEXT:
        _handleStatusText(vehicle, message);
        break;
    default:
        return;
    }

    _setTelemetryAvailable(true);
}

void DroneFactGroup::_handleStatusText(Vehicle *vehicle, const mavlink_message_t &message)
{
    mavlink_statustext_t data;
    mavlink_msg_statustext_decode(&message, &data);

    const QString text = QString::fromStdString(data.text);
    if (text.contains("Payload", Qt::CaseInsensitive)) {
        const QStringList parts = text.split(": ");
        QString payload = parts[1];
        static const QRegularExpression regexp("[()]");
        payload = payload.remove(regexp);
        if (payload.contains("Gimbal", Qt::CaseInsensitive)) {
            /* const QStringList params = payload.remove(QChar(' ')).split(",");
            const QString gimbalType = params.at(1);
            const QString gimbalModel = params.at(2);
            if (!_gimbalEnabledFact.rawValue().toBool()) {
                _gimbalEnabledFact.setRawValue(true);
                _gimbalTypeFact.setRawValue(gimbalType);
                _gimbalModelFact.setRawValue(gimbalModel);
            } */
        } else if (payload.contains("Servo", Qt::CaseInsensitive)) {
            const QStringList params = payload.remove(QChar(' ')).split(",");
            const QString servoName = params.at(1);
            const QString servoChannel = params.at(2);
            bool ok = false;
            const uint8_t ch = servoChannel.toUInt(&ok);
            if (ok) {
                if (servoName.contains("Beacon", Qt::CaseInsensitive)) {
                    if (!_beaconEnabledFact.rawValue().toBool()) {
                        _beaconPort = ch;
                        _beaconEnabledFact.setRawValue(true);
                        _beaconValue = 0;
                    }
                } else if (servoName.contains("NavigationLight", Qt::CaseInsensitive)) {
                    if (!_navigationLightEnabledFact.rawValue().toBool()) {
                        _navigationLightPort = ch;
                        _navigationLightEnabledFact.setRawValue(true);
                        _navigationLightValue = 0;
                    }
                } else if (servoName.contains("AntiCollisionLight", Qt::CaseInsensitive)) {
                    if (!_antiCollisionLightEnabledFact.rawValue().toBool()) {
                        _antiCollisionLightPort = ch;
                        _antiCollisionLightEnabledFact.setRawValue(true);
                        _antiCollisionLightValue = 0;
                        _antiCollisionLightState = AntiCollisionLightState::OFF;
                        vehicle->sendMavCommand(vehicle->defaultComponentId(), MAV_CMD_DO_SET_SERVO, true, _antiCollisionLightPort, _antiCollisionLightOff);
                    }
                } else if (servoName.contains("RemoteId", Qt::CaseInsensitive)) {
                    if (!_remoteIdEnabledFact.rawValue().toBool()) {
                        _remoteIdPort = ch;
                        _remoteIdEnabledFact.setRawValue(true);
                        _remoteIdValue = 0;
                    }
                } else if (servoName.contains("Light", Qt::CaseInsensitive)) {
                    if (!_spotlightValues.contains(ch)) {
                        (void) _spotlightValues.insert(ch, 0);
                    }
                    if (!_spotlightEnabledFact.rawValue().toBool()) {
                        _spotlightEnabledFact.setRawValue(true);
                    }
                }
            }
        } else if (payload.contains("Relay", Qt::CaseInsensitive)) {
            const QStringList params = payload.remove(QChar(' ')).split(",");
            const QString relayName = params.at(1);
            const QString relayChannel = params.at(2);
            bool ok = false;
            const uint8_t ch = relayChannel.toUInt(&ok);
            if (ok) {

            }
        }
    }
}

void DroneFactGroup::_handleServoOutputRaw(const mavlink_message_t &msg)
{
    mavlink_servo_output_raw_t data;
    mavlink_msg_servo_output_raw_decode(&msg, &data);

    // TODO: get params SERVO9_MIN SERVO9_MAX for each received servo

    if (_spotlightEnabledFact.rawValue().toBool()) {
        const QList<uint8_t> ports = _spotlightValues.keys();
        const QList<uint16_t> values = _spotlightValues.values();
        for (qsizetype i = 0; i < ports.size(); ++i) {
            uint16_t servo_raw = 0;
            switch (ports.at(i)) {
            case 9: servo_raw = data.servo9_raw; break;
            case 10: servo_raw = data.servo10_raw; break;
            case 11: servo_raw = data.servo11_raw; break;
            case 12: servo_raw = data.servo12_raw; break;
            case 13: servo_raw = data.servo13_raw; break;
            case 14: servo_raw = data.servo14_raw; break;
            case 15: servo_raw = data.servo15_raw; break;
            case 16: servo_raw = data.servo16_raw; break;
            default: break;
            }

            if (servo_raw > 0 && servo_raw != values.at(i)) {
                _spotlightValues[ports.at(i)] = servo_raw;
            }
        }

        _spotlightStatusFact.setRawValue(_calcSpotlightStatus());
    }

    if (_beaconEnabledFact.rawValue().toBool()) {
        uint16_t servo_raw = 0;
        switch (_beaconPort) {
        case 9: servo_raw = data.servo9_raw; break;
        case 10: servo_raw = data.servo10_raw; break;
        case 11: servo_raw = data.servo11_raw; break;
        case 12: servo_raw = data.servo12_raw; break;
        case 13: servo_raw = data.servo13_raw; break;
        case 14: servo_raw = data.servo14_raw; break;
        case 15: servo_raw = data.servo15_raw; break;
        case 16: servo_raw = data.servo16_raw; break;
        default: break;
        }

        if ((servo_raw > 0) && (servo_raw != _beaconValue)) {
            _beaconValue = servo_raw;
        }
    }

    if (_navigationLightEnabledFact.rawValue().toBool()) {
        uint16_t servo_raw = 0;
        switch (_navigationLightPort) {
            case 9: servo_raw = data.servo9_raw; break;
            case 10: servo_raw = data.servo10_raw; break;
            case 11: servo_raw = data.servo11_raw; break;
            case 12: servo_raw = data.servo12_raw; break;
            case 13: servo_raw = data.servo13_raw; break;
            case 14: servo_raw = data.servo14_raw; break;
            case 15: servo_raw = data.servo15_raw; break;
            case 16: servo_raw = data.servo16_raw; break;
            default: break;
        }

        if ((servo_raw > 0) && (servo_raw != _navigationLightValue)) {
            _navigationLightValue = servo_raw;
        }
    }

    if (_remoteIdEnabledFact.rawValue().toBool()) {
        uint16_t servo_raw = 0;
        switch (_remoteIdPort) {
        case 9: servo_raw = data.servo9_raw; break;
        case 10: servo_raw = data.servo10_raw; break;
        case 11: servo_raw = data.servo11_raw; break;
        case 12: servo_raw = data.servo12_raw; break;
        case 13: servo_raw = data.servo13_raw; break;
        case 14: servo_raw = data.servo14_raw; break;
        case 15: servo_raw = data.servo15_raw; break;
        case 16: servo_raw = data.servo16_raw; break;
        default: break;
        }

        if ((servo_raw > 0) && (servo_raw != _remoteIdValue)) {
            _remoteIdValue = servo_raw;
        }
    }

    if (_antiCollisionLightEnabledFact.rawValue().toBool()) {
        uint16_t servo_raw = 0;
        switch (_antiCollisionLightPort) {
        case 9: servo_raw = data.servo9_raw; break;
        case 10: servo_raw = data.servo10_raw; break;
        case 11: servo_raw = data.servo11_raw; break;
        case 12: servo_raw = data.servo12_raw; break;
        case 13: servo_raw = data.servo13_raw; break;
        case 14: servo_raw = data.servo14_raw; break;
        case 15: servo_raw = data.servo15_raw; break;
        case 16: servo_raw = data.servo16_raw; break;
        default: break;
        }

        if ((servo_raw > 0) && (servo_raw != _antiCollisionLightValue)) {
            _antiCollisionLightValue = servo_raw;
        }
    }
}

uint8_t DroneFactGroup::_calcSpotlightStatus() const
{
    uint16_t lights_status = 0;
    if (!_spotlightValues.isEmpty()) {
        const QList<uint16_t> lightsValues = _spotlightValues.values();
        uint16_t total = 0;
        for (const uint16_t value: lightsValues) {
            total += value;
        }
        const uint16_t avg = total / lightsValues.size();
        lights_status = (qBound(1000U, avg, 1900U) - 1000) / 9;
    }
    return lights_status;
}
