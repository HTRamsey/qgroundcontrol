#include "GPXSchemaValidator.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtXml/QDomDocument>

QGC_LOGGING_CATEGORY(GPXSchemaValidatorLog, "Utilities.GPXSchemaValidator")

namespace {
    // Schema resource path (PREFIX "/gpx" + FILES "gpx11.xsd")
    constexpr const char *SCHEMA_RESOURCE = ":/gpx/gpx11.xsd";
}

GPXSchemaValidator *GPXSchemaValidator::instance()
{
    static GPXSchemaValidator validator;
    return &validator;
}

GPXSchemaValidator::GPXSchemaValidator()
{
    loadSchema();
}

void GPXSchemaValidator::loadSchema()
{
    QFile schemaFile(SCHEMA_RESOURCE);
    if (!schemaFile.open(QIODevice::ReadOnly)) {
        qCWarning(GPXSchemaValidatorLog) << "Failed to open GPX schema resource:" << SCHEMA_RESOURCE;
        return;
    }

    QDomDocument schemaDoc;
    const QDomDocument::ParseResult result = schemaDoc.setContent(&schemaFile);
    if (!result) {
        qCWarning(GPXSchemaValidatorLog) << "Failed to parse GPX schema:" << result.errorMessage;
        return;
    }

    parseSchemaDocument(schemaDoc);
    _loaded = true;
    qCDebug(GPXSchemaValidatorLog) << "Loaded GPX schema with" << _numericRanges.size() << "numeric types,"
                                   << _enumTypes.size() << "enum types, and"
                                   << _validElements.size() << "elements";
}

void GPXSchemaValidator::parseSchemaDocument(const QDomDocument &schemaDoc)
{
    const QDomElement root = schemaDoc.documentElement();
    extractSimpleTypes(root);
    extractElements(root);
}

void GPXSchemaValidator::extractSimpleTypes(const QDomElement &root)
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
                if (!ok) minVal = -std::numeric_limits<double>::infinity();
            }
            if (!maxEl.isNull()) {
                bool ok = false;
                maxVal = maxEl.attribute(QStringLiteral("value")).toDouble(&ok);
                if (!ok) maxVal = std::numeric_limits<double>::infinity();
            }

            _numericRanges.insert(typeName, qMakePair(minVal, maxVal));
            qCDebug(GPXSchemaValidatorLog) << "Extracted numeric type:" << typeName << "[" << minVal << "," << maxVal << "]";
        }

        // Check for enumeration values
        QStringList enumValues;
        QDomElement enumEl = restriction.firstChildElement(QStringLiteral("enumeration"));
        while (!enumEl.isNull()) {
            const QString value = enumEl.attribute(QStringLiteral("value"));
            if (!value.isEmpty()) {
                enumValues.append(value);
            }
            enumEl = enumEl.nextSiblingElement(QStringLiteral("enumeration"));
        }

        if (!enumValues.isEmpty()) {
            _enumTypes.insert(typeName, enumValues);
            qCDebug(GPXSchemaValidatorLog) << "Extracted enum type:" << typeName << "with values:" << enumValues;
        }
    }
}

void GPXSchemaValidator::extractElements(const QDomElement &root)
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

bool GPXSchemaValidator::isValidSimpleType(const QString &typeName, const QString &value) const
{
    // Check numeric range
    const auto rangeIt = _numericRanges.constFind(typeName);
    if (rangeIt != _numericRanges.constEnd()) {
        bool ok = false;
        const double numVal = value.toDouble(&ok);
        if (!ok) {
            return false;
        }
        return numVal >= rangeIt->first && numVal <= rangeIt->second;
    }

    // Check enumeration
    const auto enumIt = _enumTypes.constFind(typeName);
    if (enumIt != _enumTypes.constEnd()) {
        return enumIt->contains(value);
    }

    return true;  // Unknown type, don't reject
}

bool GPXSchemaValidator::isValidElement(const QString &elementName) const
{
    return _validElements.contains(elementName);
}

QStringList GPXSchemaValidator::validElements() const
{
    return QStringList(_validElements.begin(), _validElements.end());
}

GPXSchemaValidator::ValidationResult GPXSchemaValidator::validateFile(const QString &gpxFile) const
{
    ValidationResult result;

    QFile file(gpxFile);
    if (!file.open(QIODevice::ReadOnly)) {
        result.addError(QString("Failed to open file: %1").arg(gpxFile));
        return result;
    }

    QDomDocument doc;
    const QDomDocument::ParseResult parseResult = doc.setContent(&file);
    if (!parseResult) {
        result.addError(QString("XML parse error at line %1: %2")
                       .arg(parseResult.errorLine)
                       .arg(parseResult.errorMessage));
        return result;
    }

    return validate(doc);
}

