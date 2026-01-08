#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoPolygon>

Q_DECLARE_LOGGING_CATEGORY(QGCGeofenceLog)

/// Geofence management and breach detection.
///
/// Supports multiple fence types:
/// - Circular (center + radius)
/// - Rectangular (bounding box)
/// - Polygonal (arbitrary shape)
///
/// Example usage:
/// @code
/// QGCGeofence fence;
/// fence.addCircularFence("home", homeCoord, 500);  // 500m radius
/// fence.addPolygonFence("survey_area", polygonCoords);
/// fence.setPosition(currentPosition);
///
/// if (fence.isBreached()) {
///     qWarning() << "Fence breached:" << fence.breachedFences();
/// }
/// @endcode
class QGCGeofence : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude NOTIFY altitudeChanged)
    Q_PROPERTY(bool breached READ isBreached NOTIFY breachChanged)
    Q_PROPERTY(int fenceCount READ fenceCount NOTIFY fenceCountChanged)

public:
    enum class FenceType {
        Inclusion,  // Must stay inside
        Exclusion   // Must stay outside
    };
    Q_ENUM(FenceType)

    enum class FenceShape {
        Circle,
        Rectangle,
        Polygon
    };
    Q_ENUM(FenceShape)

    struct Fence {
        QString id;
        QString name;
        FenceType type = FenceType::Inclusion;
        FenceShape shape = FenceShape::Circle;
        bool enabled = true;
        double minAltitude = -1000;  // -1000 means no limit
        double maxAltitude = 10000;  // 10000 means no limit

        // Shape data (only one is used based on shape)
        QGeoCircle circle;
        QGeoRectangle rectangle;
        QGeoPolygon polygon;

        QVariantMap metadata;
    };

    struct BreachInfo {
        QString fenceId;
        QString fenceName;
        FenceType fenceType;
        double distance;      // Distance to breach point (negative = inside, positive = outside)
        QGeoCoordinate nearestPoint;  // Nearest point on fence boundary
    };

    explicit QGCGeofence(QObject *parent = nullptr);
    ~QGCGeofence() override = default;

    // Fence management
    QString addCircularFence(const QString &name, const QGeoCoordinate &center, double radiusMeters,
                              FenceType type = FenceType::Inclusion);
    QString addRectangularFence(const QString &name, const QGeoRectangle &rect,
                                 FenceType type = FenceType::Inclusion);
    QString addPolygonFence(const QString &name, const QList<QGeoCoordinate> &vertices,
                             FenceType type = FenceType::Inclusion);

    bool removeFence(const QString &id);
    void clearFences();
    bool setFenceEnabled(const QString &id, bool enabled);
    bool setFenceAltitudeLimits(const QString &id, double minAlt, double maxAlt);

    Fence fence(const QString &id) const;
    QList<Fence> fences() const { return _fences; }
    int fenceCount() const { return static_cast<int>(_fences.size()); }
    bool hasFence(const QString &id) const;
    QStringList fenceIds() const;

    // Position tracking
    QGeoCoordinate position() const { return _position; }
    void setPosition(const QGeoCoordinate &position);
    double altitude() const { return _altitude; }
    void setAltitude(double altitude);

    // Breach detection
    bool isBreached() const { return !_currentBreaches.isEmpty(); }
    QList<BreachInfo> currentBreaches() const { return _currentBreaches; }
    QStringList breachedFenceIds() const;
    QStringList breachedFenceNames() const;

    // Point-in-fence checks
    Q_INVOKABLE bool isPointInFence(const QGeoCoordinate &point, const QString &fenceId) const;
    Q_INVOKABLE bool isPointInAnyFence(const QGeoCoordinate &point) const;
    Q_INVOKABLE double distanceToFence(const QGeoCoordinate &point, const QString &fenceId) const;
    Q_INVOKABLE double distanceToNearestBoundary() const;

    // Path checking
    bool doesPathBreachFence(const QList<QGeoCoordinate> &path, const QString &fenceId) const;
    QList<QGeoCoordinate> findBreachPoints(const QList<QGeoCoordinate> &path) const;

    // Serialization
    QVariantMap toVariantMap() const;
    bool fromVariantMap(const QVariantMap &map);

signals:
    void positionChanged(const QGeoCoordinate &position);
    void altitudeChanged(double altitude);
    void breachChanged(bool breached);
    void fenceBreached(const QString &fenceId, const QString &fenceName, FenceType type);
    void fenceCleared(const QString &fenceId, const QString &fenceName);
    void fenceAdded(const QString &fenceId);
    void fenceRemoved(const QString &fenceId);
    void fenceCountChanged(int count);
    void warningZoneEntered(const QString &fenceId, double distanceToBreach);

private:
    void _checkBreaches();
    bool _isInsideFence(const QGeoCoordinate &point, const Fence &fence) const;
    bool _isAltitudeValid(double alt, const Fence &fence) const;
    double _distanceToFence(const QGeoCoordinate &point, const Fence &fence) const;
    bool _pointInPolygon(const QGeoCoordinate &point, const QGeoPolygon &polygon) const;

    QList<Fence> _fences;
    QGeoCoordinate _position;
    double _altitude = 0;
    QList<BreachInfo> _currentBreaches;
    QSet<QString> _previousBreachedIds;
    int _nextId = 1;
};
