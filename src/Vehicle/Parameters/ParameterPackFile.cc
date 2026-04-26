#include "ParameterPackFile.h"

#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <cstring>

#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(ParameterPackFileLog, "Vehicle.Parameters.ParameterPackFile")

namespace {

constexpr quint16 kMagicStandard = 0x671B;
constexpr quint16 kMagicWithDefaults = 0x671C;

// AP_Param value-type tag stored in the low nibble of each record's first byte.
enum ApVarType : quint8
{
    AP_PARAM_NONE = 0,
    AP_PARAM_INT8 = 1,
    AP_PARAM_INT16 = 2,
    AP_PARAM_INT32 = 3,
    AP_PARAM_FLOAT = 4,
    AP_PARAM_VECTOR3F = 5,
    AP_PARAM_GROUP = 6,
};

FactMetaData::ValueType_t toFactType(quint8 ptype)
{
    switch (ptype) {
        case AP_PARAM_INT8:
            return FactMetaData::valueTypeInt8;
        case AP_PARAM_INT16:
            return FactMetaData::valueTypeInt16;
        case AP_PARAM_INT32:
            return FactMetaData::valueTypeInt32;
        case AP_PARAM_FLOAT:
            return FactMetaData::valueTypeFloat;
        default:
            return FactMetaData::valueTypeFloat;
    }
}

bool readScalar(QDataStream& in, quint8 ptype, QVariant& outValue)
{
    qint8 data8 = 0;
    qint16 data16 = 0;
    qint32 data32 = 0;

    switch (ptype) {
        case AP_PARAM_INT8:
            in >> data8;
            outValue = data8;
            break;
        case AP_PARAM_INT16:
            in >> data16;
            outValue = data16;
            break;
        case AP_PARAM_INT32:
            in >> data32;
            outValue = data32;
            break;
        case AP_PARAM_FLOAT: {
            in >> data32;
            float f = 0.0f;
            std::memcpy(&f, &data32, sizeof(f));
            outValue = f;
            break;
        }
        default:
            qCDebug(ParameterPackFileLog) << "Type out of range" << ptype;
            return false;
    }
    return in.status() == QDataStream::Ok;
}

}  // namespace

namespace ParameterPackFile {

ParseResult parse(const QString& filename)
{
    ParseResult result;

    qCDebug(ParameterPackFileLog) << "parse:" << filename;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(ParameterPackFileLog) << "Could not open downloaded parameter file.";
        return result;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    quint16 magic = 0;
    quint16 num_params = 0;
    quint16 total_params = 0;
    in >> magic >> num_params >> total_params;
    if (in.status() != QDataStream::Ok) {
        qCDebug(ParameterPackFileLog) << "Could not read header";
        return result;
    }

    qCDebug(ParameterPackFileLog) << "magic: 0x" << Qt::hex << magic << "num_params:" << num_params
                                  << "total_params:" << total_params;

    if ((magic != kMagicStandard) && (magic != kMagicWithDefaults)) {
        qCDebug(ParameterPackFileLog) << "File does not start with magic";
        return result;
    }
    // We always request all parameters, so a partial file is treated as malformed.
    if (num_params != total_params) {
        qCDebug(ParameterPackFileLog) << "num_params != total_params";
        return result;
    }

    const bool fileHasDefaults = (magic == kMagicWithDefaults);
    QList<ParsedParam> params;
    params.reserve(num_params);

    // AP_Param packs each record's name as a (common_len, name_len) pair where
    // the first common_len bytes are inherited from the previous record. The
    // buffer therefore must persist across iterations.
    char name_buffer[17] = {};

    while (in.status() == QDataStream::Ok) {
        quint8 byte = 0;

        // Eat padding bytes until the next record (or EOF).
        while (byte == 0x0) {
            in >> byte;
            if (in.status() != QDataStream::Ok) {
                if (params.size() == num_params) {
                    result.ok = true;
                    result.params = std::move(params);
                    return result;
                }
                qCDebug(ParameterPackFileLog)
                    << "Unexpected EOF — expected:" << num_params << "actual:" << params.size();
                return result;
            }
        }

        const quint8 ptype = byte & 0x0F;
        const quint8 flags = (byte >> 4) & 0x0F;
        const bool withDefault = (flags & 0x01) == 0x01;

        in >> byte;
        if (in.status() != QDataStream::Ok) {
            qCWarning(ParameterPackFileLog) << "Unexpected EOF reading flags";
            return result;
        }

        const quint8 name_len = ((byte >> 4) & 0x0F) + 1;
        const quint8 common_len = byte & 0x0F;
        if ((name_len + common_len) > 16) {
            qCWarning(ParameterPackFileLog) << "common_len + name_len > 16"
                                            << "name_len" << name_len << "common_len" << common_len;
            return result;
        }

        const int read = in.readRawData(&name_buffer[common_len], static_cast<int>(name_len));
        if (read != name_len) {
            qCWarning(ParameterPackFileLog)
                << "Unexpected EOF reading name — expected:" << name_len << "actual:" << read;
            return result;
        }
        name_buffer[common_len + name_len] = '\0';

        ParsedParam p;
        p.name = QString(name_buffer);
        p.type = toFactType(ptype);

        if (!readScalar(in, ptype, p.value)) {
            return result;
        }
        if (withDefault && fileHasDefaults) {
            if (!readScalar(in, ptype, p.defaultValue)) {
                return result;
            }
        }

        qCDebug(ParameterPackFileLog) << "param" << p.name << "ptype" << ptype << "flags" << flags << "value"
                                      << p.value;

        params.append(std::move(p));
        if (params.size() > num_params) {
            qCDebug(ParameterPackFileLog)
                << "More parameters than expected — expected:" << num_params << "actual:" << params.size();
            return result;
        }
    }

    return result;
}

}  // namespace ParameterPackFile
