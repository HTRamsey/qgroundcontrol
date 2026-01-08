#pragma once

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSet>
#include <QtCore/QStringList>

class QDomDocument;
class QDomElement;

Q_DECLARE_LOGGING_CATEGORY(GPXSchemaValidatorLog)

/// Validates GPX documents against rules extracted from the GPX 1.1 XSD schema.
/// This provides schema-driven validation without requiring a full XML Schema processor.
class GPXSchemaValidator
{
public:
    static GPXSchemaValidator *instance();

    struct ValidationResult {
        bool isValid = true;
        QStringList errors;
        QStringList warnings;

        void addError(const QString &msg) { errors.append(msg); isValid = false; }
        void addWarning(const QString &msg) { warnings.append(msg); }
    };

    /// Validate a GPX document
    ValidationResult validate(const QDomDocument &doc) const;

    /// Validate a GPX file
    ValidationResult validateFile(const QString &gpxFile) const;

    /// Check if a value is valid for a given XSD simple type
    bool isValidSimpleType(const QString &typeName, const QString &value) const;

    /// Check if an element name is defined in the schema
    bool isValidElement(const QString &elementName) const;

    /// Get all valid GPX element names
    QStringList validElements() const;

    /// GPX 1.1 namespace
    static constexpr const char *gpxNamespace = "http://www.topografix.com/GPX/1/1";

private:
    GPXSchemaValidator();
    void loadSchema();
    void parseSchemaDocument(const QDomDocument &schemaDoc);
    void extractSimpleTypes(const QDomElement &root);
    void extractElements(const QDomElement &root);

    void validateElement(const QDomElement &element, ValidationResult &result) const;
    void validateLatitude(double lat, const QString &context, ValidationResult &result) const;
    void validateLongitude(double lon, const QString &context, ValidationResult &result) const;

    QHash<QString, QPair<double, double>> _numericRanges;  // typeName -> (min, max)
    QHash<QString, QStringList> _enumTypes;                // typeName -> valid values
    QSet<QString> _validElements;                          // set of valid element names
    bool _loaded = false;
};
