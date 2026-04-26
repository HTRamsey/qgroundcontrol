#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>

#include <optional>

#include "MAVLinkLib.h"

/// QGC's tab-separated `.params` line format used by ParameterFileTextIO,
/// OfflineParameterLoader, and the MockLink fixture loader. Single canonical
/// parser so the three call sites can't drift apart on whitespace / type rules.
namespace ParameterTextLine {

struct ParsedRow
{
    int vehicleId = 0;
    int componentId = 0;
    QString name;
    QString rawValue;  ///< Untyped string; pass to @ref variantFromString to coerce.
    MAV_PARAM_TYPE type = MAV_PARAM_TYPE_INT32;
};

/// Parse a single line. Returns nullopt for empty lines, comment lines (starting with @c #),
/// or malformed rows (wrong field count). Caller is responsible for reporting.
[[nodiscard]] std::optional<ParsedRow> parseLine(const QString& line);

/// Coerce a raw string value to the QVariant matching @p type. Falls back to
/// MAV_PARAM_TYPE_INT32 on unknown enum values (matches legacy callers).
[[nodiscard]] QVariant variantFromString(const QString& valStr, MAV_PARAM_TYPE type);

/// Format a single tab-separated row matching @ref parseLine's expected schema.
/// Caller supplies a pre-formatted @p rawValueString (e.g. Fact::rawValueStringFullPrecision)
/// so the column layout stays in one place — guarantees parse/write symmetry.
[[nodiscard]] QString formatRow(int vehicleId, int componentId, const QString& name,
                                const QString& rawValueString, MAV_PARAM_TYPE type);

}  // namespace ParameterTextLine
