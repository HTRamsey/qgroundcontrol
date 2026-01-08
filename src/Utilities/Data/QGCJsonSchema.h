#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QStringList>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCJsonSchemaLog)

/// Simple JSON Schema validator for common validation needs.
///
/// Supports a subset of JSON Schema Draft-07:
/// - Type validation (string, number, integer, boolean, array, object, null)
/// - Required properties
/// - Minimum/maximum for numbers
/// - MinLength/maxLength for strings
/// - Pattern matching (regex)
/// - Enum values
/// - Nested object validation
/// - Array item validation
///
/// Example usage:
/// @code
/// QJsonObject schema = {
///     {"type", "object"},
///     {"required", QJsonArray{"name", "age"}},
///     {"properties", QJsonObject{
///         {"name", QJsonObject{{"type", "string"}, {"minLength", 1}}},
///         {"age", QJsonObject{{"type", "integer"}, {"minimum", 0}}}
///     }}
/// };
/// QGCJsonSchema validator(schema);
/// auto result = validator.validate(myJsonObject);
/// if (!result.isValid()) {
///     qDebug() << "Errors:" << result.errors();
/// }
/// @endcode
class QGCJsonSchema : public QObject
{
    Q_OBJECT

public:
    /// Validation result
    class ValidationResult {
    public:
        ValidationResult() = default;

        bool isValid() const { return _errors.isEmpty(); }
        QStringList errors() const { return _errors; }
        QString errorString() const { return _errors.join(QStringLiteral("; ")); }

        void addError(const QString &path, const QString &message);
        void merge(const ValidationResult &other);

    private:
        QStringList _errors;
    };

    explicit QGCJsonSchema(QObject *parent = nullptr);
    explicit QGCJsonSchema(const QJsonObject &schema, QObject *parent = nullptr);
    ~QGCJsonSchema() override = default;

    /// Set the schema
    void setSchema(const QJsonObject &schema);
    QJsonObject schema() const { return _schema; }

    /// Validate a JSON value against the schema
    ValidationResult validate(const QJsonValue &value) const;

    /// Validate and return just success/failure
    Q_INVOKABLE bool isValid(const QJsonValue &value) const;

    /// Load schema from file
    bool loadFromFile(const QString &filePath);

    /// Load schema from JSON string
    bool loadFromString(const QString &jsonString);

    // Static validation helpers

    /// Validate a value against a schema
    static ValidationResult validateValue(const QJsonValue &value,
                                          const QJsonObject &schema,
                                          const QString &path = QStringLiteral("$"));

    /// Check if a value matches a type
    static bool matchesType(const QJsonValue &value, const QString &type);

    /// Get the JSON type name for a value
    static QString typeName(const QJsonValue &value);

private:
    static ValidationResult validateObject(const QJsonObject &obj,
                                           const QJsonObject &schema,
                                           const QString &path);

    static ValidationResult validateArray(const QJsonArray &arr,
                                          const QJsonObject &schema,
                                          const QString &path);

    static ValidationResult validateString(const QString &str,
                                           const QJsonObject &schema,
                                           const QString &path);

    static ValidationResult validateNumber(double num,
                                           const QJsonObject &schema,
                                           const QString &path);

    QJsonObject _schema;
};
