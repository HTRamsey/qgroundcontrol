#include "XsdValidator.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtXml/QDomDocument>

#include <limits>

QGC_LOGGING_CATEGORY(XsdValidatorLog, "Utilities.Geo.XsdValidator")

void XsdValidator::loadSchemaFromResource(const QString &resourcePath, const QString &formatName)
{
    _formatName = formatName;

    QFile schemaFile(resourcePath);
    if (!schemaFile.open(QIODevice::ReadOnly)) {
        qCWarning(XsdValidatorLog) << "Failed to open" << formatName << "schema resource:" << resourcePath;
        return;
    }

    QDomDocument schemaDoc;
    const QDomDocument::ParseResult result = schemaDoc.setContent(&schemaFile);
    if (!result) {
        qCWarning(XsdValidatorLog) << "Failed to parse" << formatName << "schema:" << result.errorMessage;
        return;
    }

    parseSchemaDocument(schemaDoc);
    _loaded = true;
    qCDebug(XsdValidatorLog) << "Loaded" << formatName << "schema with"
                             << _numericRanges.size() << "numeric types,"
                             << _enumTypes.size() << "enum types, and"
                             << _validElements.size() << "elements";
}

void XsdValidator::parseSchemaDocument(const QDomDocument &schemaDoc)
{
    const QDomElement root = schemaDoc.documentElement();
    extractSimpleTypes(root);
    extractElements(root);
}

void XsdValidator::extractSimpleTypes(const QDomElement &root)
{
    const QDomNodeList simpleTypes = root.elementsByTagName(QStringLiteral("simpleType"));

    for (int i = 0; i < simpleTypes.count(); i++) {
        const QDomElement simpleType = simpleTypes.item(i).toElement();
        const QString typeName = simpleType.attribute(QStringLiteral("name"));
        if (typeName.isEmpty()) {
            continue;
        }

        const QDomElement restriction = simpleType.firstChildElement(QStringLiteral("restriction"));
        if (restriction.isNull()) {
            continue;
        }

        // Check for numeric range restrictions (minInclusive/maxInclusive)
        const QDomElement minEl = restriction.firstChildElement(QStringLiteral("minInclusive"));
        const QDomElement maxEl = restriction.firstChildElement(QStringLiteral("maxInclusive"));

        if (!minEl.isNull() || !maxEl.isNull()) {
            double minVal = -std::numeric_limits<double>::infinity();
            double maxVal = std::numeric_limits<double>::infinity();

            if (!minEl.isNull()) {
                bool ok = false;
                minVal = minEl.attribute(QStringLiteral("value")).toDouble(&ok);
                if (!ok) {
                    minVal = -std::numeric_limits<double>::infinity();
                }
            }
            if (!maxEl.isNull()) {
                bool ok = false;
                maxVal = maxEl.attribute(QStringLiteral("value")).toDouble(&ok);
                if (!ok) {
                    maxVal = std::numeric_limits<double>::infinity();
                }
            }

            _numericRanges.insert(typeName, qMakePair(minVal, maxVal));
            qCDebug(XsdValidatorLog) << "Extracted numeric type:" << typeName
                                     << "[" << minVal << "," << maxVal << "]";
        }

        // Check for enumeration values
        QStringList enumVals;
        QDomElement enumEl = restriction.firstChildElement(QStringLiteral("enumeration"));
        while (!enumEl.isNull()) {
            const QString value = enumEl.attribute(QStringLiteral("value"));
            if (!value.isEmpty()) {
                enumVals.append(value);
            }
            enumEl = enumEl.nextSiblingElement(QStringLiteral("enumeration"));
        }

        if (!enumVals.isEmpty()) {
            _enumTypes.insert(typeName, enumVals);
            qCDebug(XsdValidatorLog) << "Extracted enum type:" << typeName
                                     << "with values:" << enumVals;
        }
    }
}

