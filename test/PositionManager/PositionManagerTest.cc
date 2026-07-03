#include "PositionManagerTest.h"

#include <QtCore/QRegularExpression>
#include <QtNetwork/QUdpSocket>
#include <QtPositioning/QNmeaPositionInfoSource>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "AutoConnectSettings.h"
#include "PositionManager.h"
#include "SettingsManager.h"
#include "UdpIODevice.h"

namespace {

constexpr uint16_t kTestUdpPort = 40129;

// Canonical NMEA sentences with valid checksums.
const QByteArray kNmeaSentences =
    "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n"
    "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";

AutoConnectSettings* autoConnectSettings()
{
    return SettingsManager::instance()->autoConnectSettings();
}

} // namespace

void PositionManagerTest::cleanup()
{
    // The NMEA source settings are process-global; reset so ordering can't
    // leak an open device into the next case.
    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("Disabled"));
    UnitTest::cleanup();
}

void PositionManagerTest::testPollDisabledLeavesDeviceClosed()
{
    QGCPositionManager manager;
    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("Disabled"));

    manager._pollNmeaDevice();

    QVERIFY(!manager._nmeaUdpSocket || (manager._nmeaUdpSocket->state() != UdpIODevice::BoundState));
    QVERIFY(!manager._nmeaSource);
}

void PositionManagerTest::testUdpNmeaOpenBindsPort()
{
    QGCPositionManager manager;
    autoConnectSettings()->nmeaUdpPort()->setRawValue(kTestUdpPort);
    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("UDP Port"));

    manager._pollNmeaDevice();

    QVERIFY(manager._nmeaUdpSocket);
    QCOMPARE(manager._nmeaUdpSocket->state(), UdpIODevice::BoundState);
    QCOMPARE(manager._nmeaUdpSocket->localPort(), kTestUdpPort);
    QVERIFY(manager._nmeaSource);
    QCOMPARE(manager._currentSource, manager._nmeaSource);

    // Polling again with unchanged settings must not rebind.
    UdpIODevice* const socket = manager._nmeaUdpSocket;
    manager._pollNmeaDevice();
    QCOMPARE(manager._nmeaUdpSocket, socket);
    QCOMPARE(manager._nmeaUdpSocket->state(), UdpIODevice::BoundState);
}

void PositionManagerTest::testUdpNmeaPositionUpdate()
{
    QGCPositionManager manager;
    autoConnectSettings()->nmeaUdpPort()->setRawValue(kTestUdpPort);
    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("UDP Port"));

    // QNmeaPositionInfoSource reports update timeouts while waiting for the
    // first sentence; that is expected here, not a failure.
    ignoreLogMessage("PositionManager.QGCPositionManager", QtWarningMsg,
                     QRegularExpression(QStringLiteral("Positioning error")));

    manager._pollNmeaDevice();
    QVERIFY(manager._nmeaUdpSocket);
    QCOMPARE(manager._nmeaUdpSocket->state(), UdpIODevice::BoundState);

    QSignalSpy positionSpy(&manager, &QGCPositionManager::positionInfoUpdated);
    QVERIFY(positionSpy.isValid());

    QUdpSocket sender;
    QCOMPARE(sender.writeDatagram(kNmeaSentences, QHostAddress::LocalHost, kTestUdpPort),
             static_cast<qint64>(kNmeaSentences.size()));

    QTRY_VERIFY_WITH_TIMEOUT(!positionSpy.isEmpty(), 5000);
    const auto update = positionSpy.last().at(0).value<QGeoPositionInfo>();
    QVERIFY(update.isValid());
    QCOMPARE_LT(qAbs(update.coordinate().latitude() - 48.1173), 0.001);
    QCOMPARE_LT(qAbs(update.coordinate().longitude() - 11.5167), 0.001);
}

void PositionManagerTest::testDisableAfterOpenClosesPort()
{
    QGCPositionManager manager;
    autoConnectSettings()->nmeaUdpPort()->setRawValue(kTestUdpPort);
    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("UDP Port"));

    manager._pollNmeaDevice();
    QVERIFY(manager._nmeaUdpSocket);
    QCOMPARE(manager._nmeaUdpSocket->state(), UdpIODevice::BoundState);

    autoConnectSettings()->autoConnectNmeaPort()->setRawValue(QStringLiteral("Disabled"));
    manager._pollNmeaDevice();

    QVERIFY(manager._nmeaUdpSocket->state() != UdpIODevice::BoundState);
}

UT_REGISTER_TEST(PositionManagerTest, TestLabel::Unit)
