#include "GeoValidation.h"

#include <QtCore/QCoreApplication>

#include <cmath>

namespace GeoValidation
{

void validateLatitude(double lat, const QString &context, ValidationResult &result)
{
    if (std::isnan(lat)) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Invalid latitude (NaN) in %1").arg(context));
        return;
    }

    if (lat < -90.0 || lat > 90.0) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Latitude out of range [-90, 90]: %1 in %2").arg(lat).arg(context));
    }
}

void validateLongitude(double lon, const QString &context, ValidationResult &result)
{
    if (std::isnan(lon)) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Invalid longitude (NaN) in %1").arg(context));
        return;
    }

    if (lon < -180.0 || lon > 180.0) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Longitude out of range [-180, 180]: %1 in %2").arg(lon).arg(context));
    }
}

void validateAltitude(double alt, const QString &context, ValidationResult &result,
                     double minAlt, double maxAlt)
{
    if (std::isnan(alt)) {
        // NaN altitude is often valid (means "no altitude specified")
        return;
    }

    if (alt < minAlt || alt > maxAlt) {
        result.addWarning(QCoreApplication::translate("GeoValidation",
            "Altitude %1 outside typical range [%2, %3] in %4")
            .arg(alt).arg(minAlt).arg(maxAlt).arg(context));
    }
}

void validateKmlCoordinateString(const QString &coordString, const QString &context,
                                 ValidationResult &result)
{
    if (coordString.trimmed().isEmpty()) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Empty coordinates string in %1").arg(context));
        return;
    }

    // KML format: lon,lat[,alt] with space-separated coordinates
    const QStringList coordGroups = coordString.simplified().split(QLatin1Char(' '), Qt::SkipEmptyParts);

    for (int i = 0; i < coordGroups.size(); ++i) {
        const QString &group = coordGroups[i];
        const QStringList values = group.split(QLatin1Char(','));

        if (values.size() < 2) {
            result.addError(QCoreApplication::translate("GeoValidation",
                "Invalid coordinate format (expected lon,lat[,alt]): '%1' in %2")
                .arg(group, context));
            continue;
        }

        bool lonOk = false, latOk = false;
        const double lon = values[0].toDouble(&lonOk);
        const double lat = values[1].toDouble(&latOk);

        if (!lonOk || !latOk) {
            result.addError(QCoreApplication::translate("GeoValidation",
                "Failed to parse coordinate values: '%1' in %2")
                .arg(group, context));
            continue;
        }

        const QString coordContext = QCoreApplication::translate("GeoValidation",
            "coordinate %1 of %2").arg(i + 1).arg(context);
        validateLatitude(lat, coordContext, result);
        validateLongitude(lon, coordContext, result);

        if (values.size() >= 3) {
            bool altOk = false;
            const double alt = values[2].toDouble(&altOk);
            if (altOk) {
                validateAltitude(alt, coordContext, result);
            }
        }
    }
}

void validateWktCoordinateString(const QString &coordString, const QString &context,
                                ValidationResult &result)
{
    if (coordString.trimmed().isEmpty()) {
        result.addError(QCoreApplication::translate("GeoValidation",
            "Empty coordinates string in %1").arg(context));
        return;
    }

    // WKT format: x y [z], comma-separated coordinates
    const QStringList coordGroups = coordString.split(QLatin1Char(','), Qt::SkipEmptyParts);

    for (int i = 0; i < coordGroups.size(); ++i) {
        const QString group = coordGroups[i].trimmed();
        const QStringList values = group.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);

        if (values.size() < 2) {
            result.addError(QCoreApplication::translate("GeoValidation",
                "Invalid coordinate format (expected x y [z]): '%1' in %2")
                .arg(group, context));
            continue;
        }

        bool xOk = false, yOk = false;
        const double x = values[0].toDouble(&xOk);  // longitude
        const double y = values[1].toDouble(&yOk);  // latitude

        if (!xOk || !yOk) {
            result.addError(QCoreApplication::translate("GeoValidation",
                "Failed to parse coordinate values: '%1' in %2")
                .arg(group, context));
            continue;
        }

        const QString coordContext = QCoreApplication::translate("GeoValidation",
            "coordinate %1 of %2").arg(i + 1).arg(context);
        validateLatitude(y, coordContext, result);
        validateLongitude(x, coordContext, result);

        if (values.size() >= 3) {
            bool zOk = false;
            const double z = values[2].toDouble(&zOk);
            if (zOk) {
                validateAltitude(z, coordContext, result);
            }
        }
    }
}

} // namespace GeoValidation
