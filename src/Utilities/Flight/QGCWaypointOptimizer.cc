#include "QGCWaypointOptimizer.h"

#include <QtCore/QtMath>
#include <algorithm>
#include <limits>

Q_LOGGING_CATEGORY(QGCWaypointOptimizerLog, "Utilities.QGCWaypointOptimizer")

namespace {
constexpr double kEarthRadiusMeters = 6371000.0;

double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}
}

QGCWaypointOptimizer::QGCWaypointOptimizer(QObject *parent)
    : QObject(parent)
{
}

QList<QGeoCoordinate> QGCWaypointOptimizer::simplify(const QList<QGeoCoordinate> &path, double toleranceMeters)
{
    if (path.size() <= 2) {
        return path;
    }

    QList<bool> keep(path.size(), false);
    keep[0] = true;
    keep[path.size() - 1] = true;

    _douglasPeucker(path, 0, path.size() - 1, toleranceMeters, keep);

    QList<QGeoCoordinate> result;
    for (int i = 0; i < path.size(); ++i) {
        if (keep[i]) {
            result.append(path[i]);
        }
    }

    qCDebug(QGCWaypointOptimizerLog) << "Simplified path from" << path.size() << "to" << result.size() << "points";
    return result;
}

QGCWaypointOptimizer::OptimizationResult QGCWaypointOptimizer::simplifyWithStats(
    const QList<Waypoint> &waypoints, double toleranceMeters)
{
    OptimizationResult result;
    result.originalDistance = totalDistance(waypoints);

    if (waypoints.size() <= 2) {
        result.waypoints = waypoints;
        result.optimizedDistance = result.originalDistance;
        result.distanceSaved = 0;
        result.percentSaved = 0;
        result.pointsRemoved = 0;
        return result;
    }

    QList<QGeoCoordinate> coords;
    for (const Waypoint &wp : waypoints) {
        coords.append(wp.coordinate);
    }

    QList<bool> keep(waypoints.size(), false);
    keep[0] = true;
    keep[waypoints.size() - 1] = true;
    _douglasPeucker(coords, 0, coords.size() - 1, toleranceMeters, keep);

    for (int i = 0; i < waypoints.size(); ++i) {
        if (keep[i]) {
            result.waypoints.append(waypoints[i]);
        }
    }

    result.optimizedDistance = totalDistance(result.waypoints);
    result.distanceSaved = result.originalDistance - result.optimizedDistance;
    result.percentSaved = (result.distanceSaved / result.originalDistance) * 100.0;
    result.pointsRemoved = waypoints.size() - result.waypoints.size();

    return result;
}

QList<QGeoCoordinate> QGCWaypointOptimizer::simplifyRadial(const QList<QGeoCoordinate> &path, double toleranceMeters)
{
    if (path.size() <= 2) {
        return path;
    }

    QList<QGeoCoordinate> result;
    result.append(path.first());

    QGeoCoordinate lastKept = path.first();
    for (int i = 1; i < path.size() - 1; ++i) {
        if (distanceBetween(lastKept, path[i]) >= toleranceMeters) {
            result.append(path[i]);
            lastKept = path[i];
        }
    }

    result.append(path.last());
    return result;
}

QList<QGeoCoordinate> QGCWaypointOptimizer::optimizeOrder(const QList<QGeoCoordinate> &waypoints)
{
    if (waypoints.size() <= 2) {
        return waypoints;
    }

    return optimizeOrderFromStart(waypoints, waypoints.first());
}

QList<QGeoCoordinate> QGCWaypointOptimizer::optimizeOrderFromStart(const QList<QGeoCoordinate> &waypoints,
                                                                     const QGeoCoordinate &startPoint)
{
    if (waypoints.size() <= 2) {
        return waypoints;
    }

    // Nearest neighbor algorithm
    QList<QGeoCoordinate> remaining = waypoints;
    QList<QGeoCoordinate> result;

    QGeoCoordinate current = startPoint;

    while (!remaining.isEmpty()) {
        int nearestIdx = 0;
        double nearestDist = std::numeric_limits<double>::max();

        for (int i = 0; i < remaining.size(); ++i) {
            const double dist = distanceBetween(current, remaining[i]);
            if (dist < nearestDist) {
                nearestDist = dist;
                nearestIdx = i;
            }
        }

        result.append(remaining[nearestIdx]);
        current = remaining[nearestIdx];
        remaining.removeAt(nearestIdx);
    }

    qCDebug(QGCWaypointOptimizerLog) << "Optimized order, distance:" << totalDistance(result) << "m";
    return result;
}

