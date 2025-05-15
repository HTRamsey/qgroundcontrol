/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCMapPolyline.h"
#include "JsonHelper.h"
#include "QGCApplication.h"
#include "QGCGeo.h"
#include "QGCLoggingCategory.h"
#include "QGCQGeoCoordinate.h"
#include "ShapeFileHelper.h"

#include <QtCore/QLineF>

QGC_LOGGING_CATEGORY(QGCMapPolylineLog, "qgc.qmlcontrols.qgcmappolyline")

QGCMapPolyline::QGCMapPolyline(QObject *parent)
    : QmlObjectListModel(parent)
{
    _init();
}

QGCMapPolyline::QGCMapPolyline(const QGCMapPolyline &other, QObject *parent)
    : QmlObjectListModel(parent)
{
    *this = other;

    _init();
}

QGCMapPolyline::~QGCMapPolyline()
{
    qCDebug(QGCMapPolylineLog) << this;
}

const QGCMapPolyline &QGCMapPolyline::operator=(const QGCMapPolyline &other)
{
    setPath(other.path());

    return *this;
}

void QGCMapPolyline::_init()
{
    (void) connect(this, &QGCMapPolyline::countChanged, this, &QGCMapPolyline::isValidChanged);
    (void) connect(this, &QGCMapPolyline::cleared, this, [this]() {
        _polylinePath.clearPath();
        emit pathChanged();
        setDirty(true);
    });

    qCDebug(QGCMapPolylineLog) << this;
}

void QGCMapPolyline::setTraceMode(bool traceMode)
{
    if (traceMode != _traceMode) {
        _traceMode = traceMode;
        emit traceModeChanged(traceMode);
    }
}

void QGCMapPolyline::setInteractive(bool interactive)
{
    if (_interactive != interactive) {
        _interactive = interactive;
        emit interactiveChanged(interactive);
    }
}

void QGCMapPolyline::setPath(const QVariantList &path)
{
    _beginResetIfNotActive();

    clear();
    appendVertices(path);

    setDirty(true);

    _endResetIfActive();
}

void QGCMapPolyline::setPath(const QList<QGeoCoordinate> &path)
{
    _beginResetIfNotActive();

    clear();
    appendVertices(path);

    setDirty(true);

    _endResetIfActive();
}

void QGCMapPolyline::saveToJson(QJsonObject &json)
{
    QJsonValue jsonValue;

    JsonHelper::saveGeoCoordinateArray(_polylinePath.path(), false /* writeAltitude*/, jsonValue);
    (void) json.insert(jsonPolylineKey, jsonValue);

    setDirty(false);
}

bool QGCMapPolyline::loadFromJson(const QJsonObject &json, bool required, QString &errorString)
{
    errorString.clear();
    clear();

    if (required) {
        if (!JsonHelper::validateRequiredKeys(json, QStringList(jsonPolylineKey), errorString)) {
            return false;
        }
    } else if (!json.contains(jsonPolylineKey)) {
        return true;
    }

    QList<QGeoCoordinate> vertices;
    if (!JsonHelper::loadGeoCoordinateArray(json[jsonPolylineKey], false /* altitudeRequired */, vertices, errorString)) {
        return false;
    }

    appendVertices(vertices);

    setDirty(false);

    return true;
}

void QGCMapPolyline::splitSegment(int vertexIndex)
{
    const int nextIndex = vertexIndex + 1;
    if (nextIndex > (_polylinePath.length() - 1)) {
        return;
    }

    const QGeoCoordinate firstVertex = vertexCoordinate(vertexIndex);
    const QGeoCoordinate nextVertex = vertexCoordinate(nextIndex);

    const double distance = firstVertex.distanceTo(nextVertex);
    const double azimuth = firstVertex.azimuthTo(nextVertex);
    const QGeoCoordinate newVertex = firstVertex.atDistanceAndAzimuth(distance / 2, azimuth);

    if (nextIndex == 0) {
        appendVertex(newVertex);
    } else {
        insert(nextIndex, new QGCQGeoCoordinate(newVertex, this));
        (void) _polylinePath.insertCoordinate(nextIndex, newVertex);
        emit pathChanged();
    }
}

