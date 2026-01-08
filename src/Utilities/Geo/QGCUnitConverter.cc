#include "QGCUnitConverter.h"

#include <cmath>

Q_LOGGING_CATEGORY(QGCUnitConverterLog, "Utilities.QGCUnitConverter")

QGCUnitConverter::QGCUnitConverter(QObject *parent)
    : QObject(parent)
{
}

// ============================================================================
// Distance conversions
// ============================================================================

double QGCUnitConverter::metersToFeet(double meters)
{
    return meters / METERS_PER_FOOT;
}

double QGCUnitConverter::feetToMeters(double feet)
{
    return feet * METERS_PER_FOOT;
}

double QGCUnitConverter::metersToKilometers(double meters)
{
    return meters / 1000.0;
}

double QGCUnitConverter::kilometersToMeters(double km)
{
    return km * 1000.0;
}

double QGCUnitConverter::metersToMiles(double meters)
{
    return meters / METERS_PER_MILE;
}

double QGCUnitConverter::milesToMeters(double miles)
{
    return miles * METERS_PER_MILE;
}

double QGCUnitConverter::metersToNauticalMiles(double meters)
{
    return meters / METERS_PER_NAUTICAL_MILE;
}

double QGCUnitConverter::nauticalMilesToMeters(double nm)
{
    return nm * METERS_PER_NAUTICAL_MILE;
}

double QGCUnitConverter::metersToYards(double meters)
{
    return meters / METERS_PER_YARD;
}

double QGCUnitConverter::yardsToMeters(double yards)
{
    return yards * METERS_PER_YARD;
}

double QGCUnitConverter::convertDistance(double value, DistanceUnit from, DistanceUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to meters first
    double meters = value;
    switch (from) {
    case DistanceUnit::Meters:
        break;
    case DistanceUnit::Feet:
        meters = feetToMeters(value);
        break;
    case DistanceUnit::Kilometers:
        meters = kilometersToMeters(value);
        break;
    case DistanceUnit::Miles:
        meters = milesToMeters(value);
        break;
    case DistanceUnit::NauticalMiles:
        meters = nauticalMilesToMeters(value);
        break;
    case DistanceUnit::Yards:
        meters = yardsToMeters(value);
        break;
    }

    // Convert from meters to target
    switch (to) {
    case DistanceUnit::Meters:
        return meters;
    case DistanceUnit::Feet:
        return metersToFeet(meters);
    case DistanceUnit::Kilometers:
        return metersToKilometers(meters);
    case DistanceUnit::Miles:
        return metersToMiles(meters);
    case DistanceUnit::NauticalMiles:
        return metersToNauticalMiles(meters);
    case DistanceUnit::Yards:
        return metersToYards(meters);
    }

    return meters;
}

// ============================================================================
// Speed conversions
// ============================================================================

double QGCUnitConverter::mpsToKph(double mps)
{
    return mps / MPS_PER_KPH;
}

double QGCUnitConverter::kphToMps(double kph)
{
    return kph * MPS_PER_KPH;
}

double QGCUnitConverter::mpsToMph(double mps)
{
    return mps / MPS_PER_MPH;
}

double QGCUnitConverter::mphToMps(double mph)
{
    return mph * MPS_PER_MPH;
}

double QGCUnitConverter::mpsToKnots(double mps)
{
    return mps / MPS_PER_KNOT;
}

double QGCUnitConverter::knotsToMps(double knots)
{
    return knots * MPS_PER_KNOT;
}

double QGCUnitConverter::mpsToFps(double mps)
{
    return mps / METERS_PER_FOOT;
}

double QGCUnitConverter::fpsToMps(double fps)
{
    return fps * METERS_PER_FOOT;
}

