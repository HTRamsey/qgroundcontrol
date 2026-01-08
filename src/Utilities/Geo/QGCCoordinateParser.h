#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

#include <optional>

Q_DECLARE_LOGGING_CATEGORY(QGCCoordinateParserLog)

/// Parses GPS coordinates from various text formats.
///
/// Supported formats:
/// - Decimal degrees: "37.7749, -122.4194" or "37.7749 -122.4194"
/// - DMS: "37°46'29.64\"N 122°25'9.84\"W"
/// - DMM: "37°46.494'N 122°25.164'W"
/// - MGRS: "10SEG8520057521" (requires proj library - optional)
/// - UTM: "10S 552065 4180066"
///
/// Example usage:
/// @code
/// auto coord = QGCCoordinateParser::parse("37.7749, -122.4194");
/// if (coord.isValid()) {
///     qDebug() << coord.latitude() << coord.longitude();
/// }
/// @endcode
class QGCCoordinateParser : public QObject
{
    Q_OBJECT

public:
    enum class Format {
        Unknown,
        DecimalDegrees,     // 37.7749, -122.4194
        DegreesMinutesSeconds,  // 37°46'29.64"N 122°25'9.84"W
        DegreesDecimalMinutes,  // 37°46.494'N 122°25.164'W
        UTM,                // 10S 552065 4180066
        MGRS                // 10SEG8520057521
    };
    Q_ENUM(Format)

    explicit QGCCoordinateParser(QObject *parent = nullptr);
    ~QGCCoordinateParser() override = default;

    /// Parse coordinate string, auto-detecting format
    static QGeoCoordinate parse(const QString &text);

    /// Parse with specific format hint
    static QGeoCoordinate parse(const QString &text, Format format);

    /// Detect the format of a coordinate string
    static Format detectFormat(const QString &text);

    /// Format a coordinate as string
    static QString toString(const QGeoCoordinate &coord, Format format = Format::DecimalDegrees,
                            int precision = 6);

    /// Check if a string looks like a valid coordinate
    static bool isValidCoordinateString(const QString &text);

    // Individual format parsers
    static QGeoCoordinate parseDecimalDegrees(const QString &text);
    static QGeoCoordinate parseDMS(const QString &text);
    static QGeoCoordinate parseDMM(const QString &text);
    static QGeoCoordinate parseUTM(const QString &text);

    // Individual format formatters
    static QString toDecimalDegrees(const QGeoCoordinate &coord, int precision = 6);
    static QString toDMS(const QGeoCoordinate &coord, int precision = 2);
    static QString toDMM(const QGeoCoordinate &coord, int precision = 4);

    /// Get human-readable format name
    static QString formatName(Format format);

private:
    static double parseDMSComponent(const QString &text, bool &isNegative);
    static double parseDMMComponent(const QString &text, bool &isNegative);
    static QString normalizeInput(const QString &text);
};