GPXSchemaValidator::ValidationResult GPXSchemaValidator::validate(const QDomDocument &doc) const
{
    ValidationResult result;

    const QDomElement root = doc.documentElement();

    // Check root element is <gpx>
    if (root.tagName() != QStringLiteral("gpx")) {
        result.addError(QString("Root element must be 'gpx', found '%1'").arg(root.tagName()));
        return result;
    }

    // Check version attribute (required, must be "1.1")
    const QString version = root.attribute(QStringLiteral("version"));
    if (version.isEmpty()) {
        result.addError("Missing required 'version' attribute on gpx element");
    } else if (version != QStringLiteral("1.1")) {
        result.addWarning(QString("GPX version '%1' - this validator is designed for version 1.1").arg(version));
    }

    // Check creator attribute (required)
    const QString creator = root.attribute(QStringLiteral("creator"));
    if (creator.isEmpty()) {
        result.addWarning("Missing 'creator' attribute on gpx element (required by GPX 1.1 spec)");
    }

    // Check namespace
    const QString ns = root.namespaceURI();
    if (!ns.isEmpty() && ns != QString::fromLatin1(gpxNamespace)) {
        result.addWarning(QString("Expected namespace '%1', found '%2'").arg(QString::fromLatin1(gpxNamespace), ns));
    }

    // Recursively validate all elements
    validateElement(root, result);

    return result;
}

void GPXSchemaValidator::validateElement(const QDomElement &element, ValidationResult &result) const
{
    const QString tagName = element.tagName();

    // Check if element is known (skip namespace-prefixed elements)
    if (!tagName.contains(':') && !_validElements.isEmpty() && !isValidElement(tagName)) {
        result.addWarning(QString("Unknown element: '%1'").arg(tagName));
    }

    // Validate specific elements and attributes
    if (tagName == QStringLiteral("wpt") || tagName == QStringLiteral("rtept") || tagName == QStringLiteral("trkpt")) {
        // Waypoint/route point/track point - validate lat/lon attributes
        bool latOk = false, lonOk = false;
        const double lat = element.attribute(QStringLiteral("lat")).toDouble(&latOk);
        const double lon = element.attribute(QStringLiteral("lon")).toDouble(&lonOk);

        if (!latOk) {
            result.addError(QString("Missing or invalid 'lat' attribute on <%1>").arg(tagName));
        } else {
            validateLatitude(lat, tagName, result);
        }

        if (!lonOk) {
            result.addError(QString("Missing or invalid 'lon' attribute on <%1>").arg(tagName));
        } else {
            validateLongitude(lon, tagName, result);
        }
    } else if (tagName == QStringLiteral("bounds")) {
        // Bounds element - validate minlat, maxlat, minlon, maxlon
        bool ok = false;

        const double minlat = element.attribute(QStringLiteral("minlat")).toDouble(&ok);
        if (ok) validateLatitude(minlat, "bounds.minlat", result);

        const double maxlat = element.attribute(QStringLiteral("maxlat")).toDouble(&ok);
        if (ok) validateLatitude(maxlat, "bounds.maxlat", result);

        const double minlon = element.attribute(QStringLiteral("minlon")).toDouble(&ok);
        if (ok) validateLongitude(minlon, "bounds.minlon", result);

        const double maxlon = element.attribute(QStringLiteral("maxlon")).toDouble(&ok);
        if (ok) validateLongitude(maxlon, "bounds.maxlon", result);
    } else if (tagName == QStringLiteral("fix")) {
        // Fix type enumeration
        const QString value = element.text();
        if (!value.isEmpty() && !isValidSimpleType(QStringLiteral("fixType"), value)) {
            result.addError(QString("Invalid fix type: '%1'. Valid values: none, 2d, 3d, dgps, pps").arg(value));
        }
    } else if (tagName == QStringLiteral("magvar")) {
        // Magnetic variation - degrees [0, 360)
        bool ok = false;
        const double degrees = element.text().toDouble(&ok);
        if (ok && (degrees < 0.0 || degrees >= 360.0)) {
            result.addError(QString("Magnetic variation out of range [0, 360): %1").arg(degrees));
        }
    }

    // Validate children recursively
    QDomElement child = element.firstChildElement();
    while (!child.isNull()) {
        validateElement(child, result);
        child = child.nextSiblingElement();
    }
}

void GPXSchemaValidator::validateLatitude(double lat, const QString &context, ValidationResult &result) const
{
    if (lat < -90.0 || lat > 90.0) {
        result.addError(QString("Latitude out of range [-90, 90] in %1: %2").arg(context).arg(lat));
    }
}

void GPXSchemaValidator::validateLongitude(double lon, const QString &context, ValidationResult &result) const
{
    if (lon < -180.0 || lon > 180.0) {
        result.addError(QString("Longitude out of range [-180, 180] in %1: %2").arg(context).arg(lon));
    }
}
