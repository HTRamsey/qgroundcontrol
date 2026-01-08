#include "QGCDownloadQueueTest.h"
#include "QGCDownloadQueue.h"

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Construction Tests
// ============================================================================

void QGCDownloadQueueTest::_testConstruction()
{
    QGCDownloadQueue queue(this);
    QVERIFY(!queue.isRunning());
}

void QGCDownloadQueueTest::_testInitialState()
{
    QGCDownloadQueue queue(this);

    QVERIFY(!queue.isRunning());
    QVERIFY(!queue.isPaused());
    QCOMPARE(queue.pendingCount(), 0);
    QCOMPARE(queue.activeCount(), 0);
    QCOMPARE(queue.completedCount(), 0);
    QCOMPARE(queue.failedCount(), 0);
    QCOMPARE(queue.totalCount(), 0);
    QCOMPARE(queue.overallProgress(), 0.0);
    QCOMPARE(queue.maxConcurrent(), 3);
    QCOMPARE(queue.maxRetries(), 2);
}

// ============================================================================
// Configuration Tests
// ============================================================================

void QGCDownloadQueueTest::_testSetMaxConcurrent()
{
    QGCDownloadQueue queue(this);

    QSignalSpy spy(&queue, &QGCDownloadQueue::maxConcurrentChanged);

    queue.setMaxConcurrent(5);
    QCOMPARE(queue.maxConcurrent(), 5);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toInt(), 5);

    // Setting same value should not emit signal
    queue.setMaxConcurrent(5);
    QCOMPARE(spy.count(), 1);

    // Minimum is 1
    queue.setMaxConcurrent(0);
    QCOMPARE(queue.maxConcurrent(), 1);
}

void QGCDownloadQueueTest::_testSetMaxRetries()
{
    QGCDownloadQueue queue(this);

    queue.setMaxRetries(5);
    QCOMPARE(queue.maxRetries(), 5);

    queue.setMaxRetries(0);
    QCOMPARE(queue.maxRetries(), 0);
}

// ============================================================================
// Queue Management Tests
// ============================================================================

void QGCDownloadQueueTest::_testAddDownload()
{
    QGCDownloadQueue queue(this);

    QSignalSpy pendingSpy(&queue, &QGCDownloadQueue::pendingCountChanged);
    QSignalSpy totalSpy(&queue, &QGCDownloadQueue::totalCountChanged);

    queue.addDownload(QUrl("http://localhost:99999/file1.txt"), "/tmp/file1.txt");

    QCOMPARE(queue.pendingCount(), 1);
    QCOMPARE(queue.totalCount(), 1);
    QCOMPARE(pendingSpy.count(), 1);
    QCOMPARE(totalSpy.count(), 1);

    // Add another
    queue.addDownload(QUrl("http://localhost:99999/file2.txt"), "/tmp/file2.txt", "Test File");

    QCOMPARE(queue.pendingCount(), 2);
    QCOMPARE(queue.totalCount(), 2);
}

void QGCDownloadQueueTest::_testAddDownloadPriority()
{
    QGCDownloadQueue queue(this);

    QUrl normalUrl("http://localhost:99999/normal.txt");
    QUrl priorityUrl("http://localhost:99999/priority.txt");

    queue.addDownload(normalUrl, "/tmp/normal.txt");
    queue.addDownload(priorityUrl, "/tmp/priority.txt", "Priority", 0, true);

    QCOMPARE(queue.pendingCount(), 2);

    // Priority item should be first in pending list
    QList<QGCDownloadItem> pending = queue.pendingItems();
    QCOMPARE(pending.size(), 2);

    // Check that priority item is first
    QCOMPARE(pending.at(0).localPath, QStringLiteral("/tmp/priority.txt"));
    QCOMPARE(pending.at(1).localPath, QStringLiteral("/tmp/normal.txt"));
}

