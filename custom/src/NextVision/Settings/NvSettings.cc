#include "NvSettings.h"

#include <QQmlEngine>
#include <QtQml>

DECLARE_SETTINGGROUP(NextVision, "NextVision")
{
    qmlRegisterUncreatableType<NextVisionSettings>("QGroundControl.SettingsManager", 1, 0, "NextVisionSettings", "Reference only");
}

DECLARE_SETTINGSFACT(NextVisionSettings, telemetryValuesBarLocation)
DECLARE_SETTINGSFACT(NextVisionSettings, virtualCamJoystick)
DECLARE_SETTINGSFACT(NextVisionSettings, virtualCamJoystickShowZoom)
DECLARE_SETTINGSFACT(NextVisionSettings, showFullScreenButton)
DECLARE_SETTINGSFACT(NextVisionSettings, localPositionRoll)
DECLARE_SETTINGSFACT(NextVisionSettings, localPositionPitch)
DECLARE_SETTINGSFACT(NextVisionSettings, globalPositionElevation)
DECLARE_SETTINGSFACT(NextVisionSettings, globalPositionAzimuth)
DECLARE_SETTINGSFACT(NextVisionSettings, camJoystickDZ)
DECLARE_SETTINGSFACT(NextVisionSettings, camJoystickGain)
DECLARE_SETTINGSFACT(NextVisionSettings, quickViewMode)
DECLARE_SETTINGSFACT(NextVisionSettings, camControlFontSize)
