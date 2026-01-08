#include "GeoUtilities.h"

#include <QtCore/QObject>
#include <cmath>

namespace GeoUtilities
{

// ============================================================================
// Bounding Box Utilities
// ============================================================================

QGeoRectangle boundingBox(const QList<QGeoCoordinate> &coords)
{
    if (coords.isEmpty()) {
        return QGeoRectangle();
    }

    QGeoRectangle box(coords.first(), coords.first());
    for (int i = 1; i < coords.size(); ++i) {
        expandBoundingBox(box, coords[i]);
    }
    return box;
}

QGeoRectangle boundingBox(const QList<QList<QGeoCoordinate>> &coordLists)
{
    QGeoRectangle box;
    for (const auto &coords : coordLists) {
        if (coords.isEmpty()) {
            continue;
        }
        if (!box.isValid()) {
            box = boundingBox(coords);
        } else {
            expandBoundingBox(box, boundingBox(coords));
        }
    }
    return box;
}

void expandBoundingBox(QGeoRectangle &box, const QGeoCoordinate &coord)
{
    if (!coord.isValid()) {
        return;
    }

    if (!box.isValid()) {
        box = QGeoRectangle(coord, coord);
        return;
    }

    // QGeoRectangle::extendRectangle is available in Qt 6
    box.extendRectangle(coord);
}

void expandBoundingBox(QGeoRectangle &box, const QGeoRectangle &other)
{
    if (!other.isValid()) {
        return;
    }

    if (!box.isValid()) {
        box = other;
        return;
    }

    box = box.united(other);
}

// ============================================================================
// Coordinate Normalization
// ============================================================================

double normalizeLongitude(double longitude)
{
    // Wrap to [-180, 180]
    while (longitude > 180.0) {
        longitude -= 360.0;
    }
    while (longitude < -180.0) {
        longitude += 360.0;
    }
    return longitude;
}

double normalizeLatitude(double latitude)
{
    // Clamp to [-90, 90] - latitude cannot wrap
    if (latitude > 90.0) {
        return 90.0;
    }
    if (latitude < -90.0) {
        return -90.0;
    }
    return latitude;
}

QGeoCoordinate normalizeCoordinate(const QGeoCoordinate &coord)
{
    if (!coord.isValid()) {
        return coord;
    }

    QGeoCoordinate normalized(
        normalizeLatitude(coord.latitude()),
        normalizeLongitude(coord.longitude())
    );

    if (!std::isnan(coord.altitude())) {
        normalized.setAltitude(coord.altitude());
    }

    return normalized;
}

void normalizeCoordinates(QList<QGeoCoordinate> &coords)
{
    for (int i = 0; i < coords.size(); ++i) {
        coords[i] = normalizeCoordinate(coords[i]);
    }
}

// ============================================================================
// Altitude Validation
// ============================================================================

bool isValidAltitude(double altitude, double minAltitude, double maxAltitude)
{
    if (std::isnan(altitude)) {
        return true;  // No altitude is valid
    }
    return altitude >= minAltitude && altitude <= maxAltitude;
}

bool isValidAltitude(const QGeoCoordinate &coord, double minAltitude, double maxAltitude)
{
    return isValidAltitude(coord.altitude(), minAltitude, maxAltitude);
}

bool validateAltitudes(const QList<QGeoCoordinate> &coords, QString &errorString,
                       double minAltitude, double maxAltitude)
{
    for (int i = 0; i < coords.size(); ++i) {
        const double alt = coords[i].altitude();
        if (!std::isnan(alt) && (alt < minAltitude || alt > maxAltitude)) {
            errorString = QObject::tr("Altitude %1m at index %2 is outside valid range [%3, %4]")
                .arg(alt, 0, 'f', 1)
                .arg(i)
                .arg(minAltitude, 0, 'f', 0)
                .arg(maxAltitude, 0, 'f', 0);
            return false;
        }
    }
    return true;
}

// ============================================================================
// Polygon Validation
// ============================================================================

namespace {
    // Check if two line segments intersect (2D)
    // Segments: (p1, p2) and (p3, p4)
    bool segmentsIntersect(const QGeoCoordinate &p1, const QGeoCoordinate &p2,
                           const QGeoCoordinate &p3, const QGeoCoordinate &p4)
    {
        // Using cross product method
        auto cross = [](double ax, double ay, double bx, double by) {
            return ax * by - ay * bx;
        };

        const double x1 = p1.longitude(), y1 = p1.latitude();
        const double x2 = p2.longitude(), y2 = p2.latitude();
        const double x3 = p3.longitude(), y3 = p3.latitude();
        const double x4 = p4.longitude(), y4 = p4.latitude();

        const double d1 = cross(x4 - x3, y4 - y3, x1 - x3, y1 - y3);
        const double d2 = cross(x4 - x3, y4 - y3, x2 - x3, y2 - y3);
        const double d3 = cross(x2 - x1, y2 - y1, x3 - x1, y3 - y1);
        const double d4 = cross(x2 - x1, y2 - y1, x4 - x1, y4 - y1);

        // Check if segments straddle each other
        if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
            ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
            return true;
        }