QList<QGeoCoordinate> QGCWaypointOptimizer::improve2Opt(const QList<QGeoCoordinate> &waypoints, int maxIterations)
{
    if (waypoints.size() <= 3) {
        return waypoints;
    }

    QList<QGeoCoordinate> result = waypoints;
    bool improved = true;
    int iterations = 0;

    while (improved && iterations < maxIterations) {
        improved = false;
        ++iterations;

        for (int i = 0; i < result.size() - 2; ++i) {
            for (int j = i + 2; j < result.size() - 1; ++j) {
                // Calculate current distance
                const double d1 = distanceBetween(result[i], result[i + 1]);
                const double d2 = distanceBetween(result[j], result[j + 1]);

                // Calculate new distance after swap
                const double d3 = distanceBetween(result[i], result[j]);
                const double d4 = distanceBetween(result[i + 1], result[j + 1]);

                if (d3 + d4 < d1 + d2) {
                    // Reverse the segment between i+1 and j
                    std::reverse(result.begin() + i + 1, result.begin() + j + 1);
                    improved = true;
                }
            }
        }
    }

    qCDebug(QGCWaypointOptimizerLog) << "2-opt improved in" << iterations << "iterations";
    return result;
}

QList<QGCWaypointOptimizer::Waypoint> QGCWaypointOptimizer::smoothAltitudes(
    const QList<Waypoint> &waypoints, double maxClimbRateMps)
{
    if (waypoints.size() <= 1) {
        return waypoints;
    }

    QList<Waypoint> result = waypoints;

    // Forward pass - limit climb rate
    for (int i = 1; i < result.size(); ++i) {
        const double dist = distanceBetween(result[i - 1].coordinate, result[i].coordinate);
        const double speed = result[i].speed > 0 ? result[i].speed : 10.0;  // Default 10 m/s
        const double time = dist / speed;
        const double maxAltChange = time * maxClimbRateMps;

        const double altDiff = result[i].altitude - result[i - 1].altitude;
        if (std::abs(altDiff) > maxAltChange) {
            result[i].altitude = result[i - 1].altitude + (altDiff > 0 ? maxAltChange : -maxAltChange);
        }
    }

    // Backward pass - limit descent rate
    for (int i = result.size() - 2; i >= 0; --i) {
        const double dist = distanceBetween(result[i].coordinate, result[i + 1].coordinate);
        const double speed = result[i].speed > 0 ? result[i].speed : 10.0;
        const double time = dist / speed;
        const double maxAltChange = time * maxClimbRateMps;

        const double altDiff = result[i].altitude - result[i + 1].altitude;
        if (std::abs(altDiff) > maxAltChange) {
            result[i].altitude = result[i + 1].altitude + (altDiff > 0 ? maxAltChange : -maxAltChange);
        }
    }

    return result;
}

QList<QGCWaypointOptimizer::Waypoint> QGCWaypointOptimizer::interpolateAltitudes(
    const QList<Waypoint> &waypoints, int numPoints)
{
    if (waypoints.size() < 2 || numPoints < 2) {
        return waypoints;
    }

    QList<Waypoint> result;
    const double totalDist = totalDistance(waypoints);
    const double segmentDist = totalDist / (numPoints - 1);

    result.append(waypoints.first());
    double accumulatedDist = 0;
    double targetDist = segmentDist;
    int currentIdx = 0;

    while (result.size() < numPoints - 1 && currentIdx < waypoints.size() - 1) {
        const double segDist = distanceBetween(waypoints[currentIdx].coordinate,
                                                waypoints[currentIdx + 1].coordinate);

        while (accumulatedDist + segDist >= targetDist && result.size() < numPoints - 1) {
            const double fraction = (targetDist - accumulatedDist) / segDist;

            Waypoint wp;
            wp.coordinate = interpolate(waypoints[currentIdx].coordinate,
                                        waypoints[currentIdx + 1].coordinate, fraction);
            wp.altitude = waypoints[currentIdx].altitude +
                         (waypoints[currentIdx + 1].altitude - waypoints[currentIdx].altitude) * fraction;
            wp.speed = waypoints[currentIdx].speed +
                      (waypoints[currentIdx + 1].speed - waypoints[currentIdx].speed) * fraction;
            result.append(wp);

            targetDist += segmentDist;
        }

        accumulatedDist += segDist;
        ++currentIdx;
    }

    result.append(waypoints.last());
    return result;
}

