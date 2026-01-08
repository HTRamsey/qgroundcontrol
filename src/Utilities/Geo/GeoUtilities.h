#pragma once

#include <QtCore/QList>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoRectangle>

/// Geographic utility functions for validation, normalization, and geometric operations.
namespace GeoUtilities
{
    // ========================================================================
    // Bounding Box Utilities (using QGeoRectangle)
    // ========================================================================

    /// Compute bounding box for a list of coordinates
    QGeoRectangle boundingBox(const QList<QGeoCoordinate> &coords);

    /// Compute bounding box for multiple coordinate lists (e.g., multiple polygons)
    QGeoRectangle boundingBox(const QList<QList<QGeoCoordinate>> &coordLists);

    /// Expand a bounding box to include a coordinate
    void expandBoundingBox(QGeoRectangle &box, const QGeoCoordinate &coord);

    /// Expand a bounding box to include another bounding box
    void expandBoundingBox(QGeoRectangle &box, const QGeoRectangle &other);

    // ========================================================================
    // Coordinate Normalization
    // ========================================================================

    /// Normalize longitude to [-180, 180] range
    /// @param longitude Input longitude (may be outside normal range)
    /// @return Normalized longitude in [-180, 180]
    double normalizeLongitude(double longitude);

    /// Normalize latitude to [-90, 90] range (clamped, not wrapped)
    /// @param latitude Input latitude (may be outside normal range)
    /// @return Clamped latitude in [-90, 90]
    double normalizeLatitude(double latitude);

    /// Normalize a coordinate (longitude wrapped, latitude clamped)
    /// @param coord Input coordinate
    /// @return New coordinate with normalized lat/lon, altitude preserved
    QGeoCoordinate normalizeCoordinate(const QGeoCoordinate &coord);

    /// Normalize all coordinates in a list
    void normalizeCoordinates(QList<QGeoCoordinate> &coords);

    // ========================================================================
    // Altitude Validation
    // ========================================================================

    /// Default altitude limits for drone operations
    static constexpr double kDefaultMinAltitude = -500.0;   // Below sea level (Dead Sea, mines)
    static constexpr double kDefaultMaxAltitude = 10000.0;  // ~33,000 ft (commercial airspace)

    /// Validate that an altitude is within acceptable range
    /// @param altitude Altitude to validate (meters AMSL)
    /// @param minAltitude Minimum allowed altitude (default: -500m)
    /// @param maxAltitude Maximum allowed altitude (default: 10000m)
    /// @return true if altitude is valid or NaN (no altitude)
    bool isValidAltitude(double altitude,
                         double minAltitude = kDefaultMinAltitude,
                         double maxAltitude = kDefaultMaxAltitude);

    /// Validate altitude of a coordinate
    /// @return true if coordinate has no altitude or altitude is in range
    bool isValidAltitude(const QGeoCoordinate &coord,
                         double minAltitude = kDefaultMinAltitude,
                         double maxAltitude = kDefaultMaxAltitude);

    /// Validate altitudes of all coordinates in a list
    /// @param coords Coordinates to validate
    /// @param errorString Output: description of first invalid altitude found
    /// @param minAltitude Minimum allowed altitude
    /// @param maxAltitude Maximum allowed altitude
    /// @return true if all altitudes are valid
    bool validateAltitudes(const QList<QGeoCoordinate> &coords, QString &errorString,
                           double minAltitude = kDefaultMinAltitude,
                           double maxAltitude = kDefaultMaxAltitude);

    // ========================================================================
    // Polygon Validation
    // ========================================================================

    /// Check if a polygon is self-intersecting (2D check, ignores altitude)
    /// Uses the Shamos-Hoey algorithm concept with O(n²) simple implementation
    /// @param vertices Polygon vertices (open or closed)
    /// @return true if any non-adjacent edges intersect
    bool isSelfIntersecting(const QList<QGeoCoordinate> &vertices);

    /// Check if polygon vertices are in clockwise order (2D)
    /// @param vertices Polygon vertices
    /// @return true if clockwise, false if counter-clockwise
    bool isClockwise(const QList<QGeoCoordinate> &vertices);

    /// Reverse vertex order (to change winding direction)
    void reverseVertices(QList<QGeoCoordinate> &vertices);

    /// Validate a polygon for common issues
    /// @param vertices Polygon vertices
    /// @param errorString Output: description of validation errors
    /// @return true if polygon is valid
    bool validatePolygon(const QList<QGeoCoordinate> &vertices, QString &errorString);

    // ========================================================================
    // Coordinate Validation
    // ========================================================================

    /// Check if coordinate has valid lat/lon (ignores altitude)
    bool isValidCoordinate(const QGeoCoordinate &coord);

    /// Validate all coordinates in a list
    /// @param coords Coordinates to validate
    /// @param errorString Output: description of first invalid coordinate
    /// @return true if all coordinates are valid
    bool validateCoordinates(const QList<QGeoCoordinate> &coords, QString &errorString);
}
