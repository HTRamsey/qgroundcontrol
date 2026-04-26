#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

class QTextStream;
class ParameterManager;

Q_DECLARE_LOGGING_CATEGORY(ParameterFileTextIOLog)

/// QGC's tab-separated `.params` text format: one parameter per line as
/// @code vehicleId\tcomponentId\tname\tvalue\tmavType@endcode, preceded by
/// `#`-prefixed comment lines describing the source vehicle.
namespace ParameterFileTextIO {

/// Apply parameters from @p stream onto @p manager.
/// Lines that do not match a known parameter (or have a type mismatch) are
/// skipped and reported in the returned error string. An empty return string
/// indicates a fully successful load.
[[nodiscard]] QString read(QTextStream& stream, ParameterManager* manager);

/// Serialize every parameter currently held by @p manager to @p stream.
void write(QTextStream& stream, ParameterManager* manager);

}  // namespace ParameterFileTextIO
