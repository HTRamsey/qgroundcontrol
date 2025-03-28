/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QtMath>

#include "FactMetaData.h"

class QmlUnitsConversion : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appSettingsHorizontalDistanceUnitsString READ appSettingsHorizontalDistanceUnitsString CONSTANT)
    Q_PROPERTY(QString appSettingsVerticalDistanceUnitsString   READ appSettingsVerticalDistanceUnitsString   CONSTANT)
    Q_PROPERTY(QString appSettingsAreaUnitsString               READ appSettingsAreaUnitsString               CONSTANT)
    Q_PROPERTY(QString appSettingsWeightUnitsString             READ appSettingsWeightUnitsString             CONSTANT)
    Q_PROPERTY(QString appSettingsSpeedUnitsString              READ appSettingsSpeedUnitsString              CONSTANT)

public:
    explicit QmlUnitsConversion(QObject *parent = nullptr)
        : QObject(parent) {}

    /// Converts from meters to the user specified distance unit
    Q_INVOKABLE static QVariant metersToAppSettingsHorizontalDistanceUnits(const QVariant &meters) { return FactMetaData::metersToAppSettingsHorizontalDistanceUnits(meters); }

    /// Converts from user specified distance unit to meters
    Q_INVOKABLE static QVariant appSettingsHorizontalDistanceUnitsToMeters(const QVariant &distance) { return FactMetaData::appSettingsHorizontalDistanceUnitsToMeters(distance); }

    static QString appSettingsHorizontalDistanceUnitsString() { return FactMetaData::appSettingsHorizontalDistanceUnitsString(); }

    /// Converts from meters to the user specified distance unit
    Q_INVOKABLE static QVariant metersToAppSettingsVerticalDistanceUnits(const QVariant &meters) { return FactMetaData::metersToAppSettingsVerticalDistanceUnits(meters); }

    /// Converts from user specified distance unit to meters
    Q_INVOKABLE static QVariant appSettingsVerticalDistanceUnitsToMeters(const QVariant &distance) { return FactMetaData::appSettingsVerticalDistanceUnitsToMeters(distance); }

    static QString appSettingsVerticalDistanceUnitsString() { return FactMetaData::appSettingsVerticalDistanceUnitsString(); }

    /// Converts from grams to the user specified weight unit
    Q_INVOKABLE static QVariant gramsToAppSettingsWeightUnits(const QVariant &meters) { return FactMetaData::gramsToAppSettingsWeightUnits(meters); }

    /// Converts from user specified weight unit to grams
    Q_INVOKABLE static QVariant appSettingsWeightUnitsToGrams(const QVariant &distance) { return FactMetaData::appSettingsWeightUnitsToGrams(distance); }

    static QString appSettingsWeightUnitsString() { return FactMetaData::appSettingsWeightUnitsString(); }

    /// Converts from square meters to the user specified area unit
    Q_INVOKABLE static QVariant squareMetersToAppSettingsAreaUnits(const QVariant &meters) { return FactMetaData::squareMetersToAppSettingsAreaUnits(meters); }

    /// Converts from user specified area unit to square meters
    Q_INVOKABLE static QVariant appSettingsAreaUnitsToSquareMeters(const QVariant &area) { return FactMetaData::appSettingsAreaUnitsToSquareMeters(area); }

    static QString appSettingsAreaUnitsString() { return FactMetaData::appSettingsAreaUnitsString(); }

    /// Converts from meters/second to the user specified speed unit
    Q_INVOKABLE static QVariant metersSecondToAppSettingsSpeedUnits(const QVariant &metersSecond) { return FactMetaData::metersSecondToAppSettingsSpeedUnits(metersSecond); }

    /// Converts from user specified speed unit to meters/second
    Q_INVOKABLE static QVariant appSettingsSpeedUnitsToMetersSecond(const QVariant &speed) { return FactMetaData::appSettingsSpeedUnitsToMetersSecond(speed); }

    /// Returns the string for speed units which has configued by user
    static QString appSettingsSpeedUnitsString() { return FactMetaData::appSettingsSpeedUnitsString(); }

    Q_INVOKABLE static double degreesToRadians(double degrees) { return qDegreesToRadians(degrees); }
    Q_INVOKABLE static double radiansToDegrees(double radians) { return qRadiansToDegrees(radians); }
};
