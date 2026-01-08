#include "QGCGeofence.h"

#include <QtCore/QUuid>
#include <QtCore/QtMath>

Q_LOGGING_CATEGORY(QGCGeofenceLog, "Utilities.QGCGeofence")

QGCGeofence::QGCGeofence(QObject *parent)
    : QObject(parent)
{
}

QString QGCGeofence::addCircularFence(const QString &name, const QGeoCoordinate &center,
                                       double radiusMeters, FenceType type)
{
    Fence fence;
    fence.id = QStringLiteral("fence_%1").arg(_nextId++);
    fence.name = name;
    fence.type = type;
    fence.shape = FenceShape::Circle;
    fence.circle = QGeoCircle(center, radiusMeters);

    _fences.append(fence);
    emit fenceAdded(fence.id);
    emit fenceCountChanged(fenceCount());

    qCDebug(QGCGeofenceLog) << "Added circular fence:" << fence.id << name << "radius:" << radiusMeters << "m";
    _checkBreaches();

    return fence.id;
}

QString QGCGeofence::addRectangularFence(const QString &name, const QGeoRectangle &rect, FenceType type)
{
    Fence fence;
    fence.id = QStringLiteral("fence_%1").arg(_nextId++);
    fence.name = name;
    fence.type = type;
    fence.shape = FenceShape::Rectangle;
    fence.rectangle = rect;

    _fences.append(fence);
    emit fenceAdded(fence.id);
    emit fenceCountChanged(fenceCount());

    qCDebug(QGCGeofenceLog) << "Added rectangular fence:" << fence.id << name;
    _checkBreaches();

    return fence.id;
}

QString QGCGeofence::addPolygonFence(const QString &name, const QList<QGeoCoordinate> &vertices, FenceType type)
{
    if (vertices.size() < 3) {
        qCWarning(QGCGeofenceLog) << "Polygon fence requires at least 3 vertices";
        return {};
    }

    Fence fence;
    fence.id = QStringLiteral("fence_%1").arg(_nextId++);
    fence.name = name;
    fence.type = type;
    fence.shape = FenceShape::Polygon;
    fence.polygon = QGeoPolygon(vertices);

    _fences.append(fence);
    emit fenceAdded(fence.id);
    emit fenceCountChanged(fenceCount());

    qCDebug(QGCGeofenceLog) << "Added polygon fence:" << fence.id << name << "vertices:" << vertices.size();
    _checkBreaches();

    return fence.id;
}

bool QGCGeofence::removeFence(const QString &id)
{
    for (int i = 0; i < _fences.size(); ++i) {
        if (_fences[i].id == id) {
            _fences.removeAt(i);
            emit fenceRemoved(id);
            emit fenceCountChanged(fenceCount());
            _checkBreaches();
            return true;
        }
    }
    return false;
}

void QGCGeofence::clearFences()
{
    _fences.clear();
    _currentBreaches.clear();
    _previousBreachedIds.clear();
    emit fenceCountChanged(0);
    emit breachChanged(false);
}

bool QGCGeofence::setFenceEnabled(const QString &id, bool enabled)
{
    for (Fence &fence : _fences) {
        if (fence.id == id) {
            fence.enabled = enabled;
            _checkBreaches();
            return true;
        }
    }
    return false;
}

bool QGCGeofence::setFenceAltitudeLimits(const QString &id, double minAlt, double maxAlt)
{
    for (Fence &fence : _fences) {
        if (fence.id == id) {
            fence.minAltitude = minAlt;
            fence.maxAltitude = maxAlt;
            _checkBreaches();
            return true;
        }
    }
    return false;
}

QGCGeofence::Fence QGCGeofence::fence(const QString &id) const
{
    for (const Fence &fence : _fences) {
        if (fence.id == id) {
            return fence;
        }
    }
    return {};
}

bool QGCGeofence::hasFence(const QString &id) const
{
    for (const Fence &fence : _fences) {
        if (fence.id == id) {
            return true;
        }
    }
    return false;
}