void XsdValidator::extractElements(const QDomElement &root)
{
    const QDomNodeList elements = root.elementsByTagName(QStringLiteral("element"));

    for (int i = 0; i < elements.count(); i++) {
        const QDomElement element = elements.item(i).toElement();
        const QString name = element.attribute(QStringLiteral("name"));
        if (!name.isEmpty()) {
            _validElements.insert(name);
        }
    }
}

bool XsdValidator::isValidEnumValue(const QString &typeName, const QString &value) const
{
    const auto it = _enumTypes.constFind(typeName);
    if (it == _enumTypes.constEnd()) {
        return true;  // Unknown enum type, don't reject
    }
    return it->contains(value);
}

QStringList XsdValidator::enumValues(const QString &typeName) const
{
    return _enumTypes.value(typeName);
}

bool XsdValidator::isValidNumericValue(const QString &typeName, double value) const
{
    const auto it = _numericRanges.constFind(typeName);
    if (it == _numericRanges.constEnd()) {
        return true;  // Unknown type, don't reject
    }
    return value >= it->first && value <= it->second;
}

bool XsdValidator::numericRange(const QString &typeName, double &minVal, double &maxVal) const
{
    const auto it = _numericRanges.constFind(typeName);
    if (it == _numericRanges.constEnd()) {
        return false;
    }
    minVal = it->first;
    maxVal = it->second;
    return true;
}

bool XsdValidator::isValidElement(const QString &elementName) const
{
    return _validElements.contains(elementName);
}

QStringList XsdValidator::validElements() const
{
    return QStringList(_validElements.begin(), _validElements.end());
}

GeoValidation::ValidationResult XsdValidator::validateFile(const QString &filePath) const
{
    GeoValidation::ValidationResult result;
    result.context = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.addError(QCoreApplication::translate("XsdValidator",
            "Failed to open file: %1").arg(filePath));
        return result;
    }

    QDomDocument doc;
    const QDomDocument::ParseResult parseResult = doc.setContent(&file);
    if (!parseResult) {
        result.addError(QCoreApplication::translate("XsdValidator",
            "XML parse error at line %1: %2")
            .arg(parseResult.errorLine)
            .arg(parseResult.errorMessage));
        return result;
    }

    return validate(doc);
}

GeoValidation::ValidationResult XsdValidator::validate(const QDomDocument &doc) const
{
    GeoValidation::ValidationResult result;
    result.context = _formatName;

    const QDomElement root = doc.documentElement();
    const QString expectedRoot = expectedRootElement();

    // Check root element
    if (root.tagName() != expectedRoot) {
        result.addError(QCoreApplication::translate("XsdValidator",
            "Root element must be '%1', found '%2'").arg(expectedRoot, root.tagName()));
        return result;
    }

    // Check namespace
    const QString ns = root.namespaceURI();
    const QString expectedNs = expectedNamespace();
    if (!ns.isEmpty() && !expectedNs.isEmpty() && ns != expectedNs) {
        result.addWarning(QCoreApplication::translate("XsdValidator",
            "Expected namespace '%1', found '%2'").arg(expectedNs, ns));
    }

    // Let subclass validate root attributes
    validateRootAttributes(root, result);

    // Recursively validate all elements
    validateElement(root, result);

    return result;
}

void XsdValidator::validateRootAttributes(const QDomElement &, GeoValidation::ValidationResult &) const
{
    // Default: no additional root validation
}

void XsdValidator::validateChildren(const QDomElement &element,
                                    GeoValidation::ValidationResult &result) const
{
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        validateElement(child, result);
        child = child.nextSiblingElement();
    }
}

bool XsdValidator::isUnknownElement(const QString &tagName) const
{
    // Skip namespace-prefixed elements (extensions)
    if (tagName.contains(QLatin1Char(':'))) {
        return false;
    }
    // Only warn if we have loaded elements
    if (_validElements.isEmpty()) {
        return false;
    }
    return !_validElements.contains(tagName);
}
