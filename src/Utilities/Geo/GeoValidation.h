#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace GeoValidation
{

/// Unified validation result for all geo operations
/// Consolidates KMLSchemaValidator::ValidationResult, GPXSchemaValidator::ValidationResult,
/// and GeoFormatRegistry::ValidationResult into a single type.
struct ValidationResult {
    QString context;           ///< Format name, file path, or operation context
    bool isValid = true;       ///< Overall validity (false if any errors)
    QStringList errors;        ///< Critical issues that invalidate the data
    QStringList warnings;      ///< Non-critical issues (data still usable)

    /// Add an error and mark result as invalid
    void addError(const QString &msg)
    {
        errors.append(msg);
        isValid = false;
    }

    /// Add a warning (doesn't affect validity)
    void addWarning(const QString &msg)
    {
        warnings.append(msg);
    }

    /// Merge another result into this one
    /// Errors and warnings are combined, isValid becomes false if either is invalid
    void merge(const ValidationResult &other)
    {
        errors.append(other.errors);
        warnings.append(other.warnings);
        if (!other.isValid) {
            isValid = false;
        }
        if (context.isEmpty() && !other.context.isEmpty()) {
            context = other.context;
        }
    }

    /// Check if there are any issues (errors or warnings)
    bool hasIssues() const
    {
        return !errors.isEmpty() || !warnings.isEmpty();
    }

    /// Get total issue count
    int issueCount() const
    {
        return errors.size() + warnings.size();
    }

    /// Get summary string for logging/display
    QString summary() const
    {
        if (!hasIssues()) {
            return QStringLiteral("Valid");
        }

        QStringList parts;
        if (!errors.isEmpty()) {
            parts.append(QStringLiteral("%1 error(s)").arg(errors.size()));
        }
        if (!warnings.isEmpty()) {
            parts.append(QStringLiteral("%1 warning(s)").arg(warnings.size()));
        }
        return parts.join(QStringLiteral(", "));
    }

    /// Get all issues as a single list (errors first, then warnings)
    QStringList allIssues() const
    {
        QStringList result = errors;
        result.append(warnings);
        return result;
    }

    /// Clear all errors and warnings, reset to valid state
    void clear()
    {
        context.clear();
        isValid = true;
        errors.clear();
        warnings.clear();
    }
};

// ============================================================================
// Common Validation Functions
// ============================================================================

/// Validate latitude value is in range [-90, 90]
/// @param lat Latitude value to check
/// @param context Description for error messages (e.g., "coordinate 5")
/// @param result Validation result to add errors/warnings to
void validateLatitude(double lat, const QString &context, ValidationResult &result);

/// Validate longitude value is in range [-180, 180]
/// @param lon Longitude value to check
/// @param context Description for error messages (e.g., "coordinate 5")
/// @param result Validation result to add errors/warnings to
void validateLongitude(double lon, const QString &context, ValidationResult &result);

/// Validate altitude value is within reasonable bounds
/// @param alt Altitude value to check
/// @param context Description for error messages
/// @param result Validation result to add errors/warnings to
/// @param minAlt Minimum valid altitude (default -500m, below sea level)
/// @param maxAlt Maximum valid altitude (default 100000m, ~330,000 ft)
void validateAltitude(double alt, const QString &context, ValidationResult &result,
                     double minAlt = -500.0, double maxAlt = 100000.0);

/// Validate a coordinate string in KML format (lon,lat[,alt])
/// @param coordString Coordinate string to validate
/// @param context Description for error messages
/// @param result Validation result to add errors/warnings to
void validateKmlCoordinateString(const QString &coordString, const QString &context,
                                 ValidationResult &result);

/// Validate a coordinate string in WKT format (x y [z])
/// @param coordString Coordinate string to validate
/// @param context Description for error messages
/// @param result Validation result to add errors/warnings to
void validateWktCoordinateString(const QString &coordString, const QString &context,
                                ValidationResult &result);

} // namespace GeoValidation