QStringList QGCGeofence::fenceIds() const
{
    QStringList ids;
    for (const Fence &fence : _fences) {
        ids.append(fence.id);
    }
    return ids;
}

void QGCGeofence::setPosition(const QGeoCoordinate &position)
{
    if (_position == position) {
        return;
    }

    _position = position;
    emit positionChanged(position);
    _checkBreaches();
}

void QGCGeofence::setAltitude(double altitude)
{
    if (qFuzzyCompare(_altitude, altitude)) {
        return;
    }

    _altitude = altitude;
    emit altitudeChanged(altitude);
    _checkBreaches();
}

QStringList QGCGeofence::breachedFenceIds() const
{
    QStringList ids;
    for (const BreachInfo &breach : _currentBreaches) {
        ids.append(breach.fenceId);
    }
    return ids;
}

QStringList QGCGeofence::breachedFenceNames() const
{
    QStringList names;
    for (const BreachInfo &breach : _currentBreaches) {
        names.append(breach.fenceName);
    }
    return names;
}

bool QGCGeofence::isPointInFence(const QGeoCoordinate &point, const QString &fenceId) const
{
    for (const Fence &fence : _fences) {
        if (fence.id == fenceId) {
            return _isInsideFence(point, fence);
        }
    }
    return false;
}

bool QGCGeofence::isPointInAnyFence(const QGeoCoordinate &point) const
{
    for (const Fence &fence : _fences) {
        if (fence.enabled && fence.type == FenceType::Inclusion) {
            if (_isInsideFence(point, fence)) {
                return true;
            }
        }
    }
    return false;
}

double QGCGeofence::distanceToFence(const QGeoCoordinate &point, const QString &fenceId) const
{
    for (const Fence &fence : _fences) {
        if (fence.id == fenceId) {
            return _distanceToFence(point, fence);
        }
    }
    return -1;
}

double QGCGeofence::distanceToNearestBoundary() const
{
    double minDist = std::numeric_limits<double>::max();

    for (const Fence &fence : _fences) {
        if (!fence.enabled) {
            continue;
        }

        const double dist = _distanceToFence(_position, fence);
        if (dist < minDist) {
            minDist = dist;
        }
    }

    return minDist == std::numeric_limits<double>::max() ? -1 : minDist;
}

bool QGCGeofence::doesPathBreachFence(const QList<QGeoCoordinate> &path, const QString &fenceId) const
{
    const Fence *targetFence = nullptr;
    for (const Fence &fence : _fences) {
        if (fence.id == fenceId) {
            targetFence = &fence;
            break;
        }
    }

    if (!targetFence) {
        return false;
    }

    for (const QGeoCoordinate &point : path) {
        const bool inside = _isInsideFence(point, *targetFence);
        if (targetFence->type == FenceType::Inclusion && !inside) {
            return true;
        }
        if (targetFence->type == FenceType::Exclusion && inside) {
            return true;
        }
    }

    return false;
}

QList<QGeoCoordinate> QGCGeofence::findBreachPoints(const QList<QGeoCoordinate> &path) const
{
    QList<QGeoCoordinate> breachPoints;

    for (const QGeoCoordinate &point : path) {
        for (const Fence &fence : _fences) {
            if (!fence.enabled) {
                continue;
            }

            const bool inside = _isInsideFence(point, fence);
            bool breached = false;

            if (fence.type == FenceType::Inclusion && !inside) {
                breached = true;
            } else if (fence.type == FenceType::Exclusion && inside) {
                breached = true;
            }

            if (breached && !breachPoints.contains(point)) {
                breachPoints.append(point);
            }
        }
    }

    return breachPoints;
}

