/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#pragma once

#include <QtQmlIntegration/QtQmlIntegration>

#include "SettingsGroup.h"
#include "MAVLinkLib.h"

class APMMavlinkStreamRateSettings : public SettingsGroup
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit APMMavlinkStreamRateSettings(QObject *parent = nullptr);

    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(streamRateRawSensors)
    DEFINE_SETTINGFACT(streamRateExtendedStatus)
    DEFINE_SETTINGFACT(streamRateRCChannels)
    DEFINE_SETTINGFACT(streamRatePosition)
    DEFINE_SETTINGFACT(streamRateExtra1)
    DEFINE_SETTINGFACT(streamRateExtra2)
    DEFINE_SETTINGFACT(streamRateExtra3)

private slots:
    void _updateStreamRateRawSensors(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_RAW_SENSORS, value); }
    void _updateStreamRateExtendedStatus(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_EXTENDED_STATUS, value); }
    void _updateStreamRateRCChannels(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_RC_CHANNELS, value); }
    void _updateStreamRatePosition(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_POSITION, value); }
    void _updateStreamRateExtra1(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_EXTRA1, value); }
    void _updateStreamRateExtra2(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_EXTRA2, value); }
    void _updateStreamRateExtra3(const QVariant &value) { _updateStreamRateWorker(MAV_DATA_STREAM_EXTRA3, value); }

private:
    void _updateStreamRateWorker(MAV_DATA_STREAM mavStream, const QVariant &rateVar);
};