void QGCMapPolyline::adjustVertex(int vertexIndex, const QGeoCoordinate &coordinate)
{
    _polylinePath.replaceCoordinate(vertexIndex, coordinate);
    emit pathChanged();

    value<QGCQGeoCoordinate*>(vertexIndex)->setCoordinate(coordinate);

    setDirty(true);
}

void QGCMapPolyline::appendVertex(const QGeoCoordinate &coordinate)
{
    _polylinePath.addCoordinate(coordinate);
    append(new QGCQGeoCoordinate(coordinate, this));
    emit pathChanged();
}

void QGCMapPolyline::appendVertices(const QList<QGeoCoordinate> &coordinates)
{
    _beginResetIfNotActive();

    for (const QGeoCoordinate &coordinate : coordinates) {
        appendVertex(coordinate);
    }

    _endResetIfActive();
}

void QGCMapPolyline::appendVertices(const QVariantList &varCoords)
{
    _beginResetIfNotActive();

    for (const QVariant &vertex : varCoords) {
        appendVertex(vertex.value<QGeoCoordinate>());
    }

    _endResetIfActive();
}

void QGCMapPolyline::selectVertex(int index)
{
    if (index == _selectedVertexIndex) {
        return;
    }

    if ((index >= -1) && (index < count())) {
        _selectedVertexIndex = index;
    } else {
        if (!qgcApp()->runningUnitTests()) {
            qCWarning(QGCMapPolylineLog) << QStringLiteral("Selected vertex index (%1) is out of bounds! "
                                            "Polyline vertices indexes range is [%2..%3].").arg(index).arg(0).arg(count() - 1);
        }
        _selectedVertexIndex = -1;
    }

    emit selectedVertexChanged(_selectedVertexIndex);
}

void QGCMapPolyline::removeVertex(int vertexIndex)
{
    if ((vertexIndex < 0) || (vertexIndex > (_polylinePath.length() - 1))) {
        qCWarning(QGCMapPolylineLog) << "bad vertexIndex:count" << vertexIndex << _polylinePath.length();
        return;
    }

    if (_polylinePath.length() <= 2) {
        // Don't allow the user to trash the polyline
        return;
    }

    QObject *coordObj = removeAt(vertexIndex);
    coordObj->deleteLater();

    if (vertexIndex == _selectedVertexIndex) {
        selectVertex(-1);
    } else if (vertexIndex < _selectedVertexIndex) {
        selectVertex(_selectedVertexIndex - 1);
    } // else do nothing - keep current selected vertex

    _polylinePath.removeCoordinate(vertexIndex);
    emit pathChanged();
}

QGeoCoordinate QGCMapPolyline::vertexCoordinate(int vertex) const
{
    if (vertex < 0) {
        return QGeoCoordinate();
    }

    if (vertex >= _polylinePath.size()) {
        qCWarning(QGCMapPolylineLog) << "bad vertex requested:count" << vertex << _polylinePath.size();
        return QGeoCoordinate();
    }

    return _polylinePath.coordinateAt(vertex);
}

QGeoCoordinate QGCMapPolyline::_coordFromPointF(const QPointF &point) const
{
    if (_polylinePath.isEmpty()) {
        return QGeoCoordinate();
    }

    QGeoCoordinate coord;
    const QGeoCoordinate tangentOrigin = vertexCoordinate(0);
    QGCGeo::convertNedToGeo(-point.y(), point.x(), 0, tangentOrigin, coord);
    return coord;
}

QPointF QGCMapPolyline::_pointFFromCoord(const QGeoCoordinate &coordinate) const
{
    if (_polylinePath.isEmpty()) {
        return QPointF();
    }

    const QGeoCoordinate tangentOrigin = vertexCoordinate(0);
    double y = 0, x = 0, down = 0;
    QGCGeo::convertGeoToNed(coordinate, tangentOrigin, y, x, down);
    return QPointF(x, -y);
}