QVariantMap QGCGeofence::toVariantMap() const
{
    QVariantMap map;
    QVariantList fenceList;

    for (const Fence &fence : _fences) {
        QVariantMap fenceMap;
        fenceMap[QStringLiteral("id")] = fence.id;
        fenceMap[QStringLiteral("name")] = fence.name;
        fenceMap[QStringLiteral("type")] = static_cast<int>(fence.type);
        fenceMap[QStringLiteral("shape")] = static_cast<int>(fence.shape);
        fenceMap[QStringLiteral("enabled")] = fence.enabled;
        fenceMap[QStringLiteral("minAltitude")] = fence.minAltitude;
        fenceMap[QStringLiteral("maxAltitude")] = fence.maxAltitude;

        switch (fence.shape) {
        case FenceShape::Circle:
            fenceMap[QStringLiteral("center")] = QVariant::fromValue(fence.circle.center());
            fenceMap[QStringLiteral("radius")] = fence.circle.radius();
            break;
        case FenceShape::Rectangle:
            fenceMap[QStringLiteral("topLeft")] = QVariant::fromValue(fence.rectangle.topLeft());
            fenceMap[QStringLiteral("bottomRight")] = QVariant::fromValue(fence.rectangle.bottomRight());
            break;
        case FenceShape::Polygon: {
            QVariantList vertices;
            for (const QGeoCoordinate &coord : fence.polygon.perimeter()) {
                vertices.append(QVariant::fromValue(coord));
            }
            fenceMap[QStringLiteral("vertices")] = vertices;
            break;
        }
        }

        fenceList.append(fenceMap);
    }

    map[QStringLiteral("fences")] = fenceList;
    map[QStringLiteral("nextId")] = _nextId;

    return map;
}

bool QGCGeofence::fromVariantMap(const QVariantMap &map)
{
    clearFences();

    _nextId = map.value(QStringLiteral("nextId"), 1).toInt();
    const QVariantList fenceList = map.value(QStringLiteral("fences")).toList();

    for (const QVariant &var : fenceList) {
        const QVariantMap fenceMap = var.toMap();

        Fence fence;
        fence.id = fenceMap.value(QStringLiteral("id")).toString();
        fence.name = fenceMap.value(QStringLiteral("name")).toString();
        fence.type = static_cast<FenceType>(fenceMap.value(QStringLiteral("type")).toInt());
        fence.shape = static_cast<FenceShape>(fenceMap.value(QStringLiteral("shape")).toInt());
        fence.enabled = fenceMap.value(QStringLiteral("enabled"), true).toBool();
        fence.minAltitude = fenceMap.value(QStringLiteral("minAltitude"), -1000).toDouble();
        fence.maxAltitude = fenceMap.value(QStringLiteral("maxAltitude"), 10000).toDouble();

        switch (fence.shape) {
        case FenceShape::Circle: {
            const QGeoCoordinate center = fenceMap.value(QStringLiteral("center")).value<QGeoCoordinate>();
            const double radius = fenceMap.value(QStringLiteral("radius")).toDouble();
            fence.circle = QGeoCircle(center, radius);
            break;
        }
        case FenceShape::Rectangle: {
            const QGeoCoordinate topLeft = fenceMap.value(QStringLiteral("topLeft")).value<QGeoCoordinate>();
            const QGeoCoordinate bottomRight = fenceMap.value(QStringLiteral("bottomRight")).value<QGeoCoordinate>();
            fence.rectangle = QGeoRectangle(topLeft, bottomRight);
            break;
        }
        case FenceShape::Polygon: {
            QList<QGeoCoordinate> vertices;
            const QVariantList vertexList = fenceMap.value(QStringLiteral("vertices")).toList();
            for (const QVariant &v : vertexList) {
                vertices.append(v.value<QGeoCoordinate>());
            }
            fence.polygon = QGeoPolygon(vertices);
            break;
        }
        }

        _fences.append(fence);
    }

    emit fenceCountChanged(fenceCount());
    _checkBreaches();

    return true;
}

