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

class UnitsSettings : public SettingsGroup
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit UnitsSettings(QObject* parent = nullptr);

    DEFINE_SETTING_NAME_GROUP()

    enum HorizontalDistanceUnits {
        HorizontalDistanceUnitsFeet = 0,
        HorizontalDistanceUnitsMeters
    };
    Q_ENUM(HorizontalDistanceUnits)

    enum VerticalDistanceUnits {
        VerticalDistanceUnitsFeet = 0,
        VerticalDistanceUnitsMeters
    };
    Q_ENUM(VerticalDistanceUnits)

    enum AreaUnits {
        AreaUnitsSquareFeet = 0,
        AreaUnitsSquareMeters,
        AreaUnitsSquareKilometers,
        AreaUnitsHectares,
        AreaUnitsAcres,
        AreaUnitsSquareMiles,
    };
    Q_ENUM(AreaUnits)

    enum SpeedUnits {
        SpeedUnitsFeetPerSecond = 0,
        SpeedUnitsMetersPerSecond,
        SpeedUnitsMilesPerHour,
        SpeedUnitsKilometersPerHour,
        SpeedUnitsKnots,
    };
    Q_ENUM(SpeedUnits)

    enum TemperatureUnits {
        TemperatureUnitsCelsius = 0,
        TemperatureUnitsFarenheit,
    };
    Q_ENUM(TemperatureUnits)

    enum WeightUnits {
        WeightUnitsGrams = 0,
        WeightUnitsKg,
        WeightUnitsOz,
        WeightUnitsLbs
    };
    Q_ENUM(WeightUnits)


    DEFINE_SETTINGFACT(horizontalDistanceUnits)
    DEFINE_SETTINGFACT(verticalDistanceUnits)
    DEFINE_SETTINGFACT(areaUnits)
    DEFINE_SETTINGFACT(speedUnits)
    DEFINE_SETTINGFACT(temperatureUnits)
    DEFINE_SETTINGFACT(weightUnits)
};
