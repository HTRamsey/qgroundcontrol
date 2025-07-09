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
#include <QtCore/QMetaMethod>

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
    qgcApp()->removeCompressedSignal(QMetaMethod::fromSignal(&QGCMapPolyline::pathChanged));
}

const QGCMapPolyline& QGCMapPolyline::operator=(const QGCMapPolyline &other)
{
    clear();

    QVariantList vertices = other.path();
    for (int i = 0; i < vertices.count(); i++) {
        appendVertex(vertices[i].value<QGeoCoordinate>());
    }

    setDirty(true);

    return *this;
}

void QGCMapPolyline::_init()
{
    (void) connect(this, &QmlObjectListModel::modelReset, this, &QGCMapPolyline::pathChanged);

    (void) connect(this, &QGCMapPolyline::countChanged, this, [this](int count) {
        emit isValidChanged();
    });

    qgcApp()->addCompressedSignal(QMetaMethod::fromSignal(&QGCMapPolyline::pathChanged));
}

QGeoCoordinate QGCMapPolyline::_coordFromPointF(const QPointF &point) const
{
    if (_polylinePath.isEmpty()) {
        return QGeoCoordinate();
    }

    QGeoCoordinate coord;
    const QGeoCoordinate tangentOrigin = _polylinePath[0].value<QGeoCoordinate>();
    QGCGeo::convertNedToGeo(-point.y(), point.x(), 0, tangentOrigin, coord);

    return coord;
}

QPointF QGCMapPolyline::_pointFFromCoord(const QGeoCoordinate &coordinate) const
{
    if (_polylinePath.isEmpty()) {
        return QPointF();
    }

    double y, x, down;
    const QGeoCoordinate tangentOrigin = _polylinePath[0].value<QGeoCoordinate>();
    QGCGeo::convertGeoToNed(coordinate, tangentOrigin, y, x, down);

    const QPointF point(x, -y);
    return point;
}

void QGCMapPolyline::clear()
{
    _polylinePath.clear();
    emit pathChanged();

    clearAndDeleteContents();

    emit cleared();

    setDirty(true);
}

void QGCMapPolyline::setPath(const QList<QGeoCoordinate> &path)
{
    beginResetModel();

    _polylinePath.clear();
    clearAndDeleteContents();
    for (const QGeoCoordinate& coord: path) {
        _polylinePath.append(QVariant::fromValue(coord));
        append(new QGCQGeoCoordinate(coord, this));
    }

    setDirty(true);

    endResetModel();
}

void QGCMapPolyline::setPath(const QVariantList &path)
{
    beginResetModel();

    _polylinePath = path;
    clearAndDeleteContents();
    for (int i = 0; i < _polylinePath.count(); i++) {
        append(new QGCQGeoCoordinate(_polylinePath[i].value<QGeoCoordinate>(), this));
    }
    setDirty(true);

    endResetModel();
}

void QGCMapPolyline::saveToJson(QJsonObject &json)
{
    QJsonValue jsonValue;

    JsonHelper::saveGeoCoordinateArray(_polylinePath, false /* writeAltitude*/, jsonValue);
    json.insert(jsonPolylineKey, jsonValue);

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

    if (!JsonHelper::loadGeoCoordinateArray(json[jsonPolylineKey], false /* altitudeRequired */, _polylinePath, errorString)) {
        return false;
    }

    for (int i = 0; i < _polylinePath.count(); i++) {
        append(new QGCQGeoCoordinate(_polylinePath[i].value<QGeoCoordinate>(), this));
    }

    setDirty(false);
    emit pathChanged();

    return true;
}

bool QGCMapPolyline::loadKMLOrSHPFile(const QString &file)
{
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    if (!ShapeFileHelper::loadPolylineFromFile(file, rgCoords, errorString)) {
        qgcApp()->showAppMessage(errorString);
        return false;
    }

    beginResetModel();
    clear();
    appendVertices(rgCoords);
    endResetModel();

    return true;
}

void QGCMapPolyline::splitSegment(int vertexIndex)
{
    const int nextIndex = vertexIndex + 1;
    if (nextIndex > (_polylinePath.length() - 1)) {
        return;
    }

    const QGeoCoordinate firstVertex = _polylinePath[vertexIndex].value<QGeoCoordinate>();
    const QGeoCoordinate nextVertex = _polylinePath[nextIndex].value<QGeoCoordinate>();

    const double distance = firstVertex.distanceTo(nextVertex);
    const double azimuth = firstVertex.azimuthTo(nextVertex);
    const QGeoCoordinate newVertex = firstVertex.atDistanceAndAzimuth(distance / 2, azimuth);

    if (nextIndex == 0) {
        appendVertex(newVertex);
    } else {
        insert(nextIndex, new QGCQGeoCoordinate(newVertex, this));
        (void) _polylinePath.insert(nextIndex, QVariant::fromValue(newVertex));
        emit pathChanged();
    }
}

