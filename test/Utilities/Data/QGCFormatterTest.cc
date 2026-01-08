#include "QGCFormatterTest.h"
#include "Data/QGCFormatter.h"

#include <QtTest/QTest>

void QGCFormatterTest::_testFormatDistance()
{
    // Metric
    QCOMPARE(QGCFormatter::formatDistance(100, QGCFormatter::DistanceFormat::Metric), QString("100.0 m"));
    QCOMPARE(QGCFormatter::formatDistance(1500, QGCFormatter::DistanceFormat::Metric), QString("1.5 km"));

    // Imperial
    QCOMPARE(QGCFormatter::formatDistance(1609.344, QGCFormatter::DistanceFormat::Imperial), QString("1.0 mi"));

    // Nautical
    QCOMPARE(QGCFormatter::formatDistance(1852, QGCFormatter::DistanceFormat::Nautical), QString("1.0 nm"));
}

void QGCFormatterTest::_testFormatSpeed()
{
    // Metric (km/h)
    QCOMPARE(QGCFormatter::formatSpeed(10, QGCFormatter::DistanceFormat::Metric), QString("36.0 km/h"));

    // Imperial (mph)
    QCOMPARE(QGCFormatter::formatSpeed(10, QGCFormatter::DistanceFormat::Imperial), QString("22.4 mph"));

    // Nautical (knots)
    QCOMPARE(QGCFormatter::formatSpeed(10, QGCFormatter::DistanceFormat::Nautical), QString("19.4 kn"));
}

void QGCFormatterTest::_testFormatCoordinates()
{
    // Decimal
    const QString lat = QGCFormatter::formatLatitude(47.123456, QGCFormatter::CoordinateFormat::Decimal);
    QVERIFY(lat.contains("47.123456"));
    QVERIFY(lat.contains("N"));

    const QString lon = QGCFormatter::formatLongitude(-122.123456, QGCFormatter::CoordinateFormat::Decimal);
    QVERIFY(lon.contains("122.123456"));
    QVERIFY(lon.contains("W"));

    // DMS format
    const QString dms = QGCFormatter::formatLatitude(47.5, QGCFormatter::CoordinateFormat::DMS);
    QVERIFY(dms.contains("47"));
    QVERIFY(dms.contains("30"));  // 0.5 degrees = 30 minutes
}

void QGCFormatterTest::_testFormatDuration()
{
    QCOMPARE(QGCFormatter::formatDuration(0), QString("0:00"));
    QCOMPARE(QGCFormatter::formatDuration(65), QString("1:05"));
    QCOMPARE(QGCFormatter::formatDuration(3665), QString("1:01:05"));
    QCOMPARE(QGCFormatter::formatDuration(-65), QString("-1:05"));
}

void QGCFormatterTest::_testFormatBytes()
{
    QCOMPARE(QGCFormatter::formatBytes(512), QString("512 B"));
    QCOMPARE(QGCFormatter::formatBytes(1024), QString("1.0 KB"));
    QCOMPARE(QGCFormatter::formatBytes(1024 * 1024), QString("1.0 MB"));
    QCOMPARE(QGCFormatter::formatBytes(1024LL * 1024 * 1024), QString("1.0 GB"));
}