        // Check for collinear cases (endpoints on the other segment)
        constexpr double epsilon = 1e-10;
        if (std::abs(d1) < epsilon || std::abs(d2) < epsilon ||
            std::abs(d3) < epsilon || std::abs(d4) < epsilon) {
            // Collinear - check if segments overlap
            auto onSegment = [](double px, double py, double qx, double qy, double rx, double ry) {
                return qx <= std::max(px, rx) && qx >= std::min(px, rx) &&
                       qy <= std::max(py, ry) && qy >= std::min(py, ry);
            };

            if (std::abs(d1) < epsilon && onSegment(x3, y3, x1, y1, x4, y4)) return true;
            if (std::abs(d2) < epsilon && onSegment(x3, y3, x2, y2, x4, y4)) return true;
            if (std::abs(d3) < epsilon && onSegment(x1, y1, x3, y3, x2, y2)) return true;
            if (std::abs(d4) < epsilon && onSegment(x1, y1, x4, y4, x2, y2)) return true;
        }

        return false;
    }
}

bool isSelfIntersecting(const QList<QGeoCoordinate> &vertices)
{
    const int n = vertices.size();
    if (n < 4) {
        return false;  // Triangle or less cannot self-intersect
    }

    // Check each pair of non-adjacent edges
    for (int i = 0; i < n; ++i) {
        const int i2 = (i + 1) % n;

        for (int j = i + 2; j < n; ++j) {
            // Skip adjacent edges
            if (i == 0 && j == n - 1) {
                continue;  // First and last edge share a vertex
            }

            const int j2 = (j + 1) % n;

            if (segmentsIntersect(vertices[i], vertices[i2], vertices[j], vertices[j2])) {
                return true;
            }
        }
    }

    return false;
}

bool isClockwise(const QList<QGeoCoordinate> &vertices)
{
    if (vertices.size() < 3) {
        return true;  // Degenerate
    }

    // Calculate signed area using Shoelace formula
    double sum = 0.0;
    const int n = vertices.size();

    for (int i = 0; i < n; ++i) {
        const int j = (i + 1) % n;
        sum += (vertices[j].longitude() - vertices[i].longitude()) *
               (vertices[j].latitude() + vertices[i].latitude());
    }

    // Positive sum = clockwise (in lat/lon coordinate system)
    return sum > 0.0;
}

void reverseVertices(QList<QGeoCoordinate> &vertices)
{
    std::reverse(vertices.begin(), vertices.end());
}

bool validatePolygon(const QList<QGeoCoordinate> &vertices, QString &errorString)
{
    if (vertices.size() < 3) {
        errorString = QObject::tr("Polygon must have at least 3 vertices, found %1").arg(vertices.size());
        return false;
    }

    // Check for invalid coordinates
    for (int i = 0; i < vertices.size(); ++i) {
        if (!vertices[i].isValid()) {
            errorString = QObject::tr("Invalid coordinate at index %1").arg(i);
            return false;
        }
    }

    // Check for self-intersection
    if (isSelfIntersecting(vertices)) {
        errorString = QObject::tr("Polygon is self-intersecting");
        return false;
    }

    // Check for degenerate polygon (zero area)
    double area = 0.0;
    const int n = vertices.size();
    for (int i = 0; i < n; ++i) {
        const int j = (i + 1) % n;
        area += vertices[i].longitude() * vertices[j].latitude();
        area -= vertices[j].longitude() * vertices[i].latitude();
    }
    area = std::abs(area) / 2.0;

    if (area < 1e-10) {
        errorString = QObject::tr("Polygon has zero or near-zero area");
        return false;
    }

    return true;
}

// ============================================================================
// Coordinate Validation
// ============================================================================

bool isValidCoordinate(const QGeoCoordinate &coord)
{
    if (!coord.isValid()) {
        return false;
    }

    const double lat = coord.latitude();
    const double lon = coord.longitude();

    return lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
}

bool validateCoordinates(const QList<QGeoCoordinate> &coords, QString &errorString)
{
    for (int i = 0; i < coords.size(); ++i) {
        if (!isValidCoordinate(coords[i])) {
            if (!coords[i].isValid()) {
                errorString = QObject::tr("Invalid coordinate at index %1").arg(i);
            } else {
                errorString = QObject::tr("Coordinate at index %1 out of range: lat=%2, lon=%3")
                    .arg(i)
                    .arg(coords[i].latitude())
                    .arg(coords[i].longitude());
            }
            return false;
        }
    }
    return true;
}

} // namespace GeoUtilities
