#pragma once

#include <atomic>
#include <glib.h>
#include <gst/gstelement.h>
#include <gst/gstpad.h>
#include <utility>

#include <QtCore/QMetaObject>
#include <QtCore/QMutex>
#include <QtCore/QString>

#include "VideoReceiver.h"

class QThread;
class QTimer;
class GStreamerTest;

class GstVideoReceiver : public VideoReceiver
{
    Q_OBJECT
    Q_PROPERTY(QString decoderName       READ decoderName       NOTIFY decoderStatsChanged)
    Q_PROPERTY(quint64 processedFrames   READ processedFrames   NOTIFY decoderStatsChanged)
    Q_PROPERTY(quint64 droppedFrames     READ droppedFrames     NOTIFY decoderStatsChanged)
    Q_PROPERTY(qint64  currentJitterNs   READ currentJitterNs   NOTIFY decoderStatsChanged)
    Q_PROPERTY(double  qosProportion     READ qosProportion     NOTIFY decoderStatsChanged)
    Q_PROPERTY(int     qosQuality        READ qosQuality        NOTIFY decoderStatsChanged)

public:
    explicit GstVideoReceiver(QObject *parent = nullptr);
    ~GstVideoReceiver();

    QString decoderName()     const { QMutexLocker locker(&_decoderNameMutex); return _decoderName; }
    quint64 processedFrames() const { return _processedFrames.load(std::memory_order_relaxed); }
    quint64 droppedFrames()   const { return _droppedFrames.load(std::memory_order_relaxed); }
    qint64  currentJitterNs() const { return _currentJitterNs.load(std::memory_order_relaxed); }
    double  qosProportion()   const { return _qosProportion.load(std::memory_order_relaxed); }
    int     qosQuality()      const { return _qosQuality.load(std::memory_order_relaxed); }

public slots:
    void start(uint32_t timeout) override;
    void stop() override;
    void startDecoding(void *sink) override;
    void stopDecoding() override;
    void startRecording(const QString &videoFile, FILE_FORMAT format) override;
    void stopRecording() override;
    void takeScreenshot(const QString &imageFile) override;

    /// Dump the current pipeline graph to GST_DEBUG_DUMP_DOT_DIR (if set) plus
    /// CacheLocation/qgc-pipeline-dot for field-bug-report bundles. No-op when
    /// the pipeline isn't running. Callable from QML for a debug menu.
    Q_INVOKABLE void dumpPipelineGraph(const QString &tag = QStringLiteral("manual"));

signals:
    void decoderStatsChanged();

private:
    friend class GStreamerTest;

    /// Settings a pipeline is started with, captured on the caller's thread so the worker and
    /// streaming threads never read members the GUI thread writes.
    struct StartConfig
    {
        QString uri;
        QString redactedUri;
        uint32_t timeoutSec = 0;
        bool lowLatency = false;
        int rtpJitterLatencyMs = 0;
    };

    /// Queues @p task on the worker thread, which owns the pipeline and all receiver state not marked atomic.
    template <typename Task>
    void _runOnWorker(Task&& task)
    {
        (void) QMetaObject::invokeMethod(_workerContext, std::forward<Task>(task), Qt::QueuedConnection);
    }

    bool _onWorkerThread() const;

    QString _redactedUri() const;
    GstElement *_makeDecoder();
    static GstElement* _makeFileSink(const QString& videoFile, FILE_FORMAT format, const GstCaps* inputCaps);

    void _startPipeline(const StartConfig& config);
    void _startDecoding(GstElement* videoSink, bool hasWidget);
    void _onNewSourcePad(GstPad *pad);
    void _onNewDecoderPad(GstPad *pad);
    bool _addDecoder(GstElement *src);
    void _ensureVideoSinkInPipeline();
    bool _addVideoSink(GstPad *pad);
    void _noteTeeFrame();
    void _noteVideoSinkFrame();
    void _noteEndOfStream();
    /// -Unlink the branch from the src pad
    /// -Send an EOS event at the beginning of that branch
    bool _unlinkBranch(GstElement *from);
    void _shutdownDecodingBranch();
    void _shutdownRecordingBranch();
    void _removeSourceEosProbe();
    /// Tears down the running pipeline (finalizing an active recording first) and resets all
    /// pipeline state. Returns false when nothing was running.
    bool _stopPipeline();
    /// Releases every pipeline-scoped element, probe and flag, emitting the matching change
    /// signals. Safe on a partially built pipeline (failed start()).
    void _resetPipelineState();
    void _logDecodebin3SelectedCodec(GstElement *decodebin3);
    void _handleEOS();
    void _onWatchdogTick();

