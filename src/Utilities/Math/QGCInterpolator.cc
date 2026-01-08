#include "QGCInterpolator.h"

#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(QGCInterpolatorLog, "Utilities.QGCInterpolator")

QGCInterpolator::QGCInterpolator(QObject *parent)
    : QObject(parent)
{
}

QGCInterpolator::QGCInterpolator(const QVector<QPointF> &points, QObject *parent)
    : QObject(parent)
    , _points(points)
{
}

void QGCInterpolator::setPoints(const QVector<QPointF> &points)
{
    _points = points;
    _splineValid = false;
}

void QGCInterpolator::clear()
{
    _points.clear();
    _splineValid = false;
}

void QGCInterpolator::addPoint(const QPointF &point)
{
    _points.append(point);
    _splineValid = false;
}

void QGCInterpolator::addPoint(double x, double y)
{
    addPoint(QPointF(x, y));
}

// ============================================================================
// Linear interpolation
// ============================================================================

double QGCInterpolator::linear(double x) const
{
    if (_points.size() < 2) {
        return _points.isEmpty() ? 0.0 : _points.first().y();
    }

    // Find surrounding points
    for (int i = 0; i < _points.size() - 1; ++i) {
        if (x >= _points[i].x() && x <= _points[i + 1].x()) {
            const double t = (x - _points[i].x()) / (_points[i + 1].x() - _points[i].x());
            return lerp(_points[i].y(), _points[i + 1].y(), t);
        }
    }

    // Extrapolate
    if (x < _points.first().x()) {
        return _points.first().y();
    }
    return _points.last().y();
}

double QGCInterpolator::lerp(double a, double b, double t)
{
    return a + t * (b - a);
}

double QGCInterpolator::inverseLerp(double a, double b, double value)
{
    if (std::abs(b - a) < 1e-10) {
        return 0.0;
    }
    return (value - a) / (b - a);
}

double QGCInterpolator::map(double value, double inMin, double inMax, double outMin, double outMax)
{
    const double t = inverseLerp(inMin, inMax, value);
    return lerp(outMin, outMax, t);
}

// ============================================================================
// Cubic spline interpolation
// ============================================================================

void QGCInterpolator::_computeSplineCoefficients() const
{
    if (_splineValid || _points.size() < 2) {
        return;
    }

    const int n = _points.size() - 1;

    _splineA.resize(n + 1);
    _splineB.resize(n);
    _splineC.resize(n + 1);
    _splineD.resize(n);

    for (int i = 0; i <= n; ++i) {
        _splineA[i] = _points[i].y();
    }

    QVector<double> h(n);
    for (int i = 0; i < n; ++i) {
        h[i] = _points[i + 1].x() - _points[i].x();
    }

    QVector<double> alpha(n);
    for (int i = 1; i < n; ++i) {
        alpha[i] = (3.0 / h[i]) * (_splineA[i + 1] - _splineA[i])
                 - (3.0 / h[i - 1]) * (_splineA[i] - _splineA[i - 1]);
    }

    QVector<double> l(n + 1);
    QVector<double> mu(n + 1);
    QVector<double> z(n + 1);

    l[0] = 1.0;
    mu[0] = 0.0;
    z[0] = 0.0;

    for (int i = 1; i < n; ++i) {
        l[i] = 2.0 * (_points[i + 1].x() - _points[i - 1].x()) - h[i - 1] * mu[i - 1];
        mu[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i - 1] * z[i - 1]) / l[i];
    }

    l[n] = 1.0;
    z[n] = 0.0;
    _splineC[n] = 0.0;

    for (int j = n - 1; j >= 0; --j) {
        _splineC[j] = z[j] - mu[j] * _splineC[j + 1];
        _splineB[j] = (_splineA[j + 1] - _splineA[j]) / h[j]
                    - h[j] * (_splineC[j + 1] + 2.0 * _splineC[j]) / 3.0;
        _splineD[j] = (_splineC[j + 1] - _splineC[j]) / (3.0 * h[j]);
    }

    _splineValid = true;
}

double QGCInterpolator::cubicSpline(double x) const
{
    if (_points.size() < 2) {
        return _points.isEmpty() ? 0.0 : _points.first().y();
    }

    _computeSplineCoefficients();

    // Find segment
    int i = 0;
    for (int j = 0; j < _points.size() - 1; ++j) {
        if (x >= _points[j].x() && x <= _points[j + 1].x()) {
            i = j;
            break;
        }
        if (x > _points[j + 1].x()) {
            i = j;
        }
    }

    const double dx = x - _points[i].x();
    return _splineA[i] + _splineB[i] * dx + _splineC[i] * dx * dx + _splineD[i] * dx * dx * dx;
}

QVector<QPointF> QGCInterpolator::cubicSplinePoints(int numPoints) const
{
    QVector<QPointF> result;
    if (_points.size() < 2 || numPoints < 2) {
        return result;
    }

    result.reserve(numPoints);

    const double xMin = _points.first().x();
    const double xMax = _points.last().x();
    const double step = (xMax - xMin) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        const double x = xMin + i * step;
        result.append(QPointF(x, cubicSpline(x)));
    }

    return result;
}

// ============================================================================
// Bezier curves
// ============================================================================

