/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "SysStatusSensorInfo.h"
#include "QGCLoggingCategory.h"
#include "QGCMAVLink.h"

#include <QtCore/QDebug>

QGC_LOGGING_CATEGORY(SysStatusSensorInfoLog, "qgc.mavlink.sysstatussensorinfo")

SysStatusSensorInfo::SysStatusSensorInfo(QObject *parent)
    : QObject(parent)
{
    // qCDebug(SysStatusSensorInfoLog) << Q_FUNC_INFO << this;
}

SysStatusSensorInfo::~SysStatusSensorInfo()
{
    // qCDebug(SysStatusSensorInfoLog) << Q_FUNC_INFO << this;
}

void SysStatusSensorInfo::update(const mavlink_sys_status_t &sysStatus)
{
    bool dirty = false;

    // Walk the bits
    for (int bitPosition=0; bitPosition<32; bitPosition++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = static_cast<MAV_SYS_STATUS_SENSOR>(1 << bitPosition);
        if (sysStatus.onboard_control_sensors_present & sensorBitMask) {
            if (_sensorInfoMap.contains(sensorBitMask)) {
                SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

                const bool newEnabled = sysStatus.onboard_control_sensors_enabled & sensorBitMask;
                if (sensorInfo.enabled != newEnabled) {
                    dirty = true;
                    sensorInfo.enabled = newEnabled;
                }

                const bool newHealthy = sysStatus.onboard_control_sensors_health & sensorBitMask;
                if (sensorInfo.healthy != newHealthy) {
                    dirty = true;
                    sensorInfo.healthy = newHealthy;
                }
            } else {
                dirty = true;
                const SensorInfo sensorInfo = { !!(sysStatus.onboard_control_sensors_enabled & sensorBitMask), !!(sysStatus.onboard_control_sensors_health & sensorBitMask) };
                _sensorInfoMap[sensorBitMask] = sensorInfo;
            }
        } else {
            if (_sensorInfoMap.contains(sensorBitMask)) {
                dirty = true;
                _sensorInfoMap.remove(sensorBitMask);
            }
        }
    }

    if (dirty) {
        emit sensorInfoChanged();
    }
}

QStringList SysStatusSensorInfo::sensorNames() const
{
    QStringList rgNames;

    // List ordering is unhealthy, healthy, disabled
    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (sensorInfo.enabled && !sensorInfo.healthy) {
            rgNames.append(QGCMAVLink::mavSysStatusSensorToString(sensorBitMask));
        }
    }

    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (sensorInfo.enabled && sensorInfo.healthy) {
            rgNames.append(QGCMAVLink::mavSysStatusSensorToString(sensorBitMask));
        }
    }

    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (!sensorInfo.enabled) {
            rgNames.append(QGCMAVLink::mavSysStatusSensorToString(sensorBitMask));
        }
    }

    return rgNames;
}

QStringList SysStatusSensorInfo::sensorStatus() const
{
    QStringList rgStatus;

    // List ordering is unhealthy, healthy, disabled
    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (sensorInfo.enabled && !sensorInfo.healthy) {
            rgStatus.append(tr("Error"));
        }
    }

    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (sensorInfo.enabled && sensorInfo.healthy) {
            rgStatus.append(tr("Normal"));
        }
    }

    for (int i=0; i<_sensorInfoMap.keys().count(); i++) {
        const MAV_SYS_STATUS_SENSOR sensorBitMask = _sensorInfoMap.keys()[i];
        const SensorInfo &sensorInfo = _sensorInfoMap[sensorBitMask];

        if (!sensorInfo.enabled) {
            rgStatus.append(tr("Disabled"));
        }
    }

    return rgStatus;
}
