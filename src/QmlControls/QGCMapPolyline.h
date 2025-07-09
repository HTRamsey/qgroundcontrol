/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>

#include "QmlObjectListModel.h"

Q_DECLARE_LOGGING_CATEGORY(QGCMapPolylineLog)

class QGCMapPolyline : public QmlObjectListModel
{
    Q_OBJECT
    Q_PROPERTY(QVariantList path            READ path                                   NOTIFY pathChanged)
    Q_PROPERTY(bool         interactive     READ interactive    WRITE setInteractive    NOTIFY interactiveChanged)
    Q_PROPERTY(bool         isValid         READ isValid                                NOTIFY isValidChanged)
    Q_PROPERTY(bool         traceMode       READ traceMode      WRITE setTraceMode      NOTIFY traceModeChanged)
    Q_PROPERTY(int          selectedVertex  READ selectedVertex WRITE selectVertex      NOTIFY selectedVertexChanged)
public:
    explicit QGCMapPolyline(QObject *parent = nullptr);
    explicit QGCMapPolyline(const QGCMapPolyline &other, QObject *parent = nullptr);

    virtual ~QGCMapPolyline();

    const QGCMapPolyline &operator=(const QGCMapPolyline &other);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void appendVertex(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeVertex(int vertexIndex);
    Q_INVOKABLE void appendVertices(const QList<QGeoCoordinate> &coordinates);

    /// Adjust the value for the specified coordinate
    ///     @param vertexIndex Polygon point index to modify (0-based)
    ///     @param coordinate New coordinate for point
    Q_INVOKABLE void adjustVertex(int vertexIndex, const QGeoCoordinate &coordinate);

    /// Splits the line segment comprised of vertexIndex -> vertexIndex + 1
    Q_INVOKABLE void splitSegment(int vertexIndex);

    /// Offsets the current polyline edges by the specified distance in meters
    /// @return Offset set of vertices
    QList<QGeoCoordinate> offsetPolyline(double distance);

    /// Loads a polyline from a KML/SHP file
    /// @return true: success
    Q_INVOKABLE bool loadKMLOrSHPFile(const QString &file);

    /// Returns the path in a list of QGeoCoordinate's format
    QList<QGeoCoordinate> coordinateList() const;

    /// Returns the QGeoCoordinate for the vertex specified
    Q_INVOKABLE QGeoCoordinate vertexCoordinate(int vertex) const;

    /// Saves the polyline to the json object.
    ///     @param json Json object to save to
    void saveToJson(QJsonObject &json);

    /// Load a polyline from json
    ///     @param json Json object to load from
    ///     @param required true: no polygon in object will generate error
    ///     @param errorString Error string if return is false
    /// @return true: success, false: failure (errorString set)
    bool loadFromJson(const QJsonObject &json, bool required, QString &errorString);

    /// Convert polyline to NED and return (D is ignored)
    QList<QPointF> nedPolyline();

    /// Returns the length of the polyline in meters
    double length() const;

    // Property methods
    bool isValid() const { return (count() >= 2); }
    bool interactive() const { return _interactive; }
    QVariantList path() const { return _polylinePath; }
    bool traceMode() const { return _traceMode; }
    int selectedVertex() const { return _selectedVertexIndex; }

    void setInteractive(bool interactive);
    void setPath(const QList<QGeoCoordinate>& path);
    void setPath(const QVariantList& path);
    void setTraceMode(bool traceMode);
    void selectVertex(int index);

    static constexpr const char *jsonPolylineKey = "polyline";

signals:
    void pathChanged();
    void interactiveChanged(bool interactive);
    void isValidChanged();
    void traceModeChanged(bool traceMode);
    void selectedVertexChanged(int index);

private:
    void _init();
    QGeoCoordinate _coordFromPointF(const QPointF &point) const;
    QPointF _pointFFromCoord(const QGeoCoordinate &coordinate) const;

    // QGeoPath _polylinePath;
    QVariantList _polylinePath;
    bool _deferredPathChanged = false;
    bool _interactive = false;
    bool _traceMode = false;
    int _selectedVertexIndex = -1;
};
