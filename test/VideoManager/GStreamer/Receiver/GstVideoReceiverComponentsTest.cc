#include "GstVideoReceiverComponentsTest.h"

#ifdef QGC_GST_STREAMING

#include "StreamWatchdog.h"

namespace {

// Any monotonic time well clear of 0, which the watchdog reads as "no frame yet".
constexpr qint64 kNowMs = 100000;
constexpr qint64 kNoFrame = -1;

qint64 nextDelaySec(StreamWatchdog& watchdog)
{
    return watchdog.nextReconnectDelay().count();
}

}  // namespace

void GstVideoReceiverComponentsTest::_testReconnectBackoffSequence()
{
    StreamWatchdog watchdog;
    for (const qint64 expectedSec : {1, 2, 4, 8, 16, 30, 30}) {
        QCOMPARE(nextDelaySec(watchdog), expectedSec);
    }
}

void GstVideoReceiverComponentsTest::_testReconnectBackoffRestartsAfterRecovery()
{
    StreamWatchdog watchdog;
    for (int failedStart = 0; failedStart < 3; ++failedStart) {
        (void) nextDelaySec(watchdog);
    }

    // The reconnect worked and delivered frames before the stream failed again.
    watchdog.start(1);
    (void) watchdog.noteSourceFrame(kNowMs);
    QCOMPARE(nextDelaySec(watchdog), qint64(1));
    // Its own retry failing to start backs off from there.
    watchdog.start(1);
    QCOMPARE(nextDelaySec(watchdog), qint64(2));
}

void GstVideoReceiverComponentsTest::_testStallDetection_data()
{
    QTest::addColumn<uint>("timeoutSec");
    QTest::addColumn<qint64>("sourceFrameAgeMs");
    QTest::addColumn<qint64>("sinkFrameAgeMs");
    QTest::addColumn<bool>("decoding");
    QTest::addColumn<int>("expected");

    const int none = int(StreamWatchdog::Stall::None);
    const int source = int(StreamWatchdog::Stall::Source);
    const int decoder = int(StreamWatchdog::Stall::Decoder);

    QTest::newRow("source within timeout") << 2u << qint64(2000) << qint64(0) << true << none;
    QTest::newRow("source past timeout") << 2u << qint64(2001) << qint64(0) << true << source;
    QTest::newRow("zero timeout floors at 1 s, within") << 0u << qint64(1000) << kNoFrame << false << none;
    QTest::newRow("zero timeout floors at 1 s, past") << 0u << qint64(1001) << kNoFrame << false << source;
    QTest::newRow("sink within twice timeout") << 2u << qint64(0) << qint64(4000) << true << none;
    QTest::newRow("sink past twice timeout") << 2u << qint64(0) << qint64(4001) << true << decoder;
    QTest::newRow("sink silence ignored when not decoding") << 2u << qint64(0) << qint64(60000) << false << none;
    QTest::newRow("source stall reported first") << 2u << qint64(3000) << qint64(60000) << true << source;
}

void GstVideoReceiverComponentsTest::_testStallDetection()
{
    QFETCH(uint, timeoutSec);
    QFETCH(qint64, sourceFrameAgeMs);
    QFETCH(qint64, sinkFrameAgeMs);
    QFETCH(bool, decoding);
    QFETCH(int, expected);

    StreamWatchdog watchdog;
    watchdog.start(timeoutSec);
    (void) watchdog.noteSourceFrame(kNowMs - sourceFrameAgeMs);
    if (sinkFrameAgeMs != kNoFrame) {
        (void) watchdog.noteSinkFrame(kNowMs - sinkFrameAgeMs);
    }

    QCOMPARE(int(watchdog.check(kNowMs, decoding).stall), expected);
}

void GstVideoReceiverComponentsTest::_testSourceTimeoutStartsAtFirstCheck()
{
    // A source that never delivers a frame must still time out, counted from the first check.
    StreamWatchdog watchdog;
    watchdog.start(1);
    QCOMPARE(watchdog.check(kNowMs, false).stall, StreamWatchdog::Stall::None);
    QCOMPARE(watchdog.check(kNowMs + 1000, false).stall, StreamWatchdog::Stall::None);
    const StreamWatchdog::Result result = watchdog.check(kNowMs + 1001, false);
    QCOMPARE(result.stall, StreamWatchdog::Stall::Source);
    QCOMPARE(result.silentMs, qint64(1001));
}

#else

void GstVideoReceiverComponentsTest::_testReconnectBackoffSequence()
{
    QSKIP("GStreamer not enabled");
}

void GstVideoReceiverComponentsTest::_testReconnectBackoffRestartsAfterRecovery()
{
    QSKIP("GStreamer not enabled");
}

void GstVideoReceiverComponentsTest::_testStallDetection_data() {}

void GstVideoReceiverComponentsTest::_testStallDetection()
{
    QSKIP("GStreamer not enabled");
}

void GstVideoReceiverComponentsTest::_testSourceTimeoutStartsAtFirstCheck()
{
    QSKIP("GStreamer not enabled");
}

#endif

UT_REGISTER_TEST(GstVideoReceiverComponentsTest, TestLabel::Unit)
