#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QHostAddress>

#include "FactGroup.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(GpuFactGroupLog)

class GPUFactGroup : public FactGroup
{
    Q_OBJECT

    Q_PROPERTY(Fact* connected READ connected CONSTANT)
    Q_PROPERTY(Fact* discovered READ discovered CONSTANT)
    Q_PROPERTY(Fact* psuTemperature READ psuTemperature CONSTANT)
    Q_PROPERTY(Fact* spoolTemperature READ spoolTemperature CONSTANT)
    Q_PROPERTY(Fact* psuFanDuty READ psuFanDuty CONSTANT)
    Q_PROPERTY(Fact* spoolFanDuty READ spoolFanDuty CONSTANT)
    Q_PROPERTY(Fact* motorDuty READ motorDuty CONSTANT)
    Q_PROPERTY(Fact* tetherTension READ tetherTension CONSTANT)
    Q_PROPERTY(Fact* tetherLength READ tetherLength CONSTANT)
    Q_PROPERTY(Fact* latitude READ latitude CONSTANT)
    Q_PROPERTY(Fact* longitude READ longitude CONSTANT)
    Q_PROPERTY(Fact* altitude READ altitude CONSTANT)
    Q_PROPERTY(Fact* pressure READ pressure CONSTANT)
    Q_PROPERTY(Fact* pressureTemperature READ pressureTemperature CONSTANT)
    Q_PROPERTY(Fact* outputVoltage READ outputVoltage CONSTANT)
    Q_PROPERTY(Fact* psuTempWarning READ psuTempWarning CONSTANT)
    Q_PROPERTY(Fact* psuTempCritical READ psuTempCritical CONSTANT)
    Q_PROPERTY(Fact* spoolTempWarning READ spoolTempWarning CONSTANT)
    Q_PROPERTY(Fact* spoolTempCritical READ spoolTempCritical CONSTANT)

public:
    GPUFactGroup(QObject *parent = nullptr);
    ~GPUFactGroup();

    Fact *connected() { return &_connectedFact; }
    Fact *discovered() { return &_discoveredFact; }
    Fact *psuTemperature() { return &_psuTemperatureFact; }
    Fact *spoolTemperature() { return &_spoolTemperatureFact; }
    Fact *psuFanDuty() { return &_psuFanDutyFact; }
    Fact *spoolFanDuty() { return &_spoolFanDutyFact; }
    Fact *motorDuty() { return &_motorDutyFact; }
    Fact *tetherTension() { return &_tetherTensionFact; }
    Fact *tetherLength() { return &_tetherLengthFact; }
    Fact *latitude() { return &_latitudeFact; }
    Fact *longitude() { return &_longitudeFact; }
    Fact *altitude() { return &_altitudeFact; }
    Fact *pressure() { return &_pressureFact; }
    Fact *pressureTemperature() { return &_pressureTemperatureFact; }
    Fact *outputVoltage() { return &_outputVoltageFact; }
    Fact *psuTempWarning() { return &_psuTempWarningFact; }
    Fact *psuTempCritical() { return &_psuTempCriticalFact; }
    Fact *spoolTempWarning() { return &_spoolTempWarningFact; }
    Fact *spoolTempCritical() { return &_spoolTempCriticalFact; }

    void handleMessage(Vehicle *vehicle, mavlink_message_t &message) final;

private:
    void _handleHeartbeat(const mavlink_message_t &message);
    void _handleGlobalPosition(const mavlink_message_t &message);
    void _handleZatGpuData(const mavlink_message_t &message);
    void _handleWinchStatus(const mavlink_message_t &message);
    void _handleOnboardComputerStatus(const mavlink_message_t &message);
    void _handlePowerStatus(const mavlink_message_t &message);
    void _handleGeneratorStatus(const mavlink_message_t &message);

    void _setUpConnectionChecker();