QList<QPointF> QGCMapPolyline::nedPolyline()
{
    QList<QPointF> nedPolyline;

    if (!isEmpty()) {
        const QGeoCoordinate tangentOrigin = vertexCoordinate(0);

        for (int i = 0; i < _polylinePath.count(); i++) {
            QPointF point(0, 0);
            if (i > 0) {
                double down;
                const QGeoCoordinate vertex = vertexCoordinate(i);
                QGCGeo::convertGeoToNed(vertex, tangentOrigin, point.ry(), point.rx(), down);
            }
            nedPolyline += point;
        }
    }

    return nedPolyline;
}

QList<QGeoCoordinate> QGCMapPolyline::offsetPolyline(double distance)
{
    QList<QGeoCoordinate> rgNewPolyline;

    // I'm sure there is some beautiful famous algorithm to do this, but here is a brute force method

    if (count() <= 1) {
        return rgNewPolyline;
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

    const QGeoCoordinate tangentOrigin = vertexCoordinate(0);

    // Add first vertex
    QGeoCoordinate coord;
    QGCGeo::convertNedToGeo(rgOffsetEdges[0].p1().y(), rgOffsetEdges[0].p1().x(), 0, tangentOrigin, coord);
    rgNewPolyline.append(coord);

    // Intersect the offset edges to generate new central vertices
    QPointF newVertex;
    for (int i = 1; i < rgOffsetEdges.count(); i++) {
        const auto intersect = rgOffsetEdges[i - 1].intersects(rgOffsetEdges[i], &newVertex);
        if (intersect == QLineF::NoIntersection) {
            // Two lines are colinear
            newVertex = rgOffsetEdges[i].p2();
        }
        QGCGeo::convertNedToGeo(newVertex.y(), newVertex.x(), 0, tangentOrigin, coord);
        rgNewPolyline.append(coord);
    }

    // Add last vertex
    const int lastIndex = rgOffsetEdges.count() - 1;
    QGCGeo::convertNedToGeo(rgOffsetEdges[lastIndex].p2().y(), rgOffsetEdges[lastIndex].p2().x(), 0, tangentOrigin, coord);
    rgNewPolyline.append(coord);

    return rgNewPolyline;
}

void QGCMapPolyline::adjustVertex(int vertexIndex, const QGeoCoordinate &coordinate)
{
    _polylinePath[vertexIndex] = QVariant::fromValue(coordinate);
    value<QGCQGeoCoordinate*>(vertexIndex)->setCoordinate(coordinate);
    if (!_deferredPathChanged) {
        _deferredPathChanged = true;
        QTimer::singleShot(0, this, [this]() {
            emit pathChanged();
            _deferredPathChanged = false;
        });
    }
    setDirty(true);
}

void QGCMapPolyline::appendVertex(const QGeoCoordinate &coordinate)
{
    _polylinePath.append(QVariant::fromValue(coordinate));
    append(new QGCQGeoCoordinate(coordinate, this));
    emit pathChanged();
}

void QGCMapPolyline::removeVertex(int vertexIndex)
{
    if ((vertexIndex < 0) || (vertexIndex > (_polylinePath.length() - 1))) {
        qCWarning(QGCMapPolylineLog) << "Call to removeVertex with bad vertexIndex:count" << vertexIndex << _polylinePath.length();
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

    _polylinePath.removeAt(vertexIndex);
    emit pathChanged();
}

QGeoCoordinate QGCMapPolyline::vertexCoordinate(int vertex) const
{
    if ((vertex < 0) || (vertex >= _polylinePath.count())) {
        qCWarning(QGCMapPolylineLog) << "bad vertex requested";
        return QGeoCoordinate();
    }

    return _polylinePath[vertex].value<QGeoCoordinate>();
}

void QGCMapPolyline::appendVertices(const QList<QGeoCoordinate> &coordinates)
{
    beginResetModel();

    for (const QGeoCoordinate &coordinate: coordinates) {
        appendVertex(coordinate);
    }

    endResetModel();

    emit pathChanged();
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
                                                           "Polyline vertices indexes range is [%2..%3].").arg(index).arg(0).arg(count()-1);
        }
        _selectedVertexIndex = -1; // deselect vertex
    }

    emit selectedVertexChanged(_selectedVertexIndex);
}

QList<QGeoCoordinate> QGCMapPolyline::coordinateList() const
{
    QList<QGeoCoordinate> coords;

    for (int i = 0; i < _polylinePath.count(); i++) {
        coords.append(_polylinePath[i].value<QGeoCoordinate>());
    }

    return coords;
}

double QGCMapPolyline::length() const
{
    double length = 0;

    for (int i = 0; i < _polylinePath.count() - 1; i++) {
        const QGeoCoordinate from = _polylinePath[i].value<QGeoCoordinate>();
        const QGeoCoordinate to = _polylinePath[i+1].value<QGeoCoordinate>();
        length += from.distanceTo(to);
    }

    return length;
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
