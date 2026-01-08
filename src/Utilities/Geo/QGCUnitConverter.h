#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCUnitConverterLog)

/// Unit conversion utilities for GCS applications.
///
/// Provides static methods for converting between:
/// - Distance units (meters, feet, kilometers, miles, nautical miles)
/// - Speed units (m/s, km/h, mph, knots, ft/s)
/// - Altitude units (meters, feet)
/// - Temperature units (Celsius, Fahrenheit, Kelvin)
/// - Angle units (degrees, radians, mils)
/// - Area units (sq meters, hectares, acres, sq feet, sq miles)
///
/// Example usage:
/// @code
/// double feet = QGCUnitConverter::metersToFeet(100.0);
/// double knots = QGCUnitConverter::mpsToKnots(15.0);
/// QString formatted = QGCUnitConverter::formatDistance(1500, QGCUnitConverter::DistanceUnit::Meters);
/// @endcode
class QGCUnitConverter : public QObject
{
    Q_OBJECT

public:
    enum class DistanceUnit {
        Meters,
        Feet,
        Kilometers,
        Miles,
        NauticalMiles,
        Yards
    };
    Q_ENUM(DistanceUnit)

    enum class SpeedUnit {
        MetersPerSecond,
        KilometersPerHour,
        MilesPerHour,
        Knots,
        FeetPerSecond
    };
    Q_ENUM(SpeedUnit)

    enum class TemperatureUnit {
        Celsius,
        Fahrenheit,
        Kelvin
    };
    Q_ENUM(TemperatureUnit)

    enum class AngleUnit {
        Degrees,
        Radians,
        Mils        // NATO mils (6400 per circle)
    };
    Q_ENUM(AngleUnit)

    enum class AreaUnit {
        SquareMeters,
        SquareFeet,
        Hectares,
        Acres,
        SquareKilometers,
        SquareMiles
    };
    Q_ENUM(AreaUnit)

    enum class PressureUnit {
        Pascals,
        Hectopascals,
        Millibars,
        InchesHg,
        PSI
    };
    Q_ENUM(PressureUnit)

    explicit QGCUnitConverter(QObject *parent = nullptr);
    ~QGCUnitConverter() override = default;

    // ========================================================================
    // Distance conversions
    // ========================================================================
    static double metersToFeet(double meters);
    static double feetToMeters(double feet);
    static double metersToKilometers(double meters);
    static double kilometersToMeters(double km);
    static double metersToMiles(double meters);
    static double milesToMeters(double miles);
    static double metersToNauticalMiles(double meters);
    static double nauticalMilesToMeters(double nm);
    static double metersToYards(double meters);
    static double yardsToMeters(double yards);

    /// Generic distance conversion
    static double convertDistance(double value, DistanceUnit from, DistanceUnit to);

    // ========================================================================
    // Speed conversions
    // ========================================================================
    static double mpsToKph(double mps);
    static double kphToMps(double kph);
    static double mpsToMph(double mps);
    static double mphToMps(double mph);
    static double mpsToKnots(double mps);
    static double knotsToMps(double knots);
    static double mpsToFps(double mps);
    static double fpsToMps(double fps);

    /// Generic speed conversion
    static double convertSpeed(double value, SpeedUnit from, SpeedUnit to);

    // ========================================================================
    // Temperature conversions
    // ========================================================================
    static double celsiusToFahrenheit(double celsius);
    static double fahrenheitToCelsius(double fahrenheit);
    static double celsiusToKelvin(double celsius);
    static double kelvinToCelsius(double kelvin);

    /// Generic temperature conversion
    static double convertTemperature(double value, TemperatureUnit from, TemperatureUnit to);

    // ========================================================================
    // Angle conversions
    // ========================================================================
    static double degreesToRadians(double degrees);
    static double radiansToDegrees(double radians);
    static double degreesToMils(double degrees);
    static double milsToDegrees(double mils);

    /// Generic angle conversion
    static double convertAngle(double value, AngleUnit from, AngleUnit to);

    /// Normalize angle to 0-360 range
    static double normalizeAngle360(double degrees);

    /// Normalize angle to -180 to +180 range
    static double normalizeAngle180(double degrees);

    // ========================================================================
    // Area conversions
    // ========================================================================
    static double sqMetersToSqFeet(double sqm);
    static double sqFeetToSqMeters(double sqft);
    static double sqMetersToHectares(double sqm);
    static double hectaresToSqMeters(double ha);
    static double sqMetersToAcres(double sqm);
    static double acresToSqMeters(double acres);

    /// Generic area conversion
    static double convertArea(double value, AreaUnit from, AreaUnit to);

    // ========================================================================
    // Pressure conversions
    // ========================================================================
    static double pascalsToHectopascals(double pa);
    static double hectopascalsToPascals(double hpa);
    static double pascalsToInchesHg(double pa);
    static double inchesHgToPascals(double inHg);
    static double pascalsToPsi(double pa);
    static double psiToPascals(double psi);

    /// Generic pressure conversion
    static double convertPressure(double value, PressureUnit from, PressureUnit to);

    // ========================================================================
    // Formatting helpers
    // ========================================================================

    /// Format distance with appropriate unit suffix
    static QString formatDistance(double meters, DistanceUnit unit, int precision = 1);

    /// Format speed with appropriate unit suffix
    static QString formatSpeed(double mps, SpeedUnit unit, int precision = 1);

    /// Format temperature with appropriate unit suffix
    static QString formatTemperature(double celsius, TemperatureUnit unit, int precision = 1);

    /// Format angle with appropriate unit suffix
    static QString formatAngle(double degrees, AngleUnit unit, int precision = 1);

    /// Format area with appropriate unit suffix
    static QString formatArea(double sqMeters, AreaUnit unit, int precision = 1);

    /// Get unit suffix string
    static QString distanceUnitSuffix(DistanceUnit unit);
    static QString speedUnitSuffix(SpeedUnit unit);
    static QString temperatureUnitSuffix(TemperatureUnit unit);
    static QString angleUnitSuffix(AngleUnit unit);
    static QString areaUnitSuffix(AreaUnit unit);
    static QString pressureUnitSuffix(PressureUnit unit);

    // ========================================================================
    // Constants
    // ========================================================================
    static constexpr double METERS_PER_FOOT = 0.3048;
    static constexpr double METERS_PER_MILE = 1609.344;
    static constexpr double METERS_PER_NAUTICAL_MILE = 1852.0;
    static constexpr double METERS_PER_YARD = 0.9144;
    static constexpr double MPS_PER_KNOT = 0.514444;
    static constexpr double MPS_PER_KPH = 0.277778;
    static constexpr double MPS_PER_MPH = 0.44704;
    static constexpr double MILS_PER_DEGREE = 17.777778;  // 6400 / 360
    static constexpr double SQM_PER_HECTARE = 10000.0;
    static constexpr double SQM_PER_ACRE = 4046.8564224;
    static constexpr double PA_PER_INHG = 3386.389;
    static constexpr double PA_PER_PSI = 6894.757;
};
