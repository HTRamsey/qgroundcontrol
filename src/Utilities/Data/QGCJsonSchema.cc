#include "QGCJsonSchema.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QRegularExpression>

#include <cmath>

Q_LOGGING_CATEGORY(QGCJsonSchemaLog, "Utilities.QGCJsonSchema")

// ============================================================================
// ValidationResult
// ============================================================================

void QGCJsonSchema::ValidationResult::addError(const QString &path, const QString &message)
{
    _errors.append(QStringLiteral("%1: %2").arg(path, message));
}

void QGCJsonSchema::ValidationResult::merge(const ValidationResult &other)
{
    _errors.append(other._errors);
}

// ============================================================================
// QGCJsonSchema
// ============================================================================

QGCJsonSchema::QGCJsonSchema(QObject *parent)
    : QObject(parent)
{
}

QGCJsonSchema::QGCJsonSchema(const QJsonObject &schema, QObject *parent)
    : QObject(parent)
    , _schema(schema)
{
}

void QGCJsonSchema::setSchema(const QJsonObject &schema)
{
    _schema = schema;
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validate(const QJsonValue &value) const
{
    return validateValue(value, _schema, QStringLiteral("$"));
}

bool QGCJsonSchema::isValid(const QJsonValue &value) const
{
    return validate(value).isValid();
}

bool QGCJsonSchema::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(QGCJsonSchemaLog) << "Failed to open schema file:" << filePath;
        return false;
    }

    return loadFromString(QString::fromUtf8(file.readAll()));
}

