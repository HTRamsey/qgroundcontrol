#pragma once

#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCInterpolatorLog)

/// Interpolation utilities for smoothing data and path generation.
///
/// Supports:
/// - Linear interpolation
/// - Cubic spline interpolation
/// - Bezier curves
/// - Catmull-Rom splines
///
/// Example usage:
/// @code
/// QVector<QPointF> points = {{0, 0}, {1, 2}, {3, 1}, {4, 3}};
/// QGCInterpolator interp(points);
/// double y = interp.linear(0.5);  // Interpolate at x=0.5
/// QVector<QPointF> smooth = interp.cubicSpline(100);  // 100 smooth points
/// @endcode
class QGCInterpolator : public QObject
{
    Q_OBJECT

public:
    enum class Method {
        Linear,
        CubicSpline,
        Bezier,
        CatmullRom
    };
    Q_ENUM(Method)

    explicit QGCInterpolator(QObject *parent = nullptr);
    explicit QGCInterpolator(const QVector<QPointF> &points, QObject *parent = nullptr);
    ~QGCInterpolator() override = default;

    /// Set data points (must be sorted by x)
    void setPoints(const QVector<QPointF> &points);
    QVector<QPointF> points() const { return _points; }

    /// Clear all points
    void clear();

    /// Add a single point
    void addPoint(const QPointF &point);
    void addPoint(double x, double y);

    // ========================================================================
    // Linear interpolation
    // ========================================================================

    /// Interpolate y value at x using linear interpolation
    double linear(double x) const;

    /// Linear interpolation between two values
    static double lerp(double a, double b, double t);

    /// Inverse linear interpolation (find t given value between a and b)
    static double inverseLerp(double a, double b, double value);

    /// Map value from one range to another
    static double map(double value, double inMin, double inMax, double outMin, double outMax);

    // ========================================================================
    // Cubic spline interpolation
    // ========================================================================

    /// Interpolate y value at x using cubic spline
    double cubicSpline(double x) const;

    /// Generate smooth curve with n points using cubic spline
    QVector<QPointF> cubicSplinePoints(int numPoints) const;

    // ========================================================================
    // Bezier curves
    // ========================================================================

    /// Evaluate cubic Bezier curve at t (0-1)
    static QPointF bezierCubic(const QPointF &p0, const QPointF &p1,
                               const QPointF &p2, const QPointF &p3, double t);

    /// Generate points along a cubic Bezier curve
    static QVector<QPointF> bezierCubicPoints(const QPointF &p0, const QPointF &p1,
                                               const QPointF &p2, const QPointF &p3,
                                               int numPoints);

    /// Evaluate quadratic Bezier curve at t (0-1)
    static QPointF bezierQuadratic(const QPointF &p0, const QPointF &p1,
                                    const QPointF &p2, double t);

    // ========================================================================
    // Catmull-Rom splines
    // ========================================================================

    /// Interpolate using Catmull-Rom spline
    double catmullRom(double x) const;

    /// Generate smooth curve using Catmull-Rom spline
    QVector<QPointF> catmullRomPoints(int numPoints) const;

    /// Evaluate Catmull-Rom segment
    static QPointF catmullRomSegment(const QPointF &p0, const QPointF &p1,
                                      const QPointF &p2, const QPointF &p3,
                                      double t, double tension = 0.5);

    // ========================================================================
    // Easing functions
    // ========================================================================

    /// Apply easing function to t (0-1)
    static double easeInQuad(double t);
    static double easeOutQuad(double t);
    static double easeInOutQuad(double t);

    static double easeInCubic(double t);
    static double easeOutCubic(double t);
    static double easeInOutCubic(double t);

    static double easeInSine(double t);
    static double easeOutSine(double t);
    static double easeInOutSine(double t);

    static double easeInExpo(double t);
    static double easeOutExpo(double t);
    static double easeInOutExpo(double t);

    // ========================================================================
    // Utility functions
    // ========================================================================

    /// Clamp value to range
    static double clamp(double value, double min, double max);

    /// Smooth step (Hermite interpolation)
    static double smoothStep(double edge0, double edge1, double x);

    /// Smoother step (Ken Perlin's improved version)
    static double smootherStep(double edge0, double edge1, double x);

private:
    void _computeSplineCoefficients() const;

    QVector<QPointF> _points;

    // Cached spline coefficients
    mutable bool _splineValid = false;
    mutable QVector<double> _splineA;
    mutable QVector<double> _splineB;
    mutable QVector<double> _splineC;
    mutable QVector<double> _splineD;
};