double QGCUnitConverter::convertSpeed(double value, SpeedUnit from, SpeedUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to m/s first
    double mps = value;
    switch (from) {
    case SpeedUnit::MetersPerSecond:
        break;
    case SpeedUnit::KilometersPerHour:
        mps = kphToMps(value);
        break;
    case SpeedUnit::MilesPerHour:
        mps = mphToMps(value);
        break;
    case SpeedUnit::Knots:
        mps = knotsToMps(value);
        break;
    case SpeedUnit::FeetPerSecond:
        mps = fpsToMps(value);
        break;
    }

    // Convert from m/s to target
    switch (to) {
    case SpeedUnit::MetersPerSecond:
        return mps;
    case SpeedUnit::KilometersPerHour:
        return mpsToKph(mps);
    case SpeedUnit::MilesPerHour:
        return mpsToMph(mps);
    case SpeedUnit::Knots:
        return mpsToKnots(mps);
    case SpeedUnit::FeetPerSecond:
        return mpsToFps(mps);
    }

    return mps;
}

// ============================================================================
// Temperature conversions
// ============================================================================

double QGCUnitConverter::celsiusToFahrenheit(double celsius)
{
    return (celsius * 9.0 / 5.0) + 32.0;
}

double QGCUnitConverter::fahrenheitToCelsius(double fahrenheit)
{
    return (fahrenheit - 32.0) * 5.0 / 9.0;
}

double QGCUnitConverter::celsiusToKelvin(double celsius)
{
    return celsius + 273.15;
}

double QGCUnitConverter::kelvinToCelsius(double kelvin)
{
    return kelvin - 273.15;
}

double QGCUnitConverter::convertTemperature(double value, TemperatureUnit from, TemperatureUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to Celsius first
    double celsius = value;
    switch (from) {
    case TemperatureUnit::Celsius:
        break;
    case TemperatureUnit::Fahrenheit:
        celsius = fahrenheitToCelsius(value);
        break;
    case TemperatureUnit::Kelvin:
        celsius = kelvinToCelsius(value);
        break;
    }

    // Convert from Celsius to target
    switch (to) {
    case TemperatureUnit::Celsius:
        return celsius;
    case TemperatureUnit::Fahrenheit:
        return celsiusToFahrenheit(celsius);
    case TemperatureUnit::Kelvin:
        return celsiusToKelvin(celsius);
    }

    return celsius;
}

// ============================================================================
// Angle conversions
// ============================================================================

double QGCUnitConverter::degreesToRadians(double degrees)
{
    return degrees * M_PI / 180.0;
}

double QGCUnitConverter::radiansToDegrees(double radians)
{
    return radians * 180.0 / M_PI;
}

double QGCUnitConverter::degreesToMils(double degrees)
{
    return degrees * MILS_PER_DEGREE;
}

double QGCUnitConverter::milsToDegrees(double mils)
{
    return mils / MILS_PER_DEGREE;
}

double QGCUnitConverter::convertAngle(double value, AngleUnit from, AngleUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to degrees first
    double degrees = value;
    switch (from) {
    case AngleUnit::Degrees:
        break;
    case AngleUnit::Radians:
        degrees = radiansToDegrees(value);
        break;
    case AngleUnit::Mils:
        degrees = milsToDegrees(value);
        break;
    }

    // Convert from degrees to target
    switch (to) {
    case AngleUnit::Degrees:
        return degrees;
    case AngleUnit::Radians:
        return degreesToRadians(degrees);
    case AngleUnit::Mils:
        return degreesToMils(degrees);
    }

    return degrees;
}

double QGCUnitConverter::normalizeAngle360(double degrees)
{
    degrees = std::fmod(degrees, 360.0);
    if (degrees < 0.0) {
        degrees += 360.0;
    }
    return degrees;
}

double QGCUnitConverter::normalizeAngle180(double degrees)
{
    degrees = std::fmod(degrees + 180.0, 360.0);
    if (degrees < 0.0) {
        degrees += 360.0;
    }
    return degrees - 180.0;
}

// ============================================================================
// Area conversions
// ============================================================================

double QGCUnitConverter::sqMetersToSqFeet(double sqm)
{
    return sqm / (METERS_PER_FOOT * METERS_PER_FOOT);
}

double QGCUnitConverter::sqFeetToSqMeters(double sqft)
{
    return sqft * (METERS_PER_FOOT * METERS_PER_FOOT);
}