void QGCDownloadQueueTest::_testRemoveDownload()
{
    QGCDownloadQueue queue(this);

    queue.addDownload(QUrl("http://localhost:99999/file1.txt"), "/tmp/file1.txt");
    queue.addDownload(QUrl("http://localhost:99999/file2.txt"), "/tmp/file2.txt");

    QCOMPARE(queue.pendingCount(), 2);

    bool removed = queue.removeDownload(QUrl("http://localhost:99999/file1.txt"));
    QVERIFY(removed);
    QCOMPARE(queue.pendingCount(), 1);

    // Try to remove non-existent
    removed = queue.removeDownload(QUrl("http://localhost:99999/nonexistent.txt"));
    QVERIFY(!removed);
    QCOMPARE(queue.pendingCount(), 1);
}

void QGCDownloadQueueTest::_testClearCompleted()
{
    QGCDownloadQueue queue(this);

    queue.addDownload(QUrl("http://localhost:99999/file.txt"), "/tmp/file.txt");
    queue.clearCompleted();

    QCOMPARE(queue.completedCount(), 0);
    QCOMPARE(queue.failedCount(), 0);
    QCOMPARE(queue.overallProgress(), 0.0);
}

// ============================================================================
// State Tests
// ============================================================================

void QGCDownloadQueueTest::_testStartPause()
{
    QGCDownloadQueue queue(this);

    queue.addDownload(QUrl("http://localhost:99999/file.txt"), "/tmp/file.txt");

    QSignalSpy runningSpy(&queue, &QGCDownloadQueue::runningChanged);
    QSignalSpy pausedSpy(&queue, &QGCDownloadQueue::pausedChanged);

    queue.start();
    QVERIFY(queue.isRunning());
    QVERIFY(!queue.isPaused());
    QCOMPARE(runningSpy.count(), 1);

    queue.pause();
    QVERIFY(queue.isRunning());  // Still running, just paused
    QVERIFY(queue.isPaused());
    QVERIFY(pausedSpy.count() >= 1);  // At least one pause signal

    // Cancel to clean up (this may emit additional signals)
    queue.cancelAll();
}

void QGCDownloadQueueTest::_testPauseResume()
{
    QGCDownloadQueue queue(this);

    queue.addDownload(QUrl("http://localhost:99999/file.txt"), "/tmp/file.txt");
    queue.start();
    queue.pause();

    QVERIFY(queue.isPaused());

    QSignalSpy pausedSpy(&queue, &QGCDownloadQueue::pausedChanged);
    queue.resume();

    QVERIFY(!queue.isPaused());
    QCOMPARE(pausedSpy.count(), 1);

    // Cancel to clean up
    queue.cancelAll();
}

void QGCDownloadQueueTest::_testCancelAll()
{
    QGCDownloadQueue queue(this);

    queue.addDownload(QUrl("http://localhost:99999/file1.txt"), "/tmp/file1.txt");
    queue.addDownload(QUrl("http://localhost:99999/file2.txt"), "/tmp/file2.txt");

    queue.start();

    QSignalSpy runningSpy(&queue, &QGCDownloadQueue::runningChanged);

    queue.cancelAll();

    QVERIFY(!queue.isRunning());
    QVERIFY(!queue.isPaused());
    QCOMPARE(queue.pendingCount(), 0);
    QCOMPARE(queue.activeCount(), 0);
}

// ============================================================================
// Signal Tests
// ============================================================================

void QGCDownloadQueueTest::_testQueueSignals()
{
    QGCDownloadQueue queue(this);

    QSignalSpy pendingSpy(&queue, &QGCDownloadQueue::pendingCountChanged);
    QSignalSpy activeSpy(&queue, &QGCDownloadQueue::activeCountChanged);
    QSignalSpy startedSpy(&queue, &QGCDownloadQueue::downloadStarted);

    queue.addDownload(QUrl("http://localhost:99999/file.txt"), "/tmp/file.txt");
    QCOMPARE(pendingSpy.count(), 1);

    queue.start();

    // Wait a bit for signals
    QTest::qWait(100);

    // Should have started download
    QVERIFY(startedSpy.count() >= 1 || activeSpy.count() >= 1);

    queue.cancelAll();
}
