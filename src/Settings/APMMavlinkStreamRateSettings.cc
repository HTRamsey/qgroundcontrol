/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "APMMavlinkStreamRateSettings.h"
#include "Vehicle.h"
#include "MultiVehicleManager.h"

DECLARE_SETTINGGROUP(APMMavlinkStreamRate, "APMMavlinkStreamRate")
{
    (void) connect(streamRateRawSensors(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateRawSensors);
    (void) connect(streamRateExtendedStatus(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateExtendedStatus);
    (void) connect(streamRateRCChannels(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateRCChannels);
    (void) connect(streamRatePosition(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRatePosition);
    (void) connect(streamRateExtra1(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateExtra1);
    (void) connect(streamRateExtra2(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateExtra2);
    (void) connect(streamRateExtra3(), &Fact::rawValueChanged, this, &APMMavlinkStreamRateSettings::_updateStreamRateExtra3);
}

DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateRawSensors)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateExtendedStatus)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateRCChannels)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRatePosition)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateExtra1)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateExtra2)
DECLARE_SETTINGSFACT(APMMavlinkStreamRateSettings, streamRateExtra3)

void APMMavlinkStreamRateSettings::_updateStreamRateWorker(MAV_DATA_STREAM mavStream, const QVariant &rateVar)
{
    Vehicle *activeVehicle = MultiVehicleManager::instance()->activeVehicle();

    if (activeVehicle) {
        bool valid;
        const int streamRate = rateVar.toInt(&valid);
        if (valid && (streamRate >= 0)) {
            activeVehicle->requestDataStream(mavStream, static_cast<uint16_t>(streamRate));
        }
    }
}