double QGCUnitConverter::sqMetersToHectares(double sqm)
{
    return sqm / SQM_PER_HECTARE;
}

double QGCUnitConverter::hectaresToSqMeters(double ha)
{
    return ha * SQM_PER_HECTARE;
}

double QGCUnitConverter::sqMetersToAcres(double sqm)
{
    return sqm / SQM_PER_ACRE;
}

double QGCUnitConverter::acresToSqMeters(double acres)
{
    return acres * SQM_PER_ACRE;
}

double QGCUnitConverter::convertArea(double value, AreaUnit from, AreaUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to sq meters first
    double sqm = value;
    switch (from) {
    case AreaUnit::SquareMeters:
        break;
    case AreaUnit::SquareFeet:
        sqm = sqFeetToSqMeters(value);
        break;
    case AreaUnit::Hectares:
        sqm = hectaresToSqMeters(value);
        break;
    case AreaUnit::Acres:
        sqm = acresToSqMeters(value);
        break;
    case AreaUnit::SquareKilometers:
        sqm = value * 1000000.0;
        break;
    case AreaUnit::SquareMiles:
        sqm = value * METERS_PER_MILE * METERS_PER_MILE;
        break;
    }

    // Convert from sq meters to target
    switch (to) {
    case AreaUnit::SquareMeters:
        return sqm;
    case AreaUnit::SquareFeet:
        return sqMetersToSqFeet(sqm);
    case AreaUnit::Hectares:
        return sqMetersToHectares(sqm);
    case AreaUnit::Acres:
        return sqMetersToAcres(sqm);
    case AreaUnit::SquareKilometers:
        return sqm / 1000000.0;
    case AreaUnit::SquareMiles:
        return sqm / (METERS_PER_MILE * METERS_PER_MILE);
    }

    return sqm;
}

// ============================================================================
// Pressure conversions
// ============================================================================

double QGCUnitConverter::pascalsToHectopascals(double pa)
{
    return pa / 100.0;
}

double QGCUnitConverter::hectopascalsToPascals(double hpa)
{
    return hpa * 100.0;
}

double QGCUnitConverter::pascalsToInchesHg(double pa)
{
    return pa / PA_PER_INHG;
}

double QGCUnitConverter::inchesHgToPascals(double inHg)
{
    return inHg * PA_PER_INHG;
}

double QGCUnitConverter::pascalsToPsi(double pa)
{
    return pa / PA_PER_PSI;
}

double QGCUnitConverter::psiToPascals(double psi)
{
    return psi * PA_PER_PSI;
}

double QGCUnitConverter::convertPressure(double value, PressureUnit from, PressureUnit to)
{
    if (from == to) {
        return value;
    }

    // Convert to Pascals first
    double pa = value;
    switch (from) {
    case PressureUnit::Pascals:
        break;
    case PressureUnit::Hectopascals:
    case PressureUnit::Millibars:
        pa = hectopascalsToPascals(value);
        break;
    case PressureUnit::InchesHg:
        pa = inchesHgToPascals(value);
        break;
    case PressureUnit::PSI:
        pa = psiToPascals(value);
        break;
    }

    // Convert from Pascals to target
    switch (to) {
    case PressureUnit::Pascals:
        return pa;
    case PressureUnit::Hectopascals:
    case PressureUnit::Millibars:
        return pascalsToHectopascals(pa);
    case PressureUnit::InchesHg:
        return pascalsToInchesHg(pa);
    case PressureUnit::PSI:
        return pascalsToPsi(pa);
    }

    return pa;
}

// ============================================================================
// Formatting helpers
// ============================================================================

QString QGCUnitConverter::formatDistance(double meters, DistanceUnit unit, int precision)
{
    const double value = convertDistance(meters, DistanceUnit::Meters, unit);
    return QStringLiteral("%1 %2").arg(value, 0, 'f', precision).arg(distanceUnitSuffix(unit));
}

QString QGCUnitConverter::formatSpeed(double mps, SpeedUnit unit, int precision)
{
    const double value = convertSpeed(mps, SpeedUnit::MetersPerSecond, unit);
    return QStringLiteral("%1 %2").arg(value, 0, 'f', precision).arg(speedUnitSuffix(unit));
}

