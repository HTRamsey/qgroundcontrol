#pragma once

#include <QtCore/QLoggingCategory>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(DroneFactGroupLog)

class DroneFactGroup : public FactGroup
{
    Q_OBJECT

    Q_PROPERTY(Fact *spotlightEnabled READ spotlightEnabled CONSTANT)
    Q_PROPERTY(Fact *spotlightStatus READ spotlightStatus CONSTANT)
    Q_PROPERTY(Fact *beaconEnabled READ beaconEnabled CONSTANT)
    Q_PROPERTY(Fact *beaconStatus READ beaconStatus CONSTANT)
    Q_PROPERTY(Fact *remoteIdEnabled READ remoteIdEnabled CONSTANT)
    Q_PROPERTY(Fact *remoteIdStatus READ remoteIdStatus CONSTANT)
    Q_PROPERTY(Fact *navigationLightEnabled READ navigationLightEnabled CONSTANT)
    Q_PROPERTY(Fact *navigationLightStatus READ navigationLightStatus CONSTANT)
    Q_PROPERTY(Fact *antiCollisionLightEnabled READ antiCollisionLightEnabled CONSTANT)
    Q_PROPERTY(Fact *antiCollisionLightStatus READ antiCollisionLightStatus CONSTANT)
    Q_PROPERTY(Fact *antiCollisionLightState READ antiCollisionLightState CONSTANT)
    Q_PROPERTY(Fact *gimbalEnabled READ gimbalEnabled CONSTANT)
    Q_PROPERTY(Fact *gimbalType READ gimbalType CONSTANT)
    Q_PROPERTY(Fact *gimbalModel READ gimbalModel CONSTANT)
    Q_PROPERTY(qreal liteMaxPower MEMBER _liteMaxPower CONSTANT)
    Q_PROPERTY(qreal heavyMaxPower MEMBER _heavyMaxPower CONSTANT)
    Q_PROPERTY(qreal minTakeoffVoltage MEMBER _minTakeoffVoltage CONSTANT)

public:
    DroneFactGroup(QObject *parent = nullptr);
    ~DroneFactGroup();

    enum AntiCollisionLightState {
        OFF = 0,
        VISIBLE_SHORT,
        VISIBLE_LONG,
        VISIBLE_CONSTANT,
        IR_SHORT,
        IR_LONG,
        IR_CONSTANT
    };
    Q_ENUM(AntiCollisionLightState)

    Fact *spotlightEnabled() { return &_spotlightEnabledFact; }
    Fact *spotlightStatus() { return &_spotlightStatusFact; }
    Fact *beaconEnabled() { return &_beaconEnabledFact; }
    Fact *beaconStatus() { return &_beaconStatusFact; }
    Fact *remoteIdEnabled() { return &_remoteIdEnabledFact; }
    Fact *remoteIdStatus() { return &_remoteIdStatusFact; }
    Fact *navigationLightEnabled() { return &_navigationLightEnabledFact; }
    Fact *navigationLightStatus() { return &_navigationLightStatusFact; }
    Fact *antiCollisionLightEnabled() { return &_antiCollisionLightEnabledFact; }
    Fact *antiCollisionLightStatus() { return &_antiCollisionLightStatusFact; }
    Fact *antiCollisionLightState() { return &_antiCollisionLightStateFact; }
    Fact *gimbalEnabled() { return &_gimbalEnabledFact; }
    Fact *gimbalType() { return &_gimbalEnabledFact; }
    Fact *gimbalModel() { return &_gimbalEnabledFact; }

    QMap<uint8_t, uint16_t> spotlightValues() const { return _spotlightValues; }
    uint8_t beaconPort() const { return _beaconPort; }
    uint8_t remoteIdPort() const { return _remoteIdPort; }
    uint8_t navigationLightPort() const { return _navigationLightPort; }
    uint8_t antiCollisionLightPort() const { return _antiCollisionLightPort; }

    void handleMessage(Vehicle *vehicle, mavlink_message_t &message) final;

private:
    void _handleStatusText(Vehicle *vehicle, const mavlink_message_t &message);
    void _handleServoOutputRaw(const mavlink_message_t &message);

    uint8_t _calcSpotlightStatus() const;
    void _setUpPowerLimit();

