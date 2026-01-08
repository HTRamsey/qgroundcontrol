#include "QGCJsonRequestTest.h"
#include "QGCJsonRequest.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Construction and State Tests
// ============================================================================

void QGCJsonRequestTest::_testConstruction()
{
    QGCJsonRequest api(this);
    QVERIFY(!api.isRunning());
}

void QGCJsonRequestTest::_testInitialState()
{
    QGCJsonRequest api(this);

    QVERIFY(!api.isRunning());
    QVERIFY(api.errorString().isEmpty());
    QCOMPARE(api.httpStatus(), 0);
    QVERIFY(api.bearerToken().isEmpty());
    QVERIFY(api.baseUrl().isEmpty());
    QCOMPARE(api.timeout(), 30000);  // Default 30 seconds
    QVERIFY(api.responseBody().isEmpty());
    QVERIFY(api.responseDocument().isNull());
}

// ============================================================================
// Property Tests
// ============================================================================

void QGCJsonRequestTest::_testSetBearerToken()
{
    QGCJsonRequest api(this);

    QSignalSpy spy(&api, &QGCJsonRequest::bearerTokenChanged);

    api.setBearerToken("test-token-123");
    QCOMPARE(api.bearerToken(), QStringLiteral("test-token-123"));
    QCOMPARE(spy.count(), 1);

    // Setting same token should not emit signal
    api.setBearerToken("test-token-123");
    QCOMPARE(spy.count(), 1);

    // Clear token
    api.setBearerToken("");
    QVERIFY(api.bearerToken().isEmpty());
    QCOMPARE(spy.count(), 2);
}

void QGCJsonRequestTest::_testSetBaseUrl()
{
    QGCJsonRequest api(this);

    QSignalSpy spy(&api, &QGCJsonRequest::baseUrlChanged);

    api.setBaseUrl("https://api.example.com");
    QCOMPARE(api.baseUrl(), QStringLiteral("https://api.example.com"));
    QCOMPARE(spy.count(), 1);

    // Setting same URL should not emit signal
    api.setBaseUrl("https://api.example.com");
    QCOMPARE(spy.count(), 1);
}

void QGCJsonRequestTest::_testSetTimeout()
{
    QGCJsonRequest api(this);

    QSignalSpy spy(&api, &QGCJsonRequest::timeoutChanged);

    api.setTimeout(60000);  // 60 seconds
    QCOMPARE(api.timeout(), 60000);
    QCOMPARE(spy.count(), 1);

    // Setting same timeout should not emit signal
    api.setTimeout(60000);
    QCOMPARE(spy.count(), 1);
}

// ============================================================================
// Header Tests
// ============================================================================

void QGCJsonRequestTest::_testSetHeader()
{
    QGCJsonRequest api(this);

    // Set custom headers
    api.setHeader("X-Custom-Header", "value1");
    api.setHeader("X-Another-Header", "value2");

    // Replace existing header
    api.setHeader("X-Custom-Header", "new-value");

    // No crash or assertion
}

void QGCJsonRequestTest::_testRemoveHeader()
{
    QGCJsonRequest api(this);

    api.setHeader("X-Custom-Header", "value");
    api.removeHeader("X-Custom-Header");

    // Remove non-existent header should not crash
    api.removeHeader("Non-Existent");
}

void QGCJsonRequestTest::_testClearHeaders()
{
    QGCJsonRequest api(this);

    api.setHeader("Header1", "value1");
    api.setHeader("Header2", "value2");

    api.clearHeaders();

    // Clear again should not crash
    api.clearHeaders();
}

// ============================================================================
// URL Resolution Tests
// ============================================================================

