#include "ParameterTextLine.h"

#include <QtCore/QStringList>

namespace ParameterTextLine {

std::optional<ParsedRow> parseLine(const QString& line)
{
    if (line.startsWith(QLatin1Char('#'))) {
        return std::nullopt;
    }

    const QStringList parts = line.split(QLatin1Char('\t'));
    if (parts.size() != 5) {
        return std::nullopt;
    }

    ParsedRow row;
    row.vehicleId = parts.at(0).toInt();
    row.componentId = parts.at(1).toInt();
    row.name = parts.at(2);
    row.rawValue = parts.at(3);
    row.type = static_cast<MAV_PARAM_TYPE>(parts.at(4).toUInt());
    return row;
}

QVariant variantFromString(const QString& valStr, MAV_PARAM_TYPE type)
{
    switch (type) {
        case MAV_PARAM_TYPE_REAL32:
            return QVariant(valStr.toFloat());
        case MAV_PARAM_TYPE_UINT32:
            return QVariant(valStr.toUInt());
        case MAV_PARAM_TYPE_UINT16:
            return QVariant(static_cast<quint16>(valStr.toUInt()));
        case MAV_PARAM_TYPE_INT16:
            return QVariant(static_cast<qint16>(valStr.toInt()));
        case MAV_PARAM_TYPE_UINT8:
            return QVariant(static_cast<quint8>(valStr.toUInt()));
        case MAV_PARAM_TYPE_INT8:
            return QVariant(static_cast<qint8>(valStr.toInt()));
        default:
            // Unknown types fall through to int32 — matches the legacy parsers.
            [[fallthrough]];
        case MAV_PARAM_TYPE_INT32:
            return QVariant(valStr.toInt());
    }
}

QString formatRow(int vehicleId, int componentId, const QString& name,
                  const QString& rawValueString, MAV_PARAM_TYPE type)
{
    return QStringLiteral("%1\t%2\t%3\t%4\t%5\n")
        .arg(vehicleId)
        .arg(componentId)
        .arg(name, rawValueString)
        .arg(static_cast<uint>(type));
}

}  // namespace ParameterTextLine
