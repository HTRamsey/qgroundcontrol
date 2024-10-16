#include "GPUFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(GpuFactGroupLog, "qgc.custom.factgroups.gpufactgroup")

GPUFactGroup::GPUFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/GPUFact.json", parent)
    , _ipAddress(_ip)
{
    _addFact(&_connectedFact, _connectedFactName);
    _addFact(&_discoveredFact, _discoveredFactName);
    _addFact(&_psuTemperatureFact, _psuTemperatureFactName);
    _addFact(&_spoolTemperatureFact, _spoolTemperatureFactName);
    _addFact(&_psuFanDutyFact, _psuFanDutyFactName);
    _addFact(&_spoolFanDutyFact, _spoolFanDutyFactName);
    _addFact(&_motorDutyFact, _motorDutyFactName);
    _addFact(&_tetherTensionFact, _tetherTensionFactName);
    _addFact(&_tetherLengthFact, _tetherLengthFactName);
    _addFact(&_latitudeFact, _latitudeFactName);
    _addFact(&_longitudeFact, _longitudeFactName);
    _addFact(&_altitudeFact, _altitudeFactName);
    _addFact(&_pressureFact, _pressureFactName);
    _addFact(&_pressureTemperatureFact, _pressureTemperatureFactName);
    _addFact(&_outputVoltageFact, _outputVoltageFactName);
    _addFact(&_psuTempWarningFact, _psuTempWarningFactName);
    _addFact(&_psuTempCriticalFact, _psuTempCriticalFactName);
    _addFact(&_spoolTempWarningFact, _spoolTempWarningFactName);
    _addFact(&_spoolTempCriticalFact, _spoolTempCriticalFactName);

    _setUpConnectionChecker();

    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

GPUFactGroup::~GPUFactGroup()
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

void GPUFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    if ((message.sysid != _mavsys.sysid) || (message.compid != _mavsys.compid)) {
        return;
    }

    if (!_discoveredFact.rawValue().toBool()) {
        _discoveredFact.setRawValue(true);
    }

    if (!_connectedFact.rawValue().toBool())
    {
        _connectedFact.setRawValue(true);
    }

    if (!_connectionTimer.isValid()) {
        _connectionTimer.start();
    } else {
        (void) _connectionTimer.restart();
    }

    switch (message.msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        _handleHeartbeat(message);
        break;
    case MAVLINK_MSG_ID_ZAT_GPU_DATA:
        _handleZatGpuData(message);
        break;
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        _handleGlobalPosition(message);
        break;
    case MAVLINK_MSG_ID_WINCH_STATUS:
        _handleWinchStatus(message);
        break;
    case MAVLINK_MSG_ID_ONBOARD_COMPUTER_STATUS:
        _handleOnboardComputerStatus(message);
        break;
    case MAVLINK_MSG_ID_POWER_STATUS:
        _handlePowerStatus(message);
        break;
    case MAVLINK_MSG_ID_GENERATOR_STATUS:
        _handleGeneratorStatus(message);
        break;
    default:
        return;
    }

    _setTelemetryAvailable(true);
}

void GPUFactGroup::_setUpConnectionChecker()
{
    QTimer *const connectionTimer = new QTimer(this);
    connectionTimer->setInterval(1000);
    connectionTimer->setSingleShot(false);
    (void) connectionTimer->callOnTimeout(this, [this]() {
        if (_connectedFact.rawValue().toBool() && _connectionTimer.isValid() && _connectionTimer.hasExpired(_connectionTimeout)) {
            _connectedFact.setRawValue(false);
        }
    }, Qt::AutoConnection);
    connectionTimer->start();
}

void GPUFactGroup::_handleHeartbeat(const mavlink_message_t &message)
{
    mavlink_heartbeat_t data;
    mavlink_msg_heartbeat_decode(&message, &data);

    _connectedFact.setRawValue(true);
}

void GPUFactGroup::_handleZatGpuData(const mavlink_message_t &message)
{
    mavlink_zat_gpu_data_t data;
    mavlink_msg_zat_gpu_data_decode(&message, &data);

    // TODO: Get PSU Type From ZAT Message
    _psuTemperatureFact.setRawValue(data.psuTemperature);
    _psuTempWarningFact.setRawValue(_psuTemperatureFact.rawValue().toInt() >= _psuTempWarningTemp);
    _psuTempCriticalFact.setRawValue(_psuTemperatureFact.rawValue().toInt() >= _psuTempCriticalTemp);

    _spoolTemperatureFact.setRawValue(data.spoolTemperature);
    _spoolTempWarningFact.setRawValue(_spoolTemperatureFact.rawValue().toInt() >= _spoolTempWarningTemp);
    _spoolTempCriticalFact.setRawValue(_spoolTemperatureFact.rawValue().toInt() >= _spoolTempCriticalTemp);

    _psuFanDutyFact.setRawValue(data.psuFanDuty);
    _spoolFanDutyFact.setRawValue(data.spoolFanDuty);

    _motorDutyFact.setRawValue(data.motorDuty);
    // _motorDutyOffsetFact.setRawValue(data.motorDutyOffset);

    _tetherTensionFact.setRawValue(data.tetherTension);
    _tetherLengthFact.setRawValue(data.tetherLength);

    _latitudeFact.setRawValue(data.latitude);
    _longitudeFact.setRawValue(data.longitude);
    _altitudeFact.setRawValue(data.altitude);

    _pressureFact.setRawValue(data.pressure);
    _pressureTemperatureFact.setRawValue(data.pressureTemperature);

    _outputVoltageFact.setRawValue(data.outputVoltage);
}

void GPUFactGroup::_handleGlobalPosition(const mavlink_message_t &message)
{
    mavlink_global_position_int_t data;
    mavlink_msg_global_position_int_decode(&message, &data);

    /*const QGeoCoordinate position(data.lat * 1e-7, data.lon * 1e-7, data.alt * 1e-3);
    if(position != m_position.coordinate())
    {
        m_position.setCoordinate(position);
        emit positionChanged();
    }*/
}

void GPUFactGroup::_handleWinchStatus(const mavlink_message_t &message)
{
    mavlink_winch_status_t data;
    mavlink_msg_winch_status_decode(&message, &data);
}

void GPUFactGroup::_handleOnboardComputerStatus(const mavlink_message_t &message)
{
    mavlink_onboard_computer_status_t data;
    mavlink_msg_onboard_computer_status_decode(&message, &data);
}

void GPUFactGroup::_handlePowerStatus(const mavlink_message_t &message)
{
    mavlink_power_status_t data;
    mavlink_msg_power_status_decode(&message, &data);
}

void GPUFactGroup::_handleGeneratorStatus(const mavlink_message_t &message)
{
    mavlink_generator_status_t data;
    mavlink_msg_generator_status_decode(&message, &data);
}
