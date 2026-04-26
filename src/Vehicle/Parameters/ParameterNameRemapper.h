#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

class Vehicle;

Q_DECLARE_LOGGING_CATEGORY(ParameterNameRemapperLog)

/// Walk a parameter name backwards through the FirmwarePlugin remap tables from
/// the highest known minor version down to the vehicle's actual version. Names
/// not found in any remap table pass through unchanged.
///
/// Names prefixed with "noremap." bypass remapping entirely — the prefix is
/// stripped and the bare name is used as-is. This is needed when code must
/// distinguish old vs new parameter names for unit conversion (e.g. checking
/// whether WPNAV_SPEED exists vs WP_SPD).
namespace ParameterNameRemapper {

/// Remap @p paramName from the newest firmware version to the version running
/// on @p vehicle. Returns the original name if no remapping applies.
[[nodiscard]] QString remap(Vehicle* vehicle, const QString& paramName);

}  // namespace ParameterNameRemapper