QString QGCUnitConverter::formatTemperature(double celsius, TemperatureUnit unit, int precision)
{
    const double value = convertTemperature(celsius, TemperatureUnit::Celsius, unit);
    return QStringLiteral("%1%2").arg(value, 0, 'f', precision).arg(temperatureUnitSuffix(unit));
}

QString QGCUnitConverter::formatAngle(double degrees, AngleUnit unit, int precision)
{
    const double value = convertAngle(degrees, AngleUnit::Degrees, unit);
    return QStringLiteral("%1%2").arg(value, 0, 'f', precision).arg(angleUnitSuffix(unit));
}

QString QGCUnitConverter::formatArea(double sqMeters, AreaUnit unit, int precision)
{
    const double value = convertArea(sqMeters, AreaUnit::SquareMeters, unit);
    return QStringLiteral("%1 %2").arg(value, 0, 'f', precision).arg(areaUnitSuffix(unit));
}

QString QGCUnitConverter::distanceUnitSuffix(DistanceUnit unit)
{
    switch (unit) {
    case DistanceUnit::Meters:
        return QStringLiteral("m");
    case DistanceUnit::Feet:
        return QStringLiteral("ft");
    case DistanceUnit::Kilometers:
        return QStringLiteral("km");
    case DistanceUnit::Miles:
        return QStringLiteral("mi");
    case DistanceUnit::NauticalMiles:
        return QStringLiteral("nm");
    case DistanceUnit::Yards:
        return QStringLiteral("yd");
    }
    return {};
}

QString QGCUnitConverter::speedUnitSuffix(SpeedUnit unit)
{
    switch (unit) {
    case SpeedUnit::MetersPerSecond:
        return QStringLiteral("m/s");
    case SpeedUnit::KilometersPerHour:
        return QStringLiteral("km/h");
    case SpeedUnit::MilesPerHour:
        return QStringLiteral("mph");
    case SpeedUnit::Knots:
        return QStringLiteral("kn");
    case SpeedUnit::FeetPerSecond:
        return QStringLiteral("ft/s");
    }
    return {};
}

QString QGCUnitConverter::temperatureUnitSuffix(TemperatureUnit unit)
{
    switch (unit) {
    case TemperatureUnit::Celsius:
        return QStringLiteral("°C");
    case TemperatureUnit::Fahrenheit:
        return QStringLiteral("°F");
    case TemperatureUnit::Kelvin:
        return QStringLiteral("K");
    }
    return {};
}

QString QGCUnitConverter::angleUnitSuffix(AngleUnit unit)
{
    switch (unit) {
    case AngleUnit::Degrees:
        return QStringLiteral("°");
    case AngleUnit::Radians:
        return QStringLiteral("rad");
    case AngleUnit::Mils:
        return QStringLiteral("mil");
    }
    return {};
}

QString QGCUnitConverter::areaUnitSuffix(AreaUnit unit)
{
    switch (unit) {
    case AreaUnit::SquareMeters:
        return QStringLiteral("m²");
    case AreaUnit::SquareFeet:
        return QStringLiteral("ft²");
    case AreaUnit::Hectares:
        return QStringLiteral("ha");
    case AreaUnit::Acres:
        return QStringLiteral("ac");
    case AreaUnit::SquareKilometers:
        return QStringLiteral("km²");
    case AreaUnit::SquareMiles:
        return QStringLiteral("mi²");
    }
    return {};
}

QString QGCUnitConverter::pressureUnitSuffix(PressureUnit unit)
{
    switch (unit) {
    case PressureUnit::Pascals:
        return QStringLiteral("Pa");
    case PressureUnit::Hectopascals:
        return QStringLiteral("hPa");
    case PressureUnit::Millibars:
        return QStringLiteral("mb");
    case PressureUnit::InchesHg:
        return QStringLiteral("inHg");
    case PressureUnit::PSI:
        return QStringLiteral("psi");
    }
    return {};
}