    const QString _spotlightEnabledFactName = QStringLiteral("spotlightEnabled");
    const QString _spotlightStatusFactName = QStringLiteral("spotlightStatus");
    const QString _beaconEnabledFactName = QStringLiteral("beaconEnabled");
    const QString _beaconStatusFactName = QStringLiteral("beaconStatus");
    const QString _remoteIdEnabledFactName = QStringLiteral("remoteIdEnabled");
    const QString _remoteIdStatusFactName = QStringLiteral("remoteIdStatus");
    const QString _navigationLightEnabledFactName = QStringLiteral("navigationLightEnabled");
    const QString _navigationLightStatusFactName = QStringLiteral("navigationLightStatus");
    const QString _antiCollisionLightEnabledFactName = QStringLiteral("antiCollisionLightEnabled");
    const QString _antiCollisionLightStatusFactName = QStringLiteral("antiCollisionLightStatus");
    const QString _antiCollisionLightStateFactName = QStringLiteral("antiCollisionLightState");
    const QString _gimbalEnabledFactName = QStringLiteral("gimbalEnabled");
    const QString _gimbalTypeFactName = QStringLiteral("gimbalType");
    const QString _gimbalModelFactName = QStringLiteral("gimbalModel");

    Fact _spotlightEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _spotlightEnabledFactName, FactMetaData::valueTypeBool);
    Fact _spotlightStatusFact = Fact(MAV_COMP_ID_AUTOPILOT1, _spotlightStatusFactName, FactMetaData::valueTypeUint8);
    Fact _beaconEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _beaconEnabledFactName, FactMetaData::valueTypeBool);
    Fact _beaconStatusFact = Fact(MAV_COMP_ID_AUTOPILOT1, _beaconStatusFactName, FactMetaData::valueTypeBool);
    Fact _remoteIdEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _remoteIdEnabledFactName, FactMetaData::valueTypeBool);
    Fact _remoteIdStatusFact = Fact(MAV_COMP_ID_AUTOPILOT1, _remoteIdEnabledFactName, FactMetaData::valueTypeBool);
    Fact _navigationLightEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _navigationLightEnabledFactName, FactMetaData::valueTypeBool);
    Fact _navigationLightStatusFact = Fact(MAV_COMP_ID_AUTOPILOT1, _navigationLightStatusFactName, FactMetaData::valueTypeBool);
    Fact _antiCollisionLightEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _antiCollisionLightEnabledFactName, FactMetaData::valueTypeBool);
    Fact _antiCollisionLightStatusFact = Fact(MAV_COMP_ID_AUTOPILOT1, _antiCollisionLightStatusFactName, FactMetaData::valueTypeBool);
    Fact _antiCollisionLightStateFact = Fact(MAV_COMP_ID_AUTOPILOT1, _antiCollisionLightStateFactName, FactMetaData::valueTypeUint8);
    Fact _gimbalEnabledFact = Fact(MAV_COMP_ID_AUTOPILOT1, _gimbalEnabledFactName, FactMetaData::valueTypeBool);
    Fact _gimbalTypeFact = Fact(MAV_COMP_ID_AUTOPILOT1, _gimbalTypeFactName, FactMetaData::valueTypeString);
    Fact _gimbalModelFact = Fact(MAV_COMP_ID_AUTOPILOT1, _gimbalModelFactName, FactMetaData::valueTypeString);

    QMap<uint8_t, uint16_t> _spotlightValues;

    uint16_t _beaconValue = 0;
    uint8_t _beaconPort = 9;
    static constexpr uint16_t _beaconOn = 1900;
    static constexpr uint16_t _beaconOff = 1100;

    uint16_t _navigationLightValue = 0;
    uint8_t _navigationLightPort = 9;
    static constexpr uint16_t _navigationLightOn = 1900;
    static constexpr uint16_t _navigationLightOff = 1100;

    uint16_t _remoteIdValue = 0;
    uint8_t _remoteIdPort = 9;
    static constexpr uint16_t _remoteIdOn = 1900;
    static constexpr uint16_t _remoteIdOff = 1100;

    AntiCollisionLightState _antiCollisionLightState = AntiCollisionLightState::OFF;
    uint16_t _antiCollisionLightValue = 0;
    uint8_t _antiCollisionLightPort = 9;
    static constexpr uint16_t _antiCollisionLightOn = 1900;
    static constexpr uint16_t _antiCollisionLightOff = 1100;

    static constexpr float _liteMaxPower = 1800.0f;
    static constexpr float _heavyMaxPower = 3200.0f;
    static constexpr float _powerCheckTime = 2.0f;

    static constexpr float _defaultGeoFence = 10.0f;

    static constexpr float _minTakeoffVoltage = 50.0f;

    static constexpr uint16_t _port = 14570;
    static constexpr mavlink_system_t _mavsys = {1, MAV_COMP_ID_AUTOPILOT1};
    static constexpr const char *_ip = "192.168.1.5";
};