QList<QPointF> QGCMapPolyline::nedPolyline() const
{
    if (count() <= 0) {
        return QList<QPointF>();
    }

    QList<QPointF> nedPolyline;
    const QGeoCoordinate tangentOrigin = vertexCoordinate(0);
    for (int i = 0; i < _polylinePath.size(); i++) {
        const QGeoCoordinate vertex = vertexCoordinate(i);
        double y = 0, x = 0, down = 0;
        QGCGeo::convertGeoToNed(vertex, tangentOrigin, y, x, down);

        nedPolyline += QPointF(x, y);
    }

    return nedPolyline;
}

QList<QGeoCoordinate> QGCMapPolyline::offsetPolyline(double distance)
{
    if (count() <= 0) {
        return QList<QGeoCoordinate>();
    }

    // Convert the polygon to NED
    const QList<QPointF> rgNedVertices = nedPolyline();

    // Walk the edges, offsetting by the specified distance
    QList<QLineF> rgOffsetEdges;
    for (int i = 0; i < rgNedVertices.count() - 1; i++) {
        QLineF offsetEdge;
        const QLineF originalEdge(rgNedVertices[i], rgNedVertices[i + 1]);

        QLineF workerLine = originalEdge;
        workerLine.setLength(distance);
        workerLine.setAngle(workerLine.angle() - 90.0);
        offsetEdge.setP1(workerLine.p2());

        workerLine.setPoints(originalEdge.p2(), originalEdge.p1());
        workerLine.setLength(distance);
        workerLine.setAngle(workerLine.angle() + 90.0);
        offsetEdge.setP2(workerLine.p2());

        rgOffsetEdges.append(offsetEdge);
    }

    QList<QGeoCoordinate> rgNewPolyline;

    // Intersect the offset edges to generate new central vertices
    for (int i = 0; i < rgOffsetEdges.count(); i++) {
        QGeoCoordinate coord;
        const QLineF offsetEdge = rgOffsetEdges[i];
        const QGeoCoordinate tangentOrigin = vertexCoordinate(0);

        // Add first vertex
        if (i == 0) {
            QGCGeo::convertNedToGeo(offsetEdge.p1().y(), offsetEdge.p1().x(), 0, tangentOrigin, coord);
            rgNewPolyline.append(coord);
            continue;
        }

        const QLineF prevOffsetEdge = rgOffsetEdges[i - 1];
        QPointF newVertex;
        const QLineF::IntersectionType intersect = prevOffsetEdge.intersects(offsetEdge, &newVertex);
        if (intersect == QLineF::NoIntersection) {
            // Two lines are colinear
            newVertex = offsetEdge.p2();
        }
        QGCGeo::convertNedToGeo(newVertex.y(), newVertex.x(), 0, tangentOrigin, coord);
        rgNewPolyline.append(coord);

        // Add last vertex
        if (i == (rgOffsetEdges.count() - 1)) {
            QGCGeo::convertNedToGeo(offsetEdge.p2().y(), offsetEdge.p2().x(), 0, tangentOrigin, coord);
            rgNewPolyline.append(coord);
        }
    }

    return rgNewPolyline;
}

bool QGCMapPolyline::loadKMLOrSHPFile(const QString &file)
{
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    if (!ShapeFileHelper::loadPolylineFromFile(file, rgCoords, errorString)) {
        qgcApp()->showAppMessage(errorString);
        return false;
    }

    setPath(rgCoords);

    return true;
}

double QGCMapPolyline::length() const
{
    double length = 0;

    for (int i = 0; i < (_polylinePath.size() - 1); i++) {
        const QGeoCoordinate from = vertexCoordinate(i);
        const QGeoCoordinate to = vertexCoordinate(i + 1);
        length += from.distanceTo(to);
    }

    return length;
}

void QGCMapPolyline::_beginResetIfNotActive()
{
    if (!_resetActive) {
        beginReset();
    }
}

void QGCMapPolyline::_endResetIfActive()
{
    if (_resetActive) {
        endReset();
    }
}
