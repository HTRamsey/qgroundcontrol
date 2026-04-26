#pragma once

#include <QtCore/QList>
#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "FactMetaData.h"

Q_DECLARE_LOGGING_CATEGORY(ParameterPackFileLog)

/// Parser for ArduPilot's binary parameter file format (`@PARAM/param.pck`).
/// See https://github.com/ArduPilot/ardupilot/tree/master/libraries/AP_Filesystem.
///
/// The format begins with a 6-byte header (magic, num_params, total_params),
/// followed by variable-length records each describing one parameter (type
/// nibble + flags nibble, name lengths nibble pair, raw name bytes, value,
/// optional default value when the @c with-defaults magic is used).
namespace ParameterPackFile {

struct ParsedParam
{
    QString name;
    FactMetaData::ValueType_t type = FactMetaData::valueTypeInt32;
    QVariant value;
    QVariant defaultValue;  ///< Invalid QVariant if no default is encoded.
};

struct ParseResult
{
    bool ok = false;
    QList<ParsedParam> params;
};

/// Parse the given file. Returns @c ok=false on any I/O or format error.
/// Caller is responsible for any vehicle-side state (Fact creation, signals).
[[nodiscard]] ParseResult parse(const QString& filename);

}  // namespace ParameterPackFile
