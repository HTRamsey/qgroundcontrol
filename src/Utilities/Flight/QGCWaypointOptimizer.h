#pragma once

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(QGCWaypointOptimizerLog)

/// Waypoint path optimization utilities.
///
/// Provides algorithms for optimizing flight paths including:
/// - Path simplification (Douglas-Peucker)
/// - TSP-style ordering optimization
/// - Altitude smoothing
/// - Turn radius constraints
///
/// Example usage:
/// @code
/// QGCWaypointOptimizer optimizer;
/// QList<QGeoCoordinate> waypoints = {...};
/// QList<QGeoCoordinate> simplified = optimizer.simplify(waypoints, 5.0);  // 5m tolerance
/// QList<QGeoCoordinate> ordered = optimizer.optimizeOrder(waypoints);
/// @endcode
class QGCWaypointOptimizer : public QObject
{
    Q_OBJECT

public:
    struct Waypoint {
        QGeoCoordinate coordinate;
        double altitude = 0;
        double speed = 0;
        int index = -1;  // Original index
    };

    struct OptimizationResult {
        QList<Waypoint> waypoints;
        double originalDistance;
        double optimizedDistance;
        double distanceSaved;
        double percentSaved;
        int pointsRemoved;
    };

    explicit QGCWaypointOptimizer(QObject *parent = nullptr);
    ~QGCWaypointOptimizer() override = default;

    // Path simplification (Douglas-Peucker algorithm)
    static QList<QGeoCoordinate> simplify(const QList<QGeoCoordinate> &path, double toleranceMeters);
    static OptimizationResult simplifyWithStats(const QList<Waypoint> &waypoints, double toleranceMeters);

    // Radial distance simplification (faster, less accurate)
    static QList<QGeoCoordinate> simplifyRadial(const QList<QGeoCoordinate> &path, double toleranceMeters);

    // Order optimization (nearest neighbor TSP approximation)
    static QList<QGeoCoordinate> optimizeOrder(const QList<QGeoCoordinate> &waypoints);
    static QList<QGeoCoordinate> optimizeOrderFromStart(const QList<QGeoCoordinate> &waypoints,
                                                         const QGeoCoordinate &startPoint);

    // 2-opt improvement for TSP
    static QList<QGeoCoordinate> improve2Opt(const QList<QGeoCoordinate> &waypoints, int maxIterations = 100);

    // Altitude smoothing
    static QList<Waypoint> smoothAltitudes(const QList<Waypoint> &waypoints, double maxClimbRateMps);
    static QList<Waypoint> interpolateAltitudes(const QList<Waypoint> &waypoints, int numPoints);

    // Turn radius constraints
    static QList<QGeoCoordinate> addTurnPoints(const QList<QGeoCoordinate> &path, double turnRadiusMeters);
    static bool checkTurnRadius(const QList<QGeoCoordinate> &path, double minTurnRadiusMeters);

    // Distance calculations
    static double totalDistance(const QList<QGeoCoordinate> &path);
    static double totalDistance(const QList<Waypoint> &waypoints);

    // Path analysis
    static double maxTurnAngle(const QList<QGeoCoordinate> &path);
    static double averageSegmentLength(const QList<QGeoCoordinate> &path);
    static QList<int> findSharpTurns(const QList<QGeoCoordinate> &path, double thresholdDegrees);

    // Coordinate utilities
    static double distanceBetween(const QGeoCoordinate &a, const QGeoCoordinate &b);
    static double bearingTo(const QGeoCoordinate &from, const QGeoCoordinate &to);
    static double angleBetween(const QGeoCoordinate &a, const QGeoCoordinate &b, const QGeoCoordinate &c);
    static QGeoCoordinate interpolate(const QGeoCoordinate &from, const QGeoCoordinate &to, double fraction);
    static QGeoCoordinate atDistanceAndBearing(const QGeoCoordinate &from, double distanceMeters, double bearingDegrees);

private:
    static double _perpendicularDistance(const QGeoCoordinate &point,
                                          const QGeoCoordinate &lineStart,
                                          const QGeoCoordinate &lineEnd);
    static void _douglasPeucker(const QList<QGeoCoordinate> &points, int start, int end,
                                 double tolerance, QList<bool> &keep);
};