void QGCGeofence::_checkBreaches()
{
    if (!_position.isValid()) {
        return;
    }

    QList<BreachInfo> newBreaches;
    QSet<QString> currentBreachedIds;

    for (const Fence &fence : _fences) {
        if (!fence.enabled) {
            continue;
        }

        const bool inside = _isInsideFence(_position, fence);
        const bool altValid = _isAltitudeValid(_altitude, fence);
        bool breached = false;

        if (fence.type == FenceType::Inclusion) {
            breached = !inside || !altValid;
        } else {
            breached = inside;
        }

        if (breached) {
            BreachInfo info;
            info.fenceId = fence.id;
            info.fenceName = fence.name;
            info.fenceType = fence.type;
            info.distance = _distanceToFence(_position, fence);
            newBreaches.append(info);
            currentBreachedIds.insert(fence.id);

            if (!_previousBreachedIds.contains(fence.id)) {
                emit fenceBreached(fence.id, fence.name, fence.type);
                qCWarning(QGCGeofenceLog) << "Fence breached:" << fence.name << "(" << fence.id << ")";
            }
        } else if (_previousBreachedIds.contains(fence.id)) {
            emit fenceCleared(fence.id, fence.name);
            qCInfo(QGCGeofenceLog) << "Fence cleared:" << fence.name << "(" << fence.id << ")";
        }
    }

    const bool wasBreached = !_currentBreaches.isEmpty();
    const bool isNowBreached = !newBreaches.isEmpty();

    _currentBreaches = newBreaches;
    _previousBreachedIds = currentBreachedIds;

    if (wasBreached != isNowBreached) {
        emit breachChanged(isNowBreached);
    }
}

bool QGCGeofence::_isInsideFence(const QGeoCoordinate &point, const Fence &fence) const
{
    switch (fence.shape) {
    case FenceShape::Circle:
        return fence.circle.contains(point);

    case FenceShape::Rectangle:
        return fence.rectangle.contains(point);

    case FenceShape::Polygon:
        return _pointInPolygon(point, fence.polygon);
    }

    return false;
}

bool QGCGeofence::_isAltitudeValid(double alt, const Fence &fence) const
{
    return alt >= fence.minAltitude && alt <= fence.maxAltitude;
}

double QGCGeofence::_distanceToFence(const QGeoCoordinate &point, const Fence &fence) const
{
    switch (fence.shape) {
    case FenceShape::Circle: {
        const double distToCenter = point.distanceTo(fence.circle.center());
        return fence.circle.radius() - distToCenter;  // Positive = inside, negative = outside
    }

    case FenceShape::Rectangle: {
        // Simplified: distance to nearest edge
        const QGeoCoordinate center = fence.rectangle.center();
        const double distToCenter = point.distanceTo(center);
        const double halfWidth = fence.rectangle.width() / 2.0 * 111320.0 * std::cos(qDegreesToRadians(center.latitude()));
        const double halfHeight = fence.rectangle.height() / 2.0 * 111320.0;
        const double approxRadius = std::min(halfWidth, halfHeight);
        return approxRadius - distToCenter;
    }

    case FenceShape::Polygon: {
        // Distance to nearest edge
        const QList<QGeoCoordinate> perimeter = fence.polygon.perimeter();
        double minDist = std::numeric_limits<double>::max();

        for (int i = 0; i < perimeter.size(); ++i) {
            const QGeoCoordinate &p1 = perimeter[i];
            const QGeoCoordinate &p2 = perimeter[(i + 1) % perimeter.size()];

            const double dist = point.distanceTo(p1);
            if (dist < minDist) {
                minDist = dist;
            }
        }

        const bool inside = _pointInPolygon(point, fence.polygon);
        return inside ? minDist : -minDist;
    }
    }

    return 0;
}

bool QGCGeofence::_pointInPolygon(const QGeoCoordinate &point, const QGeoPolygon &polygon) const
{
    const QList<QGeoCoordinate> perimeter = polygon.perimeter();
    if (perimeter.size() < 3) {
        return false;
    }

    // Ray casting algorithm
    bool inside = false;
    const double x = point.longitude();
    const double y = point.latitude();

    for (int i = 0, j = perimeter.size() - 1; i < perimeter.size(); j = i++) {
        const double xi = perimeter[i].longitude();
        const double yi = perimeter[i].latitude();
        const double xj = perimeter[j].longitude();
        const double yj = perimeter[j].latitude();

        if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
            inside = !inside;
        }
    }

    return inside;
}
