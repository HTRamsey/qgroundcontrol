/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QGCMapPolygon.h"

/// The QGCFencePolygon class provides a polygon used by GeoFence support.
class QGCFencePolygon : public QGCMapPolygon
{
    Q_OBJECT
    Q_PROPERTY(bool inclusion READ inclusion WRITE setInclusion NOTIFY inclusionChanged)

public:
    explicit QGCFencePolygon(bool inclusion, QObject* parent = nullptr);
    explicit QGCFencePolygon(const QGCFencePolygon& other, QObject* parent = nullptr);

    const QGCFencePolygon &operator=(const QGCFencePolygon &other);

    /// Saves the QGCFencePolygon to the json object.
    ///     @param json Json object to save to
    void saveToJson(QJsonObject &json);

    /// Load a QGCFencePolygon from json
    ///     @param json Json object to load from
    ///     @param required true: no polygon in object will generate error
    ///     @param errorString Error string if return is false
    /// @return true: success, false: failure (errorString set)
    bool loadFromJson(const QJsonObject &json, bool required, QString &errorString);

    // Property methods
    bool inclusion() const { return _inclusion; }
    void setInclusion(bool inclusion);

signals:
    void inclusionChanged(bool inclusion);

private slots:
    void _setDirty();

private:
    void _init();

    bool _inclusion = false;

    static constexpr int _jsonCurrentVersion = 1;
    static constexpr const char *_jsonInclusionKey = "inclusion";
};
