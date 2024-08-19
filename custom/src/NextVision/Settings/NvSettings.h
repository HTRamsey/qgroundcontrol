/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "SettingsGroup.h"

class NextVisionSettings : public SettingsGroup
{
    Q_OBJECT
public:
    NextVisionSettings(QObject* parent = nullptr);
    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(telemetryValuesBarLocation)
    DEFINE_SETTINGFACT(virtualCamJoystick)
    DEFINE_SETTINGFACT(virtualCamJoystickShowZoom)
    DEFINE_SETTINGFACT(showFullScreenButton)
    DEFINE_SETTINGFACT(localPositionRoll)
    DEFINE_SETTINGFACT(localPositionPitch)
    DEFINE_SETTINGFACT(globalPositionElevation)
    DEFINE_SETTINGFACT(globalPositionAzimuth)
    DEFINE_SETTINGFACT(camJoystickDZ)
    DEFINE_SETTINGFACT(camJoystickGain)
    DEFINE_SETTINGFACT(quickViewMode)
    DEFINE_SETTINGFACT(camControlFontSize)
};
