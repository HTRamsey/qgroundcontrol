#include "GPSFormatterTest.h"
#include "GPSFormatter.h"

#include <cmath>

// ---------------------------------------------------------------------------
// formatDuration
// ---------------------------------------------------------------------------

void GPSFormatterTest::testFormatDuration()
{
    // 0 seconds
    QCOMPARE(GPSFormatter::formatDuration(0.0), QStringLiteral("0s"));

    // Sub-minute
    QCOMPARE(GPSFormatter::formatDuration(30.0), QStringLiteral("30s"));

    // 90 s → 1m 30s
    QCOMPARE(GPSFormatter::formatDuration(90.0), QStringLiteral("1m 30s"));

    // 3700 s → 1h 1m  (3600 + 60 + 40; only hours/minutes shown)
    QCOMPARE(GPSFormatter::formatDuration(3700.0), QStringLiteral("1h 1m"));

    // Exactly 1 hour boundary
    QCOMPARE(GPSFormatter::formatDuration(3600.0), QStringLiteral("1h 0m"));

    // Exactly 59 s stays in seconds form
    QCOMPARE(GPSFormatter::formatDuration(59.0), QStringLiteral("59s"));
}

// ---------------------------------------------------------------------------
// formatDataSize
// ---------------------------------------------------------------------------

void GPSFormatterTest::testFormatDataSize()
{
    // 0 bytes
    QCOMPARE(GPSFormatter::formatDataSize(0.0), QStringLiteral("0 B"));

    // 500 bytes — still bytes range
    QCOMPARE(GPSFormatter::formatDataSize(500.0), QStringLiteral("500 B"));

    // 1023 bytes — boundary: still B
    QCOMPARE(GPSFormatter::formatDataSize(1023.0), QStringLiteral("1023 B"));

    // 2048 → 2.0 KB
    QCOMPARE(GPSFormatter::formatDataSize(2048.0), QStringLiteral("2.0 KB"));

    // 1048576 → 1.0 MB
    QCOMPARE(GPSFormatter::formatDataSize(1048576.0), QStringLiteral("1.0 MB"));

    // 1024 → 1.0 KB  (exact boundary)
    QCOMPARE(GPSFormatter::formatDataSize(1024.0), QStringLiteral("1.0 KB"));
}

// ---------------------------------------------------------------------------
// formatDataRate
// ---------------------------------------------------------------------------

void GPSFormatterTest::testFormatDataRate()
{
    // 0 B/s
    QCOMPARE(GPSFormatter::formatDataRate(0.0), QStringLiteral("0 B/s"));

    // 500 B/s — stays in B/s range
    QCOMPARE(GPSFormatter::formatDataRate(500.0), QStringLiteral("500 B/s"));

    // 2048 B/s → 2.0 KB/s
    QCOMPARE(GPSFormatter::formatDataRate(2048.0), QStringLiteral("2.0 KB/s"));

    // 1023 B/s — still B/s (boundary)
    QCOMPARE(GPSFormatter::formatDataRate(1023.0), QStringLiteral("1023 B/s"));

    // 1024 B/s → 1.0 KB/s
    QCOMPARE(GPSFormatter::formatDataRate(1024.0), QStringLiteral("1.0 KB/s"));
}

// ---------------------------------------------------------------------------
// formatLatitude / formatLongitude / formatCoordinate
// ---------------------------------------------------------------------------

void GPSFormatterTest::testFormatCoordinate()
{
    // Default precision (7)
    QCOMPARE(GPSFormatter::formatLatitude(47.1234567), QStringLiteral("47.1234567"));
    QCOMPARE(GPSFormatter::formatLongitude(-122.1234567), QStringLiteral("-122.1234567"));

    // Custom precision
    QCOMPARE(GPSFormatter::formatLatitude(47.1234567, 2), QStringLiteral("47.12"));
    QCOMPARE(GPSFormatter::formatLongitude(8.0, 3), QStringLiteral("8.000"));

    // formatCoordinate combines both with default precision
    QCOMPARE(GPSFormatter::formatCoordinate(47.1234567, -122.1234567),
             QStringLiteral("47.1234567, -122.1234567"));

    // formatCoordinate with custom precision
    QCOMPARE(GPSFormatter::formatCoordinate(0.0, 0.0, 2),
             QStringLiteral("0.00, 0.00"));
}

// ---------------------------------------------------------------------------
// formatHeading
// ---------------------------------------------------------------------------

void GPSFormatterTest::testFormatHeading()
{
    // NaN → "-.--"
    QCOMPARE(GPSFormatter::formatHeading(std::numeric_limits<double>::quiet_NaN()),
             QStringLiteral("-.--"));

    // 180.5 degrees → "180.5°"
    QCOMPARE(GPSFormatter::formatHeading(180.5), QStringLiteral("180.5\u00B0"));

    // 0 degrees
    QCOMPARE(GPSFormatter::formatHeading(0.0), QStringLiteral("0.0\u00B0"));

    // 359.9 degrees
    QCOMPARE(GPSFormatter::formatHeading(359.9), QStringLiteral("359.9\u00B0"));
}

UT_REGISTER_TEST(GPSFormatterTest, TestLabel::Unit)
