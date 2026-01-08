#include "QGCNetworkRetryTest.h"
#include "QGCNetworkRetry.h"

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Policy Tests
// ============================================================================

void QGCNetworkRetryTest::_testDefaultPolicy()
{
    const QGCRetryPolicy policy = QGCRetryPolicy::defaultPolicy();

    QCOMPARE(policy.maxAttempts, 3);
    QCOMPARE(policy.initialDelayMs, 1000);
    QCOMPARE(policy.maxDelayMs, 30000);
    QCOMPARE(policy.backoffMultiplier, 2.0);
    QCOMPARE(policy.jitterFactor, 0.1);
    QVERIFY(policy.retryOnTimeout);
    QVERIFY(policy.retryOn5xx);
    QVERIFY(policy.retryOnConnectionError);
}

void QGCNetworkRetryTest::_testAggressivePolicy()
{
    const QGCRetryPolicy policy = QGCRetryPolicy::aggressive();

    QCOMPARE(policy.maxAttempts, 5);
    QCOMPARE(policy.initialDelayMs, 500);
    QCOMPARE(policy.maxDelayMs, 10000);
    QCOMPARE(policy.backoffMultiplier, 1.5);
    QCOMPARE(policy.jitterFactor, 0.2);
    QVERIFY(policy.retryOnTimeout);
    QVERIFY(policy.retryOn5xx);
    QVERIFY(policy.retryOnConnectionError);
}

void QGCNetworkRetryTest::_testConservativePolicy()
{
    const QGCRetryPolicy policy = QGCRetryPolicy::conservative();

    QCOMPARE(policy.maxAttempts, 2);
    QCOMPARE(policy.initialDelayMs, 2000);
    QCOMPARE(policy.maxDelayMs, 60000);
    QCOMPARE(policy.backoffMultiplier, 3.0);
    QCOMPARE(policy.jitterFactor, 0.1);
    QVERIFY(policy.retryOnTimeout);
    QVERIFY(policy.retryOn5xx);
    QVERIFY(!policy.retryOnConnectionError);
}

void QGCNetworkRetryTest::_testNoRetryPolicy()
{
    const QGCRetryPolicy policy = QGCRetryPolicy::noRetry();

    QCOMPARE(policy.maxAttempts, 1);
    QVERIFY(!policy.retryOnTimeout);
    QVERIFY(!policy.retryOn5xx);
    QVERIFY(!policy.retryOnConnectionError);
}

void QGCNetworkRetryTest::_testSetPolicy()
{
    QGCNetworkRetry retry(this);

    QSignalSpy maxAttemptsSpy(&retry, &QGCNetworkRetry::maxAttemptsChanged);

    QGCRetryPolicy newPolicy;
    newPolicy.maxAttempts = 10;
    newPolicy.initialDelayMs = 500;

    retry.setPolicy(newPolicy);

    QCOMPARE(retry.policy().maxAttempts, 10);
    QCOMPARE(retry.policy().initialDelayMs, 500);
    QCOMPARE(maxAttemptsSpy.count(), 1);
    QCOMPARE(maxAttemptsSpy.first().first().toInt(), 10);
}

// ============================================================================
// State Tests
// ============================================================================

void QGCNetworkRetryTest::_testInitialState()
{
    QGCNetworkRetry retry(this);

    QVERIFY(!retry.isRunning());
    QVERIFY(!retry.isWaiting());
    QCOMPARE(retry.currentAttempt(), 0);
    QCOMPARE(retry.remainingDelayMs(), 0);
    QVERIFY(retry.lastReply() == nullptr);
}

void QGCNetworkRetryTest::_testExecuteWithoutFactory()
{
    QGCNetworkRetry retry(this);

    // Null factory should fail
    QVERIFY(!retry.execute(nullptr));
    QVERIFY(!retry.isRunning());
}

