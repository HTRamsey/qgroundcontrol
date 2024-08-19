#include "GPUFactGroup.h"
#include "Vehicle.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(GpuFactGroupLog, "qgc.custom.factgroups.gpufactgroup")

GPUFactGroup::GPUFactGroup(QObject *parent)
    : FactGroup(1000, ":/json/GPUFact.json", parent)
{
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

    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

GPUFactGroup::~GPUFactGroup()
{
    // qCDebug(GpuFactGroupLog) << Q_FUNC_INFO << this;
}

void GPUFactGroup::handleMessage(Vehicle *vehicle, mavlink_message_t &message)
{
    Q_UNUSED(vehicle);

    switch (message.msgid) {
    case MAVLINK_MSG_ID_ZAT_GPU_DATA:
        _handleZatGpuData(message);
        break;
    default:
        return;
    }

    _setTelemetryAvailable(true);
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