    const QString _connectedFactName = QStringLiteral("connected");
    const QString _discoveredFactName = QStringLiteral("discovered");
    const QString _psuTemperatureFactName = QStringLiteral("psuTemperature");
    const QString _spoolTemperatureFactName = QStringLiteral("spoolTemperature");
    const QString _psuFanDutyFactName = QStringLiteral("psuFanDuty");
    const QString _spoolFanDutyFactName = QStringLiteral("spoolFanDuty");
    const QString _motorDutyFactName = QStringLiteral("motorDuty");
    const QString _tetherTensionFactName = QStringLiteral("tetherTension");
    const QString _tetherLengthFactName = QStringLiteral("tetherLength");
    const QString _latitudeFactName = QStringLiteral("latitude");
    const QString _longitudeFactName = QStringLiteral("longitude");
    const QString _altitudeFactName = QStringLiteral("altitude");
    const QString _pressureFactName = QStringLiteral("pressure");
    const QString _pressureTemperatureFactName = QStringLiteral("pressureTemperature");
    const QString _outputVoltageFactName = QStringLiteral("outputVoltage");
    const QString _psuTempWarningFactName = QStringLiteral("psuTempWarning");
    const QString _psuTempCriticalFactName = QStringLiteral("psuTempCritical");
    const QString _spoolTempWarningFactName = QStringLiteral("spoolTempWarning");
    const QString _spoolTempCriticalFactName = QStringLiteral("spoolTempCritical");

    Fact _connectedFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _connectedFactName, FactMetaData::valueTypeBool);
    Fact _discoveredFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _connectedFactName, FactMetaData::valueTypeBool);
    Fact _psuTemperatureFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _psuTemperatureFactName, FactMetaData::valueTypeInt8);
    Fact _spoolTemperatureFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _spoolTemperatureFactName, FactMetaData::valueTypeInt8);
    Fact _psuFanDutyFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _psuFanDutyFactName, FactMetaData::valueTypeUint8);
    Fact _spoolFanDutyFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _spoolFanDutyFactName, FactMetaData::valueTypeUint8);
    Fact _motorDutyFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _motorDutyFactName, FactMetaData::valueTypeUint8);
    Fact _tetherTensionFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _tetherTensionFactName, FactMetaData::valueTypeInt8);
    Fact _tetherLengthFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _tetherLengthFactName, FactMetaData::valueTypeUint8);
    Fact _latitudeFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _latitudeFactName, FactMetaData::valueTypeDouble);
    Fact _longitudeFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _longitudeFactName, FactMetaData::valueTypeDouble);
    Fact _altitudeFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _altitudeFactName, FactMetaData::valueTypeFloat);
    Fact _pressureFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _pressureFactName, FactMetaData::valueTypeUint32);
    Fact _pressureTemperatureFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _pressureTemperatureFactName, FactMetaData::valueTypeInt8);
    Fact _outputVoltageFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _outputVoltageFactName, FactMetaData::valueTypeUint16);
    Fact _psuTempWarningFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _psuTempWarningFactName, FactMetaData::valueTypeBool);
    Fact _psuTempCriticalFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _psuTempCriticalFactName, FactMetaData::valueTypeBool);
    Fact _spoolTempWarningFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _spoolTempWarningFactName, FactMetaData::valueTypeBool);
    Fact _spoolTempCriticalFact = Fact(MAV_COMP_ID_ONBOARD_COMPUTER, _spoolTempCriticalFactName, FactMetaData::valueTypeBool);

    QElapsedTimer _connectionTimer;
    QHostAddress _ipAddress;

    static constexpr int8_t _psuTempWarningTemp = 55;
    static constexpr int8_t _psuTempCriticalTemp = 70;
    static constexpr int8_t _spoolTempWarningTemp = 65;
    static constexpr int8_t _spoolTempCriticalTemp = 75;

    static constexpr int8_t _minTensionOffset = 0;
    static constexpr int8_t _maxTensionOffset = 5;

    static constexpr uint16_t _connectionTimeout = 3000;

    static constexpr uint16_t _port = 14580;
    static constexpr mavlink_system_t _mavsys = {2, MAV_COMP_ID_ONBOARD_COMPUTER};
    static constexpr const char *_ip = "192.168.1.8";
};