QPointF QGCInterpolator::bezierCubic(const QPointF &p0, const QPointF &p1,
                                      const QPointF &p2, const QPointF &p3, double t)
{
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double mt = 1.0 - t;
    const double mt2 = mt * mt;
    const double mt3 = mt2 * mt;

    return p0 * mt3 + p1 * 3.0 * mt2 * t + p2 * 3.0 * mt * t2 + p3 * t3;
}

QVector<QPointF> QGCInterpolator::bezierCubicPoints(const QPointF &p0, const QPointF &p1,
                                                     const QPointF &p2, const QPointF &p3,
                                                     int numPoints)
{
    QVector<QPointF> result;
    result.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const double t = static_cast<double>(i) / (numPoints - 1);
        result.append(bezierCubic(p0, p1, p2, p3, t));
    }

    return result;
}

QPointF QGCInterpolator::bezierQuadratic(const QPointF &p0, const QPointF &p1,
                                          const QPointF &p2, double t)
{
    const double mt = 1.0 - t;
    return p0 * mt * mt + p1 * 2.0 * mt * t + p2 * t * t;
}

// ============================================================================
// Catmull-Rom splines
// ============================================================================

QPointF QGCInterpolator::catmullRomSegment(const QPointF &p0, const QPointF &p1,
                                            const QPointF &p2, const QPointF &p3,
                                            double t, double tension)
{
    const double t2 = t * t;
    const double t3 = t2 * t;

    const double s = (1.0 - tension) / 2.0;

    const double b1 = s * ((-t3 + 2.0 * t2 - t));
    const double b2 = s * ((-t3 + t2)) + (2.0 * t3 - 3.0 * t2 + 1.0);
    const double b3 = s * ((t3 - 2.0 * t2 + t)) + (-2.0 * t3 + 3.0 * t2);
    const double b4 = s * ((t3 - t2));

    return p0 * b1 + p1 * b2 + p2 * b3 + p3 * b4;
}

double QGCInterpolator::catmullRom(double x) const
{
    if (_points.size() < 4) {
        return linear(x);
    }

    // Find segment
    int i = 1;
    for (int j = 1; j < _points.size() - 2; ++j) {
        if (x >= _points[j].x() && x <= _points[j + 1].x()) {
            i = j;
            break;
        }
    }

    const double segmentLength = _points[i + 1].x() - _points[i].x();
    const double t = (x - _points[i].x()) / segmentLength;

    const QPointF result = catmullRomSegment(_points[i - 1], _points[i],
                                              _points[i + 1], _points[i + 2], t);
    return result.y();
}

QVector<QPointF> QGCInterpolator::catmullRomPoints(int numPoints) const
{
    QVector<QPointF> result;
    if (_points.size() < 4 || numPoints < 2) {
        return result;
    }

    result.reserve(numPoints);

    const double xMin = _points[1].x();
    const double xMax = _points[_points.size() - 2].x();
    const double step = (xMax - xMin) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        const double x = xMin + i * step;
        result.append(QPointF(x, catmullRom(x)));
    }

    return result;
}

// ============================================================================
// Easing functions
// ============================================================================

double QGCInterpolator::easeInQuad(double t)
{
    return t * t;
}

double QGCInterpolator::easeOutQuad(double t)
{
    return t * (2.0 - t);
}

double QGCInterpolator::easeInOutQuad(double t)
{
    return t < 0.5 ? 2.0 * t * t : -1.0 + (4.0 - 2.0 * t) * t;
}

double QGCInterpolator::easeInCubic(double t)
{
    return t * t * t;
}

double QGCInterpolator::easeOutCubic(double t)
{
    const double t1 = t - 1.0;
    return t1 * t1 * t1 + 1.0;
}

double QGCInterpolator::easeInOutCubic(double t)
{
    return t < 0.5 ? 4.0 * t * t * t : (t - 1.0) * (2.0 * t - 2.0) * (2.0 * t - 2.0) + 1.0;
}

double QGCInterpolator::easeInSine(double t)
{
    return 1.0 - std::cos(t * M_PI / 2.0);
}

double QGCInterpolator::easeOutSine(double t)
{
    return std::sin(t * M_PI / 2.0);
}

double QGCInterpolator::easeInOutSine(double t)
{
    return -0.5 * (std::cos(M_PI * t) - 1.0);
}

double QGCInterpolator::easeInExpo(double t)
{
    return t == 0.0 ? 0.0 : std::pow(2.0, 10.0 * (t - 1.0));
}

double QGCInterpolator::easeOutExpo(double t)
{
    return t == 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
}

double QGCInterpolator::easeInOutExpo(double t)
{
    if (t == 0.0) return 0.0;
    if (t == 1.0) return 1.0;
    if (t < 0.5) return 0.5 * std::pow(2.0, 20.0 * t - 10.0);
    return 1.0 - 0.5 * std::pow(2.0, -20.0 * t + 10.0);
}

// ============================================================================
// Utility functions
// ============================================================================

double QGCInterpolator::clamp(double value, double min, double max)
{
    return std::max(min, std::min(max, value));
}

double QGCInterpolator::smoothStep(double edge0, double edge1, double x)
{
    const double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double QGCInterpolator::smootherStep(double edge0, double edge1, double x)
{
    const double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
