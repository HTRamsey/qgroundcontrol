#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <cstddef>
#include <optional>

#include "FactMetaData.h"
#include "MAVLinkLib.h"

Q_DECLARE_LOGGING_CATEGORY(ParameterMavlinkCodecLog)

/// Pure conversions between Fact value types and MAVLink parameter wire types.
/// No vehicle, fact, or manager state — every function is a free pure mapping.
namespace ParameterMavlinkCodec {

/// Map a Fact value type to the matching MAV_PARAM_TYPE.
/// Falls back to MAV_PARAM_TYPE_INT32 for unrecognized types.
[[nodiscard]] MAV_PARAM_TYPE factTypeToMavType(FactMetaData::ValueType_t factType);

/// Map a MAV_PARAM_TYPE to the matching Fact value type.
/// Falls back to FactMetaData::valueTypeInt32 for unrecognized types.
[[nodiscard]] FactMetaData::ValueType_t mavTypeToFactType(MAV_PARAM_TYPE mavType);

/// Build the param_union for @p rawValue interpreted as @p valueType.
/// @return populated union on success, std::nullopt if the QVariant could not be coerced.
[[nodiscard]] std::optional<mavlink_param_union_t> fillUnion(FactMetaData::ValueType_t valueType,
                                                             const QVariant& rawValue);

/// Decode @p paramUnion (using its @c type field) into a QVariant.
/// @return decoded variant on success, std::nullopt on an unsupported MAV_PARAM_TYPE.
[[nodiscard]] std::optional<QVariant> unionToVariant(const mavlink_param_union_t& paramUnion);

/// True if a PARAM_VALUE/PARAM_ERROR identifier matches the requested target.
/// If @p expectedIndex >= 0, matches by index (avoids materialising the param-id string).
/// Otherwise materialises and compares the param-id field against @p expectedName.
/// @p paramIdField may not be null-terminated; @p paramIdLen is the wire field width
/// (e.g. MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN).
[[nodiscard]] bool paramIdMatches(const char* paramIdField, std::size_t paramIdLen, int paramIndexField,
                                  const QString& expectedName, int expectedIndex);

}  // namespace ParameterMavlinkCodec