QList<QGeoCoordinate> QGCWaypointOptimizer::addTurnPoints(const QList<QGeoCoordinate> &path, double turnRadiusMeters)
{
    if (path.size() < 3) {
        return path;
    }

    QList<QGeoCoordinate> result;
    result.append(path.first());

    for (int i = 1; i < path.size() - 1; ++i) {
        const double angle = angleBetween(path[i - 1], path[i], path[i + 1]);

        if (angle > 10.0) {  // Only add turn points for significant turns
            const double distBefore = distanceBetween(path[i - 1], path[i]);
            const double distAfter = distanceBetween(path[i], path[i + 1]);

            // Calculate turn point distance from vertex
            const double turnDist = std::min(turnRadiusMeters, std::min(distBefore, distAfter) / 3.0);

            // Add entry point
            const double fractionBefore = 1.0 - (turnDist / distBefore);
            result.append(interpolate(path[i - 1], path[i], fractionBefore));

            // Add the actual waypoint
            result.append(path[i]);

            // Add exit point
            const double fractionAfter = turnDist / distAfter;
            result.append(interpolate(path[i], path[i + 1], fractionAfter));
        } else {
            result.append(path[i]);
        }
    }

    result.append(path.last());
    return result;
}

bool QGCWaypointOptimizer::checkTurnRadius(const QList<QGeoCoordinate> &path, double minTurnRadiusMeters)
{
    for (int i = 1; i < path.size() - 1; ++i) {
        const double angle = angleBetween(path[i - 1], path[i], path[i + 1]);
        const double distBefore = distanceBetween(path[i - 1], path[i]);
        const double distAfter = distanceBetween(path[i], path[i + 1]);

        // Approximate turn radius
        const double avgDist = (distBefore + distAfter) / 2.0;
        const double angleRad = toRadians(angle);
        const double radius = avgDist / (2.0 * std::sin(angleRad / 2.0));

        if (radius < minTurnRadiusMeters) {
            return false;
        }
    }
    return true;
}

double QGCWaypointOptimizer::totalDistance(const QList<QGeoCoordinate> &path)
{
    double total = 0;
    for (int i = 1; i < path.size(); ++i) {
        total += distanceBetween(path[i - 1], path[i]);
    }
    return total;
}

double QGCWaypointOptimizer::totalDistance(const QList<Waypoint> &waypoints)
{
    double total = 0;
    for (int i = 1; i < waypoints.size(); ++i) {
        total += distanceBetween(waypoints[i - 1].coordinate, waypoints[i].coordinate);
    }
    return total;
}

double QGCWaypointOptimizer::maxTurnAngle(const QList<QGeoCoordinate> &path)
{
    double maxAngle = 0;
    for (int i = 1; i < path.size() - 1; ++i) {
        const double angle = angleBetween(path[i - 1], path[i], path[i + 1]);
        maxAngle = std::max(maxAngle, angle);
    }
    return maxAngle;
}

double QGCWaypointOptimizer::averageSegmentLength(const QList<QGeoCoordinate> &path)
{
    if (path.size() < 2) {
        return 0;
    }
    return totalDistance(path) / (path.size() - 1);
}

QList<int> QGCWaypointOptimizer::findSharpTurns(const QList<QGeoCoordinate> &path, double thresholdDegrees)
{
    QList<int> sharpTurns;
    for (int i = 1; i < path.size() - 1; ++i) {
        const double angle = angleBetween(path[i - 1], path[i], path[i + 1]);
        if (angle > thresholdDegrees) {
            sharpTurns.append(i);
        }
    }
    return sharpTurns;
}

double QGCWaypointOptimizer::distanceBetween(const QGeoCoordinate &a, const QGeoCoordinate &b)
{
    return a.distanceTo(b);
}

double QGCWaypointOptimizer::bearingTo(const QGeoCoordinate &from, const QGeoCoordinate &to)
{
    return from.azimuthTo(to);
}

