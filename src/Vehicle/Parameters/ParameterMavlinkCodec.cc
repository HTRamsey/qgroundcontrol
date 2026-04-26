#include "ParameterMavlinkCodec.h"

#include <QtCore/QLatin1StringView>

#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ParameterMavlinkCodecLog, "Vehicle.Parameters.ParameterMavlinkCodec")

namespace ParameterMavlinkCodec {

MAV_PARAM_TYPE factTypeToMavType(FactMetaData::ValueType_t factType)
{
    switch (factType) {
        case FactMetaData::valueTypeUint8:
            return MAV_PARAM_TYPE_UINT8;
        case FactMetaData::valueTypeInt8:
            return MAV_PARAM_TYPE_INT8;
        case FactMetaData::valueTypeUint16:
            return MAV_PARAM_TYPE_UINT16;
        case FactMetaData::valueTypeInt16:
            return MAV_PARAM_TYPE_INT16;
        case FactMetaData::valueTypeUint32:
            return MAV_PARAM_TYPE_UINT32;
        case FactMetaData::valueTypeUint64:
            return MAV_PARAM_TYPE_UINT64;
        case FactMetaData::valueTypeInt64:
            return MAV_PARAM_TYPE_INT64;
        case FactMetaData::valueTypeFloat:
            return MAV_PARAM_TYPE_REAL32;
        case FactMetaData::valueTypeDouble:
            return MAV_PARAM_TYPE_REAL64;
        default:
            qCWarning(ParameterMavlinkCodecLog) << "Unsupported fact type" << factType;
            [[fallthrough]];
        case FactMetaData::valueTypeInt32:
            return MAV_PARAM_TYPE_INT32;
    }
}

FactMetaData::ValueType_t mavTypeToFactType(MAV_PARAM_TYPE mavType)
{
    switch (mavType) {
        case MAV_PARAM_TYPE_UINT8:
            return FactMetaData::valueTypeUint8;
        case MAV_PARAM_TYPE_INT8:
            return FactMetaData::valueTypeInt8;
        case MAV_PARAM_TYPE_UINT16:
            return FactMetaData::valueTypeUint16;
        case MAV_PARAM_TYPE_INT16:
            return FactMetaData::valueTypeInt16;
        case MAV_PARAM_TYPE_UINT32:
            return FactMetaData::valueTypeUint32;
        case MAV_PARAM_TYPE_UINT64:
            return FactMetaData::valueTypeUint64;
        case MAV_PARAM_TYPE_INT64:
            return FactMetaData::valueTypeInt64;
        case MAV_PARAM_TYPE_REAL32:
            return FactMetaData::valueTypeFloat;
        case MAV_PARAM_TYPE_REAL64:
            return FactMetaData::valueTypeDouble;
        default:
            qCWarning(ParameterMavlinkCodecLog) << "Unsupported mav param type" << mavType;
            [[fallthrough]];
        case MAV_PARAM_TYPE_INT32:
            return FactMetaData::valueTypeInt32;
    }
}

std::optional<mavlink_param_union_t> fillUnion(FactMetaData::ValueType_t valueType, const QVariant& rawValue)
{
    mavlink_param_union_t paramUnion{};
    bool ok = false;
    switch (valueType) {
        case FactMetaData::valueTypeUint8:
            paramUnion.param_uint8 = static_cast<uint8_t>(rawValue.toUInt(&ok));
            break;
        case FactMetaData::valueTypeInt8:
            paramUnion.param_int8 = static_cast<int8_t>(rawValue.toInt(&ok));
            break;
        case FactMetaData::valueTypeUint16:
            paramUnion.param_uint16 = static_cast<uint16_t>(rawValue.toUInt(&ok));
            break;
        case FactMetaData::valueTypeInt16:
            paramUnion.param_int16 = static_cast<int16_t>(rawValue.toInt(&ok));
            break;
        case FactMetaData::valueTypeUint32:
            paramUnion.param_uint32 = static_cast<uint32_t>(rawValue.toUInt(&ok));
            break;
        case FactMetaData::valueTypeFloat:
            paramUnion.param_float = rawValue.toFloat(&ok);
            break;
        case FactMetaData::valueTypeInt32:
            paramUnion.param_int32 = static_cast<int32_t>(rawValue.toInt(&ok));
            break;
        default:
            qCCritical(ParameterMavlinkCodecLog) << "Internal Error: Unsupported fact value type" << valueType;
            paramUnion.param_int32 = static_cast<int32_t>(rawValue.toInt(&ok));
            break;
    }

    if (!ok) {
        qCCritical(ParameterMavlinkCodecLog) << "Fact failed to convert to param type:" << valueType;
        return std::nullopt;
    }
    return paramUnion;
}

std::optional<QVariant> unionToVariant(const mavlink_param_union_t& paramUnion)
{
    switch (paramUnion.type) {
        case MAV_PARAM_TYPE_REAL32:
            return QVariant(paramUnion.param_float);
        case MAV_PARAM_TYPE_UINT8:
            return QVariant(paramUnion.param_uint8);
        case MAV_PARAM_TYPE_INT8:
            return QVariant(paramUnion.param_int8);
        case MAV_PARAM_TYPE_UINT16:
            return QVariant(paramUnion.param_uint16);
        case MAV_PARAM_TYPE_INT16:
            return QVariant(paramUnion.param_int16);
        case MAV_PARAM_TYPE_UINT32:
            return QVariant(paramUnion.param_uint32);
        case MAV_PARAM_TYPE_INT32:
            return QVariant(paramUnion.param_int32);
        default:
            qCCritical(ParameterMavlinkCodecLog) << "Unsupported MAV_PARAM_TYPE" << paramUnion.type;
            return std::nullopt;
    }
}

bool paramIdMatches(const char* paramIdField, std::size_t paramIdLen, int paramIndexField, const QString& expectedName,
                    int expectedIndex)
{
    if (expectedIndex >= 0) {
        return paramIndexField == expectedIndex;
    }
    // Wire field is fixed-width and may not be null-terminated; bound the read.
    // QLatin1StringView avoids allocating a QString per PARAM_VALUE on the predicate hot path.
    return expectedName == QLatin1StringView(paramIdField, static_cast<qsizetype>(qstrnlen(paramIdField, paramIdLen)));
}

}  // namespace ParameterMavlinkCodec