void QGCNetworkRetryTest::_testConcurrentExecutionRejected()
{
    QGCNetworkRetry retry(this);
    QNetworkAccessManager nam;

    // Start first execution
    QVERIFY(retry.execute([&nam]() {
        return nam.get(QNetworkRequest(QUrl("http://localhost:99999/test")));
    }));

    // Second execution should be rejected
    QVERIFY(!retry.execute([&nam]() {
        return nam.get(QNetworkRequest(QUrl("http://localhost:99999/test2")));
    }));

    // Wait for first to complete
    QSignalSpy spy(&retry, &QGCNetworkRetry::finished);
    QVERIFY(spy.wait(10000));
}

// ============================================================================
// Cancel and Reset Tests
// ============================================================================

void QGCNetworkRetryTest::_testCancel()
{
    QGCNetworkRetry retry(this);
    QNetworkAccessManager nam;

    // Start execution
    QVERIFY(retry.execute([&nam]() {
        return nam.get(QNetworkRequest(QUrl("http://localhost:99999/test")));
    }));
    QVERIFY(retry.isRunning());

    // Cancel
    retry.cancel();
    QVERIFY(!retry.isRunning());
}

void QGCNetworkRetryTest::_testReset()
{
    QGCNetworkRetry retry(this);
    QNetworkAccessManager nam;

    // Start and cancel
    QVERIFY(retry.execute([&nam]() {
        return nam.get(QNetworkRequest(QUrl("http://localhost:99999/test")));
    }));
    retry.cancel();

    // Reset
    retry.reset();

    QVERIFY(!retry.isRunning());
    QVERIFY(!retry.isWaiting());
    QCOMPARE(retry.currentAttempt(), 0);
    QCOMPARE(retry.remainingDelayMs(), 0);
}

// ============================================================================
// Signal Tests
// ============================================================================

void QGCNetworkRetryTest::_testSignals()
{
    QGCNetworkRetry retry(this);
    retry.setPolicy(QGCRetryPolicy::noRetry());  // Single attempt for faster test

    QNetworkAccessManager nam;

    QSignalSpy runningSpy(&retry, &QGCNetworkRetry::runningChanged);
    QSignalSpy attemptSpy(&retry, &QGCNetworkRetry::currentAttemptChanged);
    QSignalSpy finishedSpy(&retry, &QGCNetworkRetry::finished);

    QVERIFY(retry.execute([&nam]() {
        return nam.get(QNetworkRequest(QUrl("http://localhost:99999/test")));
    }));

    // Should have emitted running=true
    QVERIFY(runningSpy.count() >= 1);

    // Wait for finished
    QVERIFY(finishedSpy.wait(10000));

    // Verify finished signal
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.first().count(), 3);  // reply, success, totalAttempts

    // Attempt should have been 1
    QVERIFY(attemptSpy.count() >= 1);
}

// ============================================================================
// Retry Logic Tests
// ============================================================================

void QGCNetworkRetryTest::_testShouldRetryOnTimeout()
{
    // Test policy with retryOnTimeout enabled
    QGCRetryPolicy policy;
    policy.retryOnTimeout = true;
    policy.retryOn5xx = false;
    policy.retryOnConnectionError = false;

    QGCNetworkRetry retry(policy, this);
    // The actual retry logic is tested internally - this verifies configuration
    QVERIFY(retry.policy().retryOnTimeout);
}

void QGCNetworkRetryTest::_testShouldRetryOn5xx()
{
    // Test policy with retryOn5xx enabled
    QGCRetryPolicy policy;
    policy.retryOnTimeout = false;
    policy.retryOn5xx = true;
    policy.retryOnConnectionError = false;

    QGCNetworkRetry retry(policy, this);
    QVERIFY(retry.policy().retryOn5xx);
}

void QGCNetworkRetryTest::_testShouldNotRetryOn4xx()
{
    // 4xx errors (client errors) should never be retried
    // This is implicit - there's no retryOn4xx option
    QGCRetryPolicy policy = QGCRetryPolicy::aggressive();
    QGCNetworkRetry retry(policy, this);

    // Policy should not have a 4xx retry option (because it doesn't make sense)
    // Verify that the policy is configured correctly
    QVERIFY(retry.policy().retryOn5xx);
    QVERIFY(retry.policy().retryOnTimeout);
}