double QGCWaypointOptimizer::angleBetween(const QGeoCoordinate &a, const QGeoCoordinate &b, const QGeoCoordinate &c)
{
    const double bearing1 = bearingTo(b, a);
    const double bearing2 = bearingTo(b, c);

    double angle = std::abs(bearing2 - bearing1);
    if (angle > 180.0) {
        angle = 360.0 - angle;
    }

    return 180.0 - angle;  // Return the turn angle, not the internal angle
}

QGeoCoordinate QGCWaypointOptimizer::interpolate(const QGeoCoordinate &from, const QGeoCoordinate &to, double fraction)
{
    if (fraction <= 0) return from;
    if (fraction >= 1) return to;

    const double lat1 = toRadians(from.latitude());
    const double lon1 = toRadians(from.longitude());
    const double lat2 = toRadians(to.latitude());
    const double lon2 = toRadians(to.longitude());

    const double dLat = lat2 - lat1;
    const double dLon = lon2 - lon1;

    const double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
                     std::cos(lat1) * std::cos(lat2) * std::sin(dLon / 2) * std::sin(dLon / 2);
    const double delta = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    const double A = std::sin((1 - fraction) * delta) / std::sin(delta);
    const double B = std::sin(fraction * delta) / std::sin(delta);

    const double x = A * std::cos(lat1) * std::cos(lon1) + B * std::cos(lat2) * std::cos(lon2);
    const double y = A * std::cos(lat1) * std::sin(lon1) + B * std::cos(lat2) * std::sin(lon2);
    const double z = A * std::sin(lat1) + B * std::sin(lat2);

    const double lat = std::atan2(z, std::sqrt(x * x + y * y));
    const double lon = std::atan2(y, x);

    const double alt = from.altitude() + (to.altitude() - from.altitude()) * fraction;

    return QGeoCoordinate(toDegrees(lat), toDegrees(lon), alt);
}

QGeoCoordinate QGCWaypointOptimizer::atDistanceAndBearing(const QGeoCoordinate &from,
                                                           double distanceMeters, double bearingDegrees)
{
    const double lat1 = toRadians(from.latitude());
    const double lon1 = toRadians(from.longitude());
    const double bearing = toRadians(bearingDegrees);
    const double angularDist = distanceMeters / kEarthRadiusMeters;

    const double lat2 = std::asin(std::sin(lat1) * std::cos(angularDist) +
                                   std::cos(lat1) * std::sin(angularDist) * std::cos(bearing));
    const double lon2 = lon1 + std::atan2(std::sin(bearing) * std::sin(angularDist) * std::cos(lat1),
                                           std::cos(angularDist) - std::sin(lat1) * std::sin(lat2));

    return QGeoCoordinate(toDegrees(lat2), toDegrees(lon2), from.altitude());
}

double QGCWaypointOptimizer::_perpendicularDistance(const QGeoCoordinate &point,
                                                      const QGeoCoordinate &lineStart,
                                                      const QGeoCoordinate &lineEnd)
{
    const double lineDist = distanceBetween(lineStart, lineEnd);
    if (lineDist < 0.001) {  // Points are essentially the same
        return distanceBetween(point, lineStart);
    }

    // Project point onto line and find perpendicular distance
    const double bearing1 = bearingTo(lineStart, lineEnd);
    const double bearing2 = bearingTo(lineStart, point);
    const double dist = distanceBetween(lineStart, point);

    const double crossTrack = std::asin(std::sin(dist / kEarthRadiusMeters) *
                                         std::sin(toRadians(bearing2 - bearing1))) * kEarthRadiusMeters;

    return std::abs(crossTrack);
}

void QGCWaypointOptimizer::_douglasPeucker(const QList<QGeoCoordinate> &points, int start, int end,
                                            double tolerance, QList<bool> &keep)
{
    if (end <= start + 1) {
        return;
    }

    double maxDist = 0;
    int maxIdx = start;

    for (int i = start + 1; i < end; ++i) {
        const double dist = _perpendicularDistance(points[i], points[start], points[end]);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }

    if (maxDist > tolerance) {
        keep[maxIdx] = true;
        _douglasPeucker(points, start, maxIdx, tolerance, keep);
        _douglasPeucker(points, maxIdx, end, tolerance, keep);
    }
}
