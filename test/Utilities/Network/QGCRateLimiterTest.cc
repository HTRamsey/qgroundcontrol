#include "QGCRateLimiterTest.h"
#include "QGCRateLimiter.h"

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

void QGCRateLimiterTest::_testDefaultConfiguration()
{
    QGCRateLimiter limiter(10, 1000, this);

    QCOMPARE(limiter.maxRequests(), 10);
    QCOMPARE(limiter.windowMs(), 1000);
    QVERIFY(limiter.availableTokens() > 0);
}

void QGCRateLimiterTest::_testCanExecute()
{
    QGCRateLimiter limiter(5, 1000, this);  // 5 requests per second

    // Should be able to execute up to max
    for (int i = 0; i < 5; ++i) {
        QVERIFY(limiter.canExecute());
        QVERIFY(limiter.tryExecute([]() {}));
    }

    // Should be at limit now
    QVERIFY(!limiter.canExecute());
}

void QGCRateLimiterTest::_testTryExecute()
{
    QGCRateLimiter limiter(3, 1000, this);  // 3 requests per second

    // Use up capacity
    QVERIFY(limiter.tryExecute([]() {}));
    QVERIFY(limiter.tryExecute([]() {}));
    QVERIFY(limiter.tryExecute([]() {}));

    // Should fail now
    QVERIFY(!limiter.tryExecute([]() {}));
}

void QGCRateLimiterTest::_testBurstCapacity()
{
    QGCRateLimiter limiter(5, 1000, this);

    limiter.setBurstCapacity(3);
    QCOMPARE(limiter.burstCapacity(), 3);
}

void QGCRateLimiterTest::_testExecuteWithCallback()
{
    QGCRateLimiter limiter(10, 1000, this);

    bool executed = false;
    limiter.execute([&executed]() {
        executed = true;
    });

    QVERIFY(executed);
}

void QGCRateLimiterTest::_testReset()
{
    QGCRateLimiter limiter(3, 1000, this);

    // Use up all tokens
    limiter.tryExecute([]() {});
    limiter.tryExecute([]() {});
    limiter.tryExecute([]() {});
    QVERIFY(!limiter.canExecute());

    // Reset should restore tokens
    limiter.reset();
    QVERIFY(limiter.canExecute());
}

void QGCRateLimiterTest::_testTimeUntilAvailable()
{
    QGCRateLimiter limiter(1, 1000, this);  // 1 request per second

    // Use the only token
    QVERIFY(limiter.tryExecute([]() {}));
    QVERIFY(!limiter.canExecute());

    // Should report time until next token
    const int timeUntil = limiter.timeUntilAvailable();
    QVERIFY(timeUntil > 0);
    QVERIFY(timeUntil <= 1000);  // Should be at most 1 second
}
