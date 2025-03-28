/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCMapCircle.h"
#include "JsonHelper.h"
#include "ParameterManager.h"

QGCMapCircle::QGCMapCircle(QObject *parent)
    : QObject(parent)
{
    _init();
}

QGCMapCircle::QGCMapCircle(const QGeoCoordinate &center, double radius, bool showRotation, bool clockwiseRotation, QObject *parent)
    : QObject(parent)
    , _center(center)
    , _radius(ParameterManager::defaultComponentId, _radiusFactName, FactMetaData::valueTypeDouble)
    , _showRotation(showRotation)
    , _clockwiseRotation(clockwiseRotation)
{
    _radius.setRawValue(radius);
    _init();
}

QGCMapCircle::QGCMapCircle(const QGCMapCircle &other, QObject *parent)
    : QObject(parent)
    , _center(other.center())
    , _radius(ParameterManager::defaultComponentId, _radiusFactName, FactMetaData::valueTypeDouble)
    , _showRotation(other.showRotation())
    , _clockwiseRotation(other.clockwiseRotation())
{
    _radius.setRawValue(other.radius().rawValue());
    _init();
}

const QGCMapCircle &QGCMapCircle::operator=(const QGCMapCircle &other)
{
    setCenter(other._center);
    _radius.setRawValue(other.radius().rawValue());
    setDirty(true);

    return *this;
}

void QGCMapCircle::_init()
{
    _nameToMetaDataMap = FactMetaData::createMapFromJsonFile(QStringLiteral(":/json/QGCMapCircle.Facts.json"), this);
    _radius.setMetaData(_nameToMetaDataMap[_radiusFactName]);

    (void) connect(this, &QGCMapCircle::centerChanged, this, &QGCMapCircle::_setDirty);
    (void) connect(&_radius, &Fact::rawValueChanged, this, &QGCMapCircle::_setDirty);
}

void QGCMapCircle::setDirty(bool dirty)
{
    if (_dirty != dirty) {
        _dirty = dirty;
        emit dirtyChanged(dirty);
    }
}

void QGCMapCircle::saveToJson(QJsonObject &json)
{
    QJsonValue jsonValue;
    JsonHelper::saveGeoCoordinate(_center, false /* writeAltitude*/, jsonValue);

    QJsonObject circleObject;
    circleObject.insert(_jsonCenterKey, jsonValue);
    circleObject.insert(_jsonRadiusKey, _radius.rawValue().toDouble());

    json.insert(jsonCircleKey, circleObject);
}

bool QGCMapCircle::loadFromJson(const QJsonObject &json, QString &errorString)
{
    errorString.clear();

    static const QList<JsonHelper::KeyValidateInfo> circleKeyInfo = {
        { jsonCircleKey, QJsonValue::Object, true },
    };
    if (!JsonHelper::validateKeys(json, circleKeyInfo, errorString)) {
        return false;
    }

    QJsonObject circleObject = json[jsonCircleKey].toObject();

    static const QList<JsonHelper::KeyValidateInfo> circleObjectKeyInfo = {
        { _jsonCenterKey, QJsonValue::Array, true },
        { _jsonRadiusKey, QJsonValue::Double, true },
    };
    if (!JsonHelper::validateKeys(circleObject, circleObjectKeyInfo, errorString)) {
        return false;
    }

    QGeoCoordinate center;
    if (!JsonHelper::loadGeoCoordinate(circleObject[_jsonCenterKey], false /* altitudeRequired */, center, errorString)) {
        return false;
    }
    setCenter(center);
    _radius.setRawValue(circleObject[_jsonRadiusKey].toDouble());

    _interactive = false;
    _showRotation = false;
    _clockwiseRotation = true;

    return true;
}

void QGCMapCircle::setCenter(const QGeoCoordinate &newCenter)
{
    if (newCenter != _center) {
        _center = newCenter;
        setDirty(true);
        emit centerChanged(newCenter);
    }
}

void QGCMapCircle::_setDirty()
{
    setDirty(true);
}

void QGCMapCircle::setInteractive(bool interactive)
{
    if (_interactive != interactive) {
        _interactive = interactive;
        emit interactiveChanged(interactive);
    }
}

void QGCMapCircle::setShowRotation(bool showRotation)
{
    if (showRotation != _showRotation) {
        _showRotation = showRotation;
        emit showRotationChanged(showRotation);
    }
}

void QGCMapCircle::setClockwiseRotation(bool clockwiseRotation)
{
    if (clockwiseRotation != _clockwiseRotation) {
        _clockwiseRotation = clockwiseRotation;
        emit clockwiseRotationChanged(clockwiseRotation);
    }
}
