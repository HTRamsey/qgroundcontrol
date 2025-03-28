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
#include <QtPositioning/QGeoCoordinate>

#include "Fact.h"

/// The QGCMapCircle represents a circular area which can be displayed on a Map control.
class QGCMapCircle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool             dirty               READ dirty              WRITE setDirty              NOTIFY dirtyChanged)
    Q_PROPERTY(QGeoCoordinate   center              READ center             WRITE setCenter             NOTIFY centerChanged)
    Q_PROPERTY(Fact             *radius             READ radius                                         CONSTANT)
    Q_PROPERTY(bool             interactive         READ interactive        WRITE setInteractive        NOTIFY interactiveChanged)
    Q_PROPERTY(bool             showRotation        READ showRotation       WRITE setShowRotation       NOTIFY showRotationChanged)
    Q_PROPERTY(bool             clockwiseRotation   READ clockwiseRotation  WRITE setClockwiseRotation  NOTIFY clockwiseRotationChanged)

public:
    explicit QGCMapCircle(QObject *parent = nullptr);
    explicit QGCMapCircle(const QGeoCoordinate &center, double radius, QObject *parent = nullptr);
    explicit QGCMapCircle(const QGeoCoordinate &center, double radius, bool showRotation, bool clockwiseRotation, QObject *parent = nullptr);
    explicit QGCMapCircle(const QGCMapCircle &other, QObject *parent = nullptr);

    const QGCMapCircle &operator=(const QGCMapCircle &other);

    /// Saves the polygon to the json object.
    ///     @param json Json object to save to
    void saveToJson(QJsonObject &json);

    /// Load a circle from json
    ///     @param json Json object to load from
    ///     @param errorString Error string if return is false
    /// @return true: success, false: failure (errorString set)
    bool loadFromJson(const QJsonObject &json, QString &errorString);

    // Property methods
    bool dirty() const { return _dirty; }
    QGeoCoordinate center() const { return _center; }
    Fact *radius() { return &_radius; }
    bool interactive() const { return _interactive; }
    bool showRotation() const { return _showRotation; }
    bool clockwiseRotation() const { return _clockwiseRotation; }

    void setDirty(bool dirty);
    void setCenter(const QGeoCoordinate &newCenter);
    void setInteractive(bool interactive);
    void setShowRotation(bool showRotation);
    void setClockwiseRotation(bool clockwiseRotation);

    static constexpr const char *jsonCircleKey = "circle";

signals:
    void dirtyChanged(bool dirty);
    void centerChanged(const QGeoCoordinate &center);
    void interactiveChanged(bool interactive);
    void showRotationChanged(bool showRotation);
    void clockwiseRotationChanged(bool clockwiseRotation);

private slots:
    void _setDirty();

private:
    void _init();

    QGeoCoordinate _center;
    Fact _radius;
    bool _showRotation = false;
    bool _clockwiseRotation = true;
    bool _dirty = false;
    bool _interactive = false;
    QMap<QString, FactMetaData*> _nameToMetaDataMap;

    static constexpr const char *_jsonCenterKey = "center";
    static constexpr const char *_jsonRadiusKey = "radius";
    static constexpr const char *_radiusFactName = "Radius";
};
