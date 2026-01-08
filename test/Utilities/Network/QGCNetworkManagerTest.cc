#include "QGCNetworkManagerTest.h"
#include "QGCNetworkManager.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Singleton Tests
// ============================================================================

void QGCNetworkManagerTest::_testSingleton()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();
    QVERIFY(manager != nullptr);
}

void QGCNetworkManagerTest::_testSingletonSameInstance()
{
    QGCNetworkManager *manager1 = QGCNetworkManager::instance();
    QGCNetworkManager *manager2 = QGCNetworkManager::instance();
    QCOMPARE(manager1, manager2);
}

// ============================================================================
// Configuration Tests
// ============================================================================

void QGCNetworkManagerTest::_testDefaultConfiguration()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QCOMPARE(manager->defaultTimeout(), 30000);
    QVERIFY(manager->autoRedirect());
    QCOMPARE(manager->maxRedirects(), 5);
}

void QGCNetworkManagerTest::_testSetDefaultTimeout()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    manager->setDefaultTimeout(60000);
    QCOMPARE(manager->defaultTimeout(), 60000);

    // Reset to default
    manager->setDefaultTimeout(30000);
}

void QGCNetworkManagerTest::_testSetAutoRedirect()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    manager->setAutoRedirect(false);
    QVERIFY(!manager->autoRedirect());

    manager->setAutoRedirect(true);
    QVERIFY(manager->autoRedirect());
}

void QGCNetworkManagerTest::_testSetMaxRedirects()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    manager->setMaxRedirects(10);
    QCOMPARE(manager->maxRedirects(), 10);

    // Reset to default
    manager->setMaxRedirects(5);
}

// ============================================================================
// Statistics Tests
// ============================================================================

void QGCNetworkManagerTest::_testInitialStatistics()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    // Clear first to get a known state
    manager->clearStatistics();

    QCOMPARE(manager->totalRequestCount(), 0);
    QCOMPARE(manager->totalBytesReceived(), 0);
    QCOMPARE(manager->totalBytesSent(), 0);
}

void QGCNetworkManagerTest::_testClearStatistics()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    // Make a request to generate some stats
    QNetworkRequest request(QUrl("http://localhost:99999/test"));
    QNetworkReply *reply = manager->get(request);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();

    // Clear stats
    manager->clearStatistics();

    QCOMPARE(manager->totalRequestCount(), 0);
    QCOMPARE(manager->totalBytesReceived(), 0);
    QCOMPARE(manager->totalBytesSent(), 0);
}

// ============================================================================
// Request Tests
// ============================================================================

void QGCNetworkManagerTest::_testGetRequest()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();
    manager->clearStatistics();

    QNetworkRequest request(QUrl("http://localhost:99999/get"));
    QNetworkReply *reply = manager->get(request);

    QVERIFY(reply != nullptr);
    QVERIFY(manager->totalRequestCount() >= 1);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();
}

void QGCNetworkManagerTest::_testPostRequest()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QNetworkRequest request(QUrl("http://localhost:99999/post"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = "{\"test\": true}";
    QNetworkReply *reply = manager->post(request, data);

    QVERIFY(reply != nullptr);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();
}

void QGCNetworkManagerTest::_testPutRequest()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QNetworkRequest request(QUrl("http://localhost:99999/put"));
    QByteArray data = "updated data";
    QNetworkReply *reply = manager->put(request, data);

    QVERIFY(reply != nullptr);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();
}

void QGCNetworkManagerTest::_testDeleteRequest()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QNetworkRequest request(QUrl("http://localhost:99999/delete"));
    QNetworkReply *reply = manager->deleteResource(request);

    QVERIFY(reply != nullptr);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();
}

void QGCNetworkManagerTest::_testHeadRequest()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QNetworkRequest request(QUrl("http://localhost:99999/head"));
    QNetworkReply *reply = manager->head(request);

    QVERIFY(reply != nullptr);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();
}

// ============================================================================
// Tracking Tests
// ============================================================================

void QGCNetworkManagerTest::_testActiveRequestCount()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QCOMPARE(manager->activeRequestCount(), 0);

    QNetworkRequest request(QUrl("http://localhost:99999/test"));
    QNetworkReply *reply = manager->get(request);

    // Should have at least 1 active request
    QVERIFY(manager->activeRequestCount() >= 1);

    QSignalSpy spy(reply, &QNetworkReply::finished);
    QVERIFY(spy.wait(5000));
    reply->deleteLater();

    // Process events to update count
    QCoreApplication::processEvents();

    QCOMPARE(manager->activeRequestCount(), 0);
}

void QGCNetworkManagerTest::_testRequestSignals()
{
    QGCNetworkManager *manager = QGCNetworkManager::instance();

    QSignalSpy startedSpy(manager, &QGCNetworkManager::requestStarted);
    QSignalSpy finishedSpy(manager, &QGCNetworkManager::requestFinished);
    QSignalSpy countSpy(manager, &QGCNetworkManager::activeRequestCountChanged);

    QNetworkRequest request(QUrl("http://localhost:99999/signals"));
    QNetworkReply *reply = manager->get(request);

    QCOMPARE(startedSpy.count(), 1);
    QVERIFY(countSpy.count() >= 1);

    QSignalSpy replySpy(reply, &QNetworkReply::finished);
    QVERIFY(replySpy.wait(5000));
    reply->deleteLater();

    QCOMPARE(finishedSpy.count(), 1);
}
