#include "VehicleGPSFactGroupTest.h"
#include "VehicleGPSFactGroup.h"
#include "QGCMAVLink.h"

#include <QtTest/QSignalSpy>

static mavlink_message_t makeGpsRawInt(int32_t lat, int32_t lon, int32_t alt, uint16_t eph,
                                       uint16_t epv, uint16_t vel, uint16_t cog,
                                       uint8_t fix_type, uint8_t satellites_visible)
{
    mavlink_message_t msg{};
    mavlink_gps_raw_int_t raw{};
    raw.lat = lat;
    raw.lon = lon;
    raw.alt = alt;
    raw.eph = eph;
    raw.epv = epv;
    raw.vel = vel;
    raw.cog = cog;
    raw.fix_type = fix_type;
    raw.satellites_visible = satellites_visible;
    mavlink_msg_gps_raw_int_encode(1, 1, &msg, &raw);
    return msg;
}

void VehicleGPSFactGroupTest::testQualityNone()
{
    VehicleGPSFactGroup fg;
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityNone);

    auto msg = makeGpsRawInt(0, 0, 0, 500, 500, UINT16_MAX, UINT16_MAX, 1, 4);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityNone);
}

void VehicleGPSFactGroupTest::testQualityPoor()
{
    VehicleGPSFactGroup fg;
    // 3D fix, high HDOP (5.0), few sats (5) -> Poor
    auto msg = makeGpsRawInt(470000000, -1220000000, 100000, 500, 500, 100, 1800, 3, 5);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityPoor);
}

void VehicleGPSFactGroupTest::testQualityFair()
{
    VehicleGPSFactGroup fg;
    // 3D fix, low HDOP (1.2) XOR sats: good hdop but few sats (5) -> Fair
    auto msg = makeGpsRawInt(470000000, -1220000000, 100000, 120, 120, 100, 1800, 3, 5);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityFair);
}

void VehicleGPSFactGroupTest::testQualityGood()
{
    VehicleGPSFactGroup fg;
    // 3D fix, low HDOP (1.2), many sats (14) -> Good
    auto msg = makeGpsRawInt(470000000, -1220000000, 100000, 120, 120, 100, 1800, 3, 14);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityGood);
}

void VehicleGPSFactGroupTest::testQualityExcellent()
{
    VehicleGPSFactGroup fg;
    // RTK Fixed (fix=6) -> Excellent regardless of other values
    auto msg = makeGpsRawInt(470000000, -1220000000, 100000, 80, 80, 100, 1800, 6, 20);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(fg.quality(), VehicleGPSFactGroup::GPSQuality::QualityExcellent);
}

void VehicleGPSFactGroupTest::testQualityChangedEmittedOnGpsRawInt()
{
    VehicleGPSFactGroup fg;
    QSignalSpy spy(&fg, &VehicleGPSFactGroup::qualityChanged);
    QVERIFY(spy.isValid());

    auto msg = makeGpsRawInt(470000000, -1220000000, 100000, 120, 120, 100, 1800, 3, 14);
    fg.handleMessage(nullptr, msg);
    QCOMPARE(spy.count(), 1);
}

UT_REGISTER_TEST(VehicleGPSFactGroupTest, TestLabel::Unit)