bool QGCJsonSchema::loadFromString(const QString &jsonString)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        qCWarning(QGCJsonSchemaLog) << "Failed to parse schema:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qCWarning(QGCJsonSchemaLog) << "Schema must be a JSON object";
        return false;
    }

    _schema = doc.object();
    return true;
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validateValue(const QJsonValue &value,
                                                             const QJsonObject &schema,
                                                             const QString &path)
{
    ValidationResult result;

    if (schema.isEmpty()) {
        return result;  // Empty schema matches everything
    }

    // Type validation
    if (schema.contains(QStringLiteral("type"))) {
        const QJsonValue typeVal = schema.value(QStringLiteral("type"));

        if (typeVal.isString()) {
            const QString type = typeVal.toString();
            if (!matchesType(value, type)) {
                result.addError(path, QStringLiteral("Expected type '%1', got '%2'")
                                .arg(type, typeName(value)));
                return result;
            }
        } else if (typeVal.isArray()) {
            // Multiple allowed types
            bool matched = false;
            const QJsonArray types = typeVal.toArray();
            for (const QJsonValue &t : types) {
                if (matchesType(value, t.toString())) {
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                QStringList typeNames;
                for (const QJsonValue &t : types) {
                    typeNames.append(t.toString());
                }
                result.addError(path, QStringLiteral("Expected one of types [%1], got '%2'")
                                .arg(typeNames.join(QStringLiteral(", ")), typeName(value)));
                return result;
            }
        }
    }

    // Const validation
    if (schema.contains(QStringLiteral("const"))) {
        if (value != schema.value(QStringLiteral("const"))) {
            result.addError(path, QStringLiteral("Value does not match const"));
        }
    }

    // Enum validation
    if (schema.contains(QStringLiteral("enum"))) {
        const QJsonArray enumValues = schema.value(QStringLiteral("enum")).toArray();
        bool found = false;
        for (const QJsonValue &enumVal : enumValues) {
            if (value == enumVal) {
                found = true;
                break;
            }
        }
        if (!found) {
            result.addError(path, QStringLiteral("Value is not in enum"));
        }
    }

    // Type-specific validation
    if (value.isObject()) {
        result.merge(validateObject(value.toObject(), schema, path));
    } else if (value.isArray()) {
        result.merge(validateArray(value.toArray(), schema, path));
    } else if (value.isString()) {
        result.merge(validateString(value.toString(), schema, path));
    } else if (value.isDouble()) {
        result.merge(validateNumber(value.toDouble(), schema, path));
    }

    return result;
}

bool QGCJsonSchema::matchesType(const QJsonValue &value, const QString &type)
{
    if (type == QStringLiteral("null")) {
        return value.isNull();
    }
    if (type == QStringLiteral("boolean")) {
        return value.isBool();
    }
    if (type == QStringLiteral("string")) {
        return value.isString();
    }
    if (type == QStringLiteral("number")) {
        return value.isDouble();
    }
    if (type == QStringLiteral("integer")) {
        if (!value.isDouble()) {
            return false;
        }
        const double num = value.toDouble();
        return std::floor(num) == num;
    }
    if (type == QStringLiteral("array")) {
        return value.isArray();
    }
    if (type == QStringLiteral("object")) {
        return value.isObject();
    }
    return false;
}

QString QGCJsonSchema::typeName(const QJsonValue &value)
{
    if (value.isNull()) {
        return QStringLiteral("null");
    }
    if (value.isBool()) {
        return QStringLiteral("boolean");
    }
    if (value.isString()) {
        return QStringLiteral("string");
    }
    if (value.isDouble()) {
        return QStringLiteral("number");
    }
    if (value.isArray()) {
        return QStringLiteral("array");
    }
    if (value.isObject()) {
        return QStringLiteral("object");
    }
    return QStringLiteral("undefined");
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validateObject(const QJsonObject &obj,
                                                              const QJsonObject &schema,
                                                              const QString &path)
{
    ValidationResult result;

    // Required properties
    if (schema.contains(QStringLiteral("required"))) {
        const QJsonArray required = schema.value(QStringLiteral("required")).toArray();
        for (const QJsonValue &reqVal : required) {
            const QString reqKey = reqVal.toString();
            if (!obj.contains(reqKey)) {
                result.addError(path, QStringLiteral("Missing required property '%1'").arg(reqKey));
            }
        }
    }

    // Properties validation
    if (schema.contains(QStringLiteral("properties"))) {
        const QJsonObject properties = schema.value(QStringLiteral("properties")).toObject();
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            const QString key = it.key();
            if (obj.contains(key)) {
                const QString propPath = QStringLiteral("%1.%2").arg(path, key);
                result.merge(validateValue(obj.value(key), it.value().toObject(), propPath));
            }
        }
    }

    // Additional properties
    if (schema.contains(QStringLiteral("additionalProperties"))) {
        const QJsonValue addPropVal = schema.value(QStringLiteral("additionalProperties"));

        if (addPropVal.isBool() && !addPropVal.toBool()) {
            // No additional properties allowed
            const QJsonObject properties = schema.value(QStringLiteral("properties")).toObject();
            for (const QString &key : obj.keys()) {
                if (!properties.contains(key)) {
                    result.addError(path, QStringLiteral("Additional property '%1' not allowed").arg(key));
                }
            }
        } else if (addPropVal.isObject()) {
            // Validate additional properties against schema
            const QJsonObject addPropSchema = addPropVal.toObject();
            const QJsonObject properties = schema.value(QStringLiteral("properties")).toObject();
            for (const QString &key : obj.keys()) {
                if (!properties.contains(key)) {
                    const QString propPath = QStringLiteral("%1.%2").arg(path, key);
                    result.merge(validateValue(obj.value(key), addPropSchema, propPath));
                }
            }
        }
    }

    // Min/max properties
    if (schema.contains(QStringLiteral("minProperties"))) {
        const int minProps = schema.value(QStringLiteral("minProperties")).toInt();
        if (obj.size() < minProps) {
            result.addError(path, QStringLiteral("Object has %1 properties, minimum is %2")
                            .arg(obj.size()).arg(minProps));
        }
    }

    if (schema.contains(QStringLiteral("maxProperties"))) {
        const int maxProps = schema.value(QStringLiteral("maxProperties")).toInt();
        if (obj.size() > maxProps) {
            result.addError(path, QStringLiteral("Object has %1 properties, maximum is %2")
                            .arg(obj.size()).arg(maxProps));
        }
    }

    return result;
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validateArray(const QJsonArray &arr,
                                                             const QJsonObject &schema,
                                                             const QString &path)
{
    ValidationResult result;

    // Min/max items
    if (schema.contains(QStringLiteral("minItems"))) {
        const int minItems = schema.value(QStringLiteral("minItems")).toInt();
        if (arr.size() < minItems) {
            result.addError(path, QStringLiteral("Array has %1 items, minimum is %2")
                            .arg(arr.size()).arg(minItems));
        }
    }

    if (schema.contains(QStringLiteral("maxItems"))) {
        const int maxItems = schema.value(QStringLiteral("maxItems")).toInt();
        if (arr.size() > maxItems) {
            result.addError(path, QStringLiteral("Array has %1 items, maximum is %2")
                            .arg(arr.size()).arg(maxItems));
        }
    }

    // Items validation
    if (schema.contains(QStringLiteral("items"))) {
        const QJsonValue itemsVal = schema.value(QStringLiteral("items"));

        if (itemsVal.isObject()) {
            // All items must match schema
            const QJsonObject itemSchema = itemsVal.toObject();
            for (int i = 0; i < arr.size(); ++i) {
                const QString itemPath = QStringLiteral("%1[%2]").arg(path).arg(i);
                result.merge(validateValue(arr.at(i), itemSchema, itemPath));
            }
        } else if (itemsVal.isArray()) {
            // Tuple validation
            const QJsonArray itemSchemas = itemsVal.toArray();
            for (int i = 0; i < std::min(arr.size(), static_cast<qsizetype>(itemSchemas.size())); ++i) {
                const QString itemPath = QStringLiteral("%1[%2]").arg(path).arg(i);
                result.merge(validateValue(arr.at(i), itemSchemas.at(i).toObject(), itemPath));
            }
        }
    }

    // Unique items
    if (schema.contains(QStringLiteral("uniqueItems")) &&
        schema.value(QStringLiteral("uniqueItems")).toBool()) {
        for (int i = 0; i < arr.size(); ++i) {
            for (int j = i + 1; j < arr.size(); ++j) {
                if (arr.at(i) == arr.at(j)) {
                    result.addError(path, QStringLiteral("Array items at index %1 and %2 are not unique")
                                    .arg(i).arg(j));
                }
            }
        }
    }

    return result;
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validateString(const QString &str,
                                                              const QJsonObject &schema,
                                                              const QString &path)
{
    ValidationResult result;

    // Min/max length
    if (schema.contains(QStringLiteral("minLength"))) {
        const int minLen = schema.value(QStringLiteral("minLength")).toInt();
        if (str.length() < minLen) {
            result.addError(path, QStringLiteral("String length %1 is less than minimum %2")
                            .arg(str.length()).arg(minLen));
        }
    }

    if (schema.contains(QStringLiteral("maxLength"))) {
        const int maxLen = schema.value(QStringLiteral("maxLength")).toInt();
        if (str.length() > maxLen) {
            result.addError(path, QStringLiteral("String length %1 exceeds maximum %2")
                            .arg(str.length()).arg(maxLen));
        }
    }

    // Pattern
    if (schema.contains(QStringLiteral("pattern"))) {
        const QString pattern = schema.value(QStringLiteral("pattern")).toString();
        const QRegularExpression regex(pattern);
        if (!regex.match(str).hasMatch()) {
            result.addError(path, QStringLiteral("String does not match pattern '%1'").arg(pattern));
        }
    }

    // Format (basic support)
    if (schema.contains(QStringLiteral("format"))) {
        const QString format = schema.value(QStringLiteral("format")).toString();

        if (format == QStringLiteral("email")) {
            static const QRegularExpression emailRegex(
                QStringLiteral(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"));
            if (!emailRegex.match(str).hasMatch()) {
                result.addError(path, QStringLiteral("String is not a valid email"));
            }
        } else if (format == QStringLiteral("uri") || format == QStringLiteral("url")) {
            static const QRegularExpression uriRegex(
                QStringLiteral(R"(^[a-zA-Z][a-zA-Z0-9+.-]*:.+)"));
            if (!uriRegex.match(str).hasMatch()) {
                result.addError(path, QStringLiteral("String is not a valid URI"));
            }
        } else if (format == QStringLiteral("date-time")) {
            // ISO 8601 datetime
            static const QRegularExpression dtRegex(
                QStringLiteral(R"(^\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}(\.\d+)?(Z|[+-]\d{2}:\d{2})?$)"));
            if (!dtRegex.match(str).hasMatch()) {
                result.addError(path, QStringLiteral("String is not a valid ISO 8601 datetime"));
            }
        } else if (format == QStringLiteral("date")) {
            static const QRegularExpression dateRegex(QStringLiteral(R"(^\d{4}-\d{2}-\d{2}$)"));
            if (!dateRegex.match(str).hasMatch()) {
                result.addError(path, QStringLiteral("String is not a valid date"));
            }
        }
    }

    return result;
}

QGCJsonSchema::ValidationResult QGCJsonSchema::validateNumber(double num,
                                                              const QJsonObject &schema,
                                                              const QString &path)
{
    ValidationResult result;

    // Minimum
    if (schema.contains(QStringLiteral("minimum"))) {
        const double minimum = schema.value(QStringLiteral("minimum")).toDouble();
        if (num < minimum) {
            result.addError(path, QStringLiteral("Number %1 is less than minimum %2")
                            .arg(num).arg(minimum));
        }
    }

    // Exclusive minimum
    if (schema.contains(QStringLiteral("exclusiveMinimum"))) {
        const double exMin = schema.value(QStringLiteral("exclusiveMinimum")).toDouble();
        if (num <= exMin) {
            result.addError(path, QStringLiteral("Number %1 is not greater than %2")
                            .arg(num).arg(exMin));
        }
    }

    // Maximum
    if (schema.contains(QStringLiteral("maximum"))) {
        const double maximum = schema.value(QStringLiteral("maximum")).toDouble();
        if (num > maximum) {
            result.addError(path, QStringLiteral("Number %1 exceeds maximum %2")
                            .arg(num).arg(maximum));
        }
    }

    // Exclusive maximum
    if (schema.contains(QStringLiteral("exclusiveMaximum"))) {
        const double exMax = schema.value(QStringLiteral("exclusiveMaximum")).toDouble();
        if (num >= exMax) {
            result.addError(path, QStringLiteral("Number %1 is not less than %2")
                            .arg(num).arg(exMax));
        }
    }

    // Multiple of
    if (schema.contains(QStringLiteral("multipleOf"))) {
        const double multiple = schema.value(QStringLiteral("multipleOf")).toDouble();
        if (multiple != 0.0) {
            const double remainder = std::fmod(num, multiple);
            if (std::abs(remainder) > 1e-10 && std::abs(remainder - multiple) > 1e-10) {
                result.addError(path, QStringLiteral("Number %1 is not a multiple of %2")
                                .arg(num).arg(multiple));
            }
        }
    }

    return result;
}
