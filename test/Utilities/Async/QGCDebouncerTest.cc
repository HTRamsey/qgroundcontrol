#include "QGCDebouncerTest.h"
#include "QGCDebouncer.h"

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

void QGCDebouncerTest::_testDefaultConfiguration()
{
    QGCDebouncer debouncer(250, this);

    QCOMPARE(debouncer.delayMs(), 250);
    QVERIFY(!debouncer.isPending());
    QVERIFY(!debouncer.isLeading());
    QVERIFY(debouncer.isTrailing());
}

void QGCDebouncerTest::_testDebounce()
{
    QGCDebouncer debouncer(50, this);  // 50ms delay

    QSignalSpy triggeredSpy(&debouncer, &QGCDebouncer::triggered);

    // Call multiple times rapidly
    debouncer.call(QVariant());
    debouncer.call(QVariant());
    debouncer.call(QVariant());

    QVERIFY(debouncer.isPending());
    QCOMPARE(triggeredSpy.count(), 0);  // Not fired yet

    // Wait for debounce delay
    QVERIFY(triggeredSpy.wait(200));
    QCOMPARE(triggeredSpy.count(), 1);  // Only one trigger
}

void QGCDebouncerTest::_testLeadingEdge()
{
    QGCDebouncer debouncer(100, this);
    debouncer.setLeading(true);
    debouncer.setTrailing(false);

    QSignalSpy triggeredSpy(&debouncer, &QGCDebouncer::triggered);

    // Should fire immediately on first call
    debouncer.call(QVariant());
    QCOMPARE(triggeredSpy.count(), 1);

    // Subsequent calls should be ignored
    debouncer.call(QVariant());
    debouncer.call(QVariant());
    QCOMPARE(triggeredSpy.count(), 1);
}

void QGCDebouncerTest::_testCancel()
{
    QGCDebouncer debouncer(100, this);

    QSignalSpy triggeredSpy(&debouncer, &QGCDebouncer::triggered);

    debouncer.call(QVariant());
    QVERIFY(debouncer.isPending());

    // Cancel before it fires
    debouncer.cancel();
    QVERIFY(!debouncer.isPending());

    // Wait and verify it never fired
    QTest::qWait(150);
    QCOMPARE(triggeredSpy.count(), 0);
}

void QGCDebouncerTest::_testFlush()
{
    QGCDebouncer debouncer(500, this);

    QSignalSpy triggeredSpy(&debouncer, &QGCDebouncer::triggered);

    debouncer.call(QVariant());
    QVERIFY(debouncer.isPending());
    QCOMPARE(triggeredSpy.count(), 0);

    // Flush should fire immediately
    debouncer.flush();
    QCOMPARE(triggeredSpy.count(), 1);
    QVERIFY(!debouncer.isPending());
}

void QGCDebouncerTest::_testThrottler()
{
    QGCThrottler throttler(100, this);

    QSignalSpy triggeredSpy(&throttler, &QGCThrottler::triggered);

    // First call should execute immediately (leading edge)
    throttler.call(QVariant());
    QCOMPARE(triggeredSpy.count(), 1);

    // Immediate subsequent calls should be throttled
    throttler.call(QVariant());
    QCOMPARE(triggeredSpy.count(), 1);  // Still just 1

    // Wait for throttle interval plus trailing
    QVERIFY(triggeredSpy.wait(200));
    QVERIFY(triggeredSpy.count() >= 2);
}