void QGCJsonRequestTest::_testResolveAbsoluteUrl()
{
    QGCJsonRequest api(this);
    api.setBaseUrl("https://base.example.com/api");

    // Absolute URL should be used as-is (test by starting request)
    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    // This will fail (no server) but will test URL resolution
    QVERIFY(api.get("https://other.example.com/data"));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testResolveRelativeUrl()
{
    QGCJsonRequest api(this);
    api.setBaseUrl("https://api.example.com");

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    // Relative URL should be resolved against base
    QVERIFY(api.get("/users/123"));
    QVERIFY(spy.wait(5000));
}

// ============================================================================
// Request Method Tests
// ============================================================================

void QGCJsonRequestTest::_testGetBasic()
{
    QGCJsonRequest api(this);

    QSignalSpy finishedSpy(&api, &QGCJsonRequest::finished);
    QSignalSpy runningSpy(&api, &QGCJsonRequest::runningChanged);

    QVERIFY(api.get("http://localhost:99999/test"));
    QVERIFY(api.isRunning());
    QVERIFY(runningSpy.count() >= 1);

    // Wait for failure (no server)
    QVERIFY(finishedSpy.wait(5000));
    QVERIFY(!api.isRunning());
}

void QGCJsonRequestTest::_testGetWithParams()
{
    QGCJsonRequest api(this);

    QVariantMap params;
    params["key1"] = "value1";
    params["key2"] = "value2";

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.get("http://localhost:99999/search", params));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testPostEmpty()
{
    QGCJsonRequest api(this);

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.post("http://localhost:99999/create"));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testPostObject()
{
    QGCJsonRequest api(this);

    QJsonObject body;
    body["name"] = "Test";
    body["value"] = 42;

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.post("http://localhost:99999/create", body));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testPostArray()
{
    QGCJsonRequest api(this);

    QJsonArray body;
    body.append("item1");
    body.append("item2");

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.post("http://localhost:99999/batch", body));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testPut()
{
    QGCJsonRequest api(this);

    QJsonObject body;
    body["updated"] = true;

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.put("http://localhost:99999/update", body));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testPatch()
{
    QGCJsonRequest api(this);

    QJsonObject body;
    body["field"] = "newValue";

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.patch("http://localhost:99999/partial", body));
    QVERIFY(spy.wait(5000));
}

void QGCJsonRequestTest::_testDelete()
{
    QGCJsonRequest api(this);

    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.deleteResource("http://localhost:99999/item/123"));
    QVERIFY(spy.wait(5000));
}

// ============================================================================
// Concurrent Request Tests
// ============================================================================

void QGCJsonRequestTest::_testConcurrentRequestRejected()
{
    QGCJsonRequest api(this);

    // Start first request
    QVERIFY(api.get("http://localhost:99999/slow"));

    // Second request should be rejected
    QVERIFY(!api.get("http://localhost:99999/other"));
    QVERIFY(!api.post("http://localhost:99999/other"));
    QVERIFY(!api.put("http://localhost:99999/other"));
    QVERIFY(!api.patch("http://localhost:99999/other"));
    QVERIFY(!api.deleteResource("http://localhost:99999/other"));

    // Wait for first request
    QSignalSpy spy(&api, &QGCJsonRequest::finished);
    QVERIFY(spy.wait(5000));
}

// ============================================================================
// Cancel Tests
// ============================================================================

void QGCJsonRequestTest::_testCancel()
{
    QGCJsonRequest api(this);

    // Create spy BEFORE starting request so we don't miss the signal
    QSignalSpy spy(&api, &QGCJsonRequest::finished);

    QVERIFY(api.get("http://localhost:99999/test"));
    QVERIFY(api.isRunning());

    api.cancel();

    // The finished signal may be emitted synchronously or async
    if (spy.count() == 0) {
        spy.wait(5000);
    }

    // Process events to ensure all signals are handled
    QCoreApplication::processEvents();

    QVERIFY(!api.isRunning());
}

// ============================================================================
// Signal Tests
// ============================================================================

void QGCJsonRequestTest::_testSignals()
{
    QGCJsonRequest api(this);

    QSignalSpy runningSpy(&api, &QGCJsonRequest::runningChanged);
    QSignalSpy httpStatusSpy(&api, &QGCJsonRequest::httpStatusChanged);
    QSignalSpy finishedSpy(&api, &QGCJsonRequest::finished);
    QSignalSpy finishedVariantSpy(&api, &QGCJsonRequest::finishedVariant);

    QVERIFY(api.get("http://localhost:99999/test"));

    // Should have emitted running=true
    QVERIFY(runningSpy.count() >= 1);

    // Wait for finished
    QVERIFY(finishedSpy.wait(5000));

    // Verify finished signal
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.first().count(), 3);  // success, response, errorMessage

    // Verify variant signal also fired
    QCOMPARE(finishedVariantSpy.count(), 1);

    // Running should be false
    QVERIFY(!api.isRunning());
}
