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

/// Most individual settings related to PlanView are still in AppSettings due to historical reasons.
class PlanViewSettings : public SettingsGroup
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit PlanViewSettings(QObject *parent = nullptr);

    DEFINE_SETTING_NAME_GROUP()

    DEFINE_SETTINGFACT(displayPresetsTabFirst)
    DEFINE_SETTINGFACT(showMissionItemStatus)
    DEFINE_SETTINGFACT(useConditionGate)
    DEFINE_SETTINGFACT(takeoffItemNotRequired)
    DEFINE_SETTINGFACT(allowMultipleLandingPatterns)
    DEFINE_SETTINGFACT(showGimbalOnlyWhenSet)
    DEFINE_SETTINGFACT(vtolTransitionDistance)
};