    /// Stop the pipeline and queue a delayed restart with exponential backoff.
    /// `reason` is logged so reconnect storms are diagnosable. No-op when
    /// autoReconnect() is disabled.
    void _scheduleReconnect(const char *reason);
    void _reconnect();

    /// Returns a strong ref to _pipeline (caller must gst_object_unref) or nullptr if torn down.
    /// Bus sync-message callbacks run on the streaming thread concurrent with stop() on the
    /// worker thread, so direct dereference of _pipeline races with gst_clear_object(&_pipeline).
    GstElement *_acquirePipelineRef() const;

    static gboolean _onBusMessage(GstBus *bus, GstMessage *message, gpointer user_data);
    static void _onNewPad(GstElement *element, GstPad *pad, gpointer data);
    static GstPadProbeReturn _teeProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _videoSinkProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _eosProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _keyframeWatch(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

    QThread* _workerThread = nullptr;
    /// Lives on _workerThread; tasks and the timers below run there. Deleted when the thread finishes.
    QObject* _workerContext = nullptr;
    QTimer* _watchdogTimer = nullptr;
    QTimer* _reconnectTimer = nullptr;

    StartConfig _config;
    GstElement *_decoder = nullptr;
    GstElement *_decoderValve = nullptr;
    GstElement *_fileSink = nullptr;
    GstElement *_pipeline = nullptr;
    mutable QMutex _pipelineMutex;  // serializes _pipeline mutation (worker) vs read in _onBusMessage (streaming thread)
    GstElement *_recorderValve = nullptr;
    GstElement *_source = nullptr;
    GstElement *_tee = nullptr;
    GstElement *_videoSink = nullptr;
    int _reconnectAttempts = 0;
    /// Bumped on every teardown (stop() or failed start()); queued ERROR handlers captured under an
    /// older epoch become no-ops.
    std::atomic<quint64> _reconnectEpoch = 0;
    std::atomic<quint64> _sourceFrameCount =
        0;  ///< Tee-probe frame tally (streaming thread); drives the source-side flow heartbeat log.
    gulong _teeProbeId = 0;
    gulong _videoSinkProbeId = 0;
    gulong _eosProbeId = 0;
    // Ref-held source pad carrying _eosProbeId; released with the pipeline, not the decode branch.
    GstPad* _eosProbePad = nullptr;
    std::atomic<gulong> _keyframeWatchId = 0;  // cleared by _keyframeWatch (streaming thread) when it removes itself
    bool _recordingStopRequested = false;

    bool _recording = false;
    bool _removingDecoder = false;
    bool _removingRecorder = false;
    int _statsTickCounter = 0;
    // Flipped on streaming threads.
    std::atomic<bool> _decoding = false;
    std::atomic<bool> _streaming = false;
    std::atomic<bool> _endOfStream = false;
    // Monotonic ms. Written on streaming threads (pad probes), read/written by the watchdog on the worker thread.
    std::atomic<qint64> _lastSourceFrameTime = 0;
    std::atomic<qint64> _lastVideoFrameTime = 0;

    mutable QMutex _decoderNameMutex;  // QString refcount isn't thread-safe across reader/writer threads
    QString _decoderName;
    std::atomic<quint64> _processedFrames{0};   // written on streaming thread (QOS), read on GUI
    std::atomic<quint64> _droppedFrames{0};
    std::atomic<qint64>  _currentJitterNs{0};
    std::atomic<double>  _qosProportion{1.0};
    std::atomic<int>     _qosQuality{1000000};
    std::atomic<bool> _qosStatsDirty{false};  // set per QOS message, drained by the 1 Hz watchdog tick

    static constexpr const char *_kFileMux[FILE_FORMAT_MAX + 1] = {
        "matroskamux",
        "qtmux",
        "mp4mux"
    };
};
