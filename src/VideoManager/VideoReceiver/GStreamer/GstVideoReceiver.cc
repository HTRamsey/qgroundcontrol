//-----------------------------------------------------------------------------
// Our pipeline look like this:
//
//              +-->queue-->_decoderValve[-->_decoder-->_videoSink]
//              |
// _source-->_tee
//              |
//              +-->RecordingBranch: queue-->valve[-->sinkbin]
//-----------------------------------------------------------------------------

#include "GstVideoReceiver.h"

#include <chrono>
#include <gst/gst.h>
#include <memory>

#include <QtCore/QDeadlineTimer>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtQuick/QQuickItem>

#include "GStreamerHelpers.h"
#include "GstScoped.h"
#include "GstSourceFactory.h"
#include "HwBuffers/common/HwBuffers.h"
#include "QGCLoggingCategory.h"
#include "QGCNetworkHelper.h"
#include "QGCQVideoSinkController.h"

QGC_LOGGING_CATEGORY(GstVideoReceiverLog, "Video.GStreamer.GstVideoReceiver")

namespace {
// Destructor wait for the worker. Must exceed the recording finalize timeout so a finalizing recording isn't
// abandoned.
constexpr std::chrono::seconds kWorkerShutdownTimeout = RecordingBranch::kFinalizeTimeout + std::chrono::seconds{2};
constexpr std::chrono::milliseconds kWatchdogInterval{1000};

void logBusWarning(GstMessage* message)
{
    // GStreamer posts WARNING for caps mismatches, decoder fallbacks, clock drift — surfacing keeps these
    // visible without escalating to STATUS_FAIL.
    gchar* debug = nullptr;
    GError* error = nullptr;
    gst_message_parse_warning(message, &error, &debug);
    qCWarning(GstVideoReceiverLog) << "GStreamer warning:" << (error ? error->message : "(no message)")
                                   << "debug:" << (debug ? debug : "(none)");
    g_clear_error(&error);
    g_clear_pointer(&debug, g_free);
}

/// Selecting only video keeps decodebin3 from instantiating audio decoder branches. Done synchronously so the
/// selection is in place before decodebin3 falls back to its default one.
void selectVideoStreams(GstMessage* message)
{
    GstStreamCollection* collection = nullptr;
    gst_message_parse_stream_collection(message, &collection);
    if (!collection) {
        return;
    }
    GList* selectedIds = nullptr;
    const guint nStreams = gst_stream_collection_get_size(collection);
    for (guint i = 0; i < nStreams; ++i) {
        GstStream* stream = gst_stream_collection_get_stream(collection, i);
        if (gst_stream_get_stream_type(stream) & GST_STREAM_TYPE_VIDEO) {
            selectedIds = g_list_append(selectedIds, g_strdup(gst_stream_get_stream_id(stream)));
        }
    }
    if (selectedIds) {
        (void) gst_element_send_event(GST_ELEMENT(GST_MESSAGE_SRC(message)), gst_event_new_select_streams(selectedIds));
        g_list_free_full(selectedIds, g_free);
    }
    gst_object_unref(collection);
}

using GstElementRef = std::unique_ptr<GstElement, GStreamer::GstObjectDeleter>;
using GstPadRef = std::unique_ptr<GstPad, GStreamer::GstObjectDeleter>;

// Blocking probe: returning OK keeps the pad blocked until the probe is removed.
GstPadProbeReturn holdPad(GstPad* /*pad*/, GstPadProbeInfo* /*info*/, gpointer /*userData*/)
{
    return GST_PAD_PROBE_OK;
}

// Refs the element's first src pad into *userData and stops iterating. Resync is handled
// internally by gst_element_foreach_src_pad (unlike a bare gst_iterator_next loop).
gboolean grabFirstSrcPad(GstElement * /*element*/, GstPad *pad, gpointer userData)
{
    *static_cast<GstPad **>(userData) = GST_PAD(gst_object_ref(pad));
    return FALSE;
}

bool isRecoverableH265PaciError(GstMessage *msg, const GError *error, const gchar *debug)
{
    if (!msg || !error || (error->domain != GST_STREAM_ERROR) || (error->code != GST_STREAM_ERROR_FORMAT) ||
        !debug || !g_strrstr(debug, "NAL unit type 50 not supported yet")) {
        return false;
    }

    GstObject *src = GST_MESSAGE_SRC(msg);
    if (!src || !GST_IS_ELEMENT(src)) {
        return false;
    }

    GstElementFactory *factory = gst_element_get_factory(GST_ELEMENT(src));
    return factory && (g_strcmp0(GST_OBJECT_NAME(factory), "rtph265depay") == 0);
}

}  // namespace

GstVideoReceiver::GstVideoReceiver(QObject* parent)
    : VideoReceiver(parent)
    , _workerThread(new QThread(this))
    , _workerContext(new QObject())
    , _watchdogTimer(new QTimer(_workerContext))
    , _reconnectTimer(new QTimer(_workerContext))
    , _recordingBranch(*this, _workerContext)
{
    qCDebug(GstVideoReceiverLog) << this;

    _workerThread->setObjectName(QStringLiteral("GstVideoReceiver"));
    _watchdogTimer->setInterval(kWatchdogInterval);
    _reconnectTimer->setSingleShot(true);
    // Backoff delays are asserted by tests; a coarse timer may fire up to 5% early.
    _reconnectTimer->setTimerType(Qt::PreciseTimer);
    (void) connect(_watchdogTimer, &QTimer::timeout, _workerContext, [this]() { _onWatchdogTick(); });
    (void) connect(_reconnectTimer, &QTimer::timeout, _workerContext, [this]() { _reconnect(); });

    _workerContext->moveToThread(_workerThread);
    (void) connect(_workerThread, &QThread::finished, _workerContext, &QObject::deleteLater);
    _workerThread->start();
}

GstVideoReceiver::~GstVideoReceiver()
{
    // Tear down synchronously: queued tasks and pipeline callbacks capture `this`, so the worker
    // must be idle and the pipeline gone before members are destroyed. No onStopComplete here;
    // listeners must not be handed a receiver that is being destroyed.
    _runOnWorker([this]() {
        _reconnectTimer->stop();
        (void) _stopPipeline();
        _workerThread->quit();
    });

    if (!_workerThread->wait(QDeadlineTimer(kWorkerShutdownTimeout))) {
        // Destroying a running QThread aborts the process, and queued tasks still reference this receiver.
        qCWarning(GstVideoReceiverLog) << "Worker still busy after"
                                       << std::chrono::milliseconds(kWorkerShutdownTimeout).count()
                                       << "ms; waiting for it to finish";
        (void) _workerThread->wait();
    }

    qCDebug(GstVideoReceiverLog) << this;
}

bool GstVideoReceiver::_onWorkerThread() const
{
    return QThread::currentThread() == _workerThread;
}

QString GstVideoReceiver::_redactedUri() const
{
    return _config.redactedUri;
}

void GstVideoReceiver::start(uint32_t timeout)
{
    const StartConfig config{
        .uri = _uri,
        .redactedUri = QGCNetworkHelper::redactedUrlForLogging(_uri),
        .timeoutSec = timeout,
        .lowLatency = _lowLatency,
        .rtpJitterLatencyMs = _rtpJitterLatencyMs,
    };
    _runOnWorker([this, config]() {
        // A user start supersedes any pending reconnect and restarts the backoff.
        _reconnectTimer->stop();
        _watchdog.resetBackoff();
        _startPipeline(config);
    });
}

void GstVideoReceiver::_startPipeline(const StartConfig& config)
{
    if (_pipeline) {
        qCDebug(GstVideoReceiverLog) << "Already running!" << _redactedUri();
        emit onStartComplete(STATUS_INVALID_STATE);
        return;
    }

    if (config.uri.isEmpty()) {
        qCDebug(GstVideoReceiverLog) << "Failed because URI is not specified";
        emit onStartComplete(STATUS_INVALID_URL);
        return;
    }

    _config = config;
    _watchdog.start(_config.timeoutSec);

    qCDebug(GstVideoReceiverLog) << "Starting" << _redactedUri() << ", lowLatency" << _config.lowLatency << ", timeout"
                                 << _config.timeoutSec;

    bool running = false;

    do {
        GstElement* pipeline = gst_pipeline_new("receiver");
        if (!pipeline) {
            qCCritical(GstVideoReceiverLog) << "gst_pipeline_new() failed";
            break;
        }
        _pipeline = pipeline;
        // Before anything can post: the context bridges in the handler must see NEED_CONTEXT, which upstream
        // queries during PAUSED->PLAYING.
        _attachBusHandler();

        g_object_set(_pipeline, "message-forward", TRUE, nullptr);

        _tee = gst_element_factory_make("tee", nullptr);
        if (!_tee)  {
            qCCritical(GstVideoReceiverLog) << "gst_element_factory_make('tee') failed";
            break;
        }
        (void) gst_bin_add(GST_BIN(_pipeline), _tee);

        GstPad *pad = gst_element_get_static_pad(_tee, "sink");
        if (!pad) {
            qCCritical(GstVideoReceiverLog) << "gst_element_get_static_pad() failed";
            break;
        }

        _teeProbeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, _teeProbe, this, nullptr);
        gst_clear_object(&pad);
        if (_teeProbeId == 0) {
            // _teeProbe feeds the watchdog; without it the watchdog fires spuriously instead of reporting a real
            // failure.
            qCCritical(GstVideoReceiverLog) << "gst_pad_add_probe(_teeProbe) failed";
            break;
        }

        GstElement* decoderQueue = gst_element_factory_make("queue", nullptr);
        if (!decoderQueue)  {
            qCCritical(GstVideoReceiverLog) << "gst_element_factory_make('queue') failed";
            break;
        }

        // leaky=downstream (2) + tiny depth: the live-display branch must drop the oldest
        // buffer on backpressure, not stall the streaming thread. The recording branch
        // keeps default non-leaky semantics so every frame reaches the muxer.
        g_object_set(decoderQueue,
                     "leaky", 2,
                     "max-size-buffers", 2,
                     "max-size-bytes", 0,
                     "max-size-time", G_GUINT64_CONSTANT(0),
                     nullptr);
        (void) gst_bin_add(GST_BIN(_pipeline), decoderQueue);

        _decoderValve = gst_element_factory_make("valve", nullptr);
        if (!_decoderValve)  {
            qCCritical(GstVideoReceiverLog) << "gst_element_factory_make('valve') failed";
            break;
        }

        g_object_set(_decoderValve,
                     "drop", TRUE,
                     nullptr);
        (void) gst_bin_add(GST_BIN(_pipeline), _decoderValve);

        if (!gst_element_link_many(_tee, decoderQueue, _decoderValve, nullptr)) {
            qCCritical(GstVideoReceiverLog) << "Unable to link decoder queue";
            break;
        }

        if (!_recordingBranch.build(_pipeline, _tee)) {
            break;
        }

        GStreamer::SourceFactory::Config sourceConfig;
        sourceConfig.jitterBuffer = _config.lowLatency ? GStreamer::SourceFactory::JitterBuffer::None
                                                       : GStreamer::SourceFactory::JitterBuffer::DropOnLatency;
        sourceConfig.latencyMs = _config.rtpJitterLatencyMs;
        // do-retransmission needs ≥40 ms latency headroom over the default 20 ms rtx-delay;
        // forcibly disable for sub-frame latency configurations to avoid retransmit storms.
        sourceConfig.doRetransmission = (_config.rtpJitterLatencyMs >= 40) &&
                                        (sourceConfig.jitterBuffer != GStreamer::SourceFactory::JitterBuffer::None);
        _source = GStreamer::SourceFactory::create(_config.uri, sourceConfig);
        if (!_source) {
            qCCritical(GstVideoReceiverLog) << "SourceFactory::create() failed";
            break;
        }
        (void) gst_bin_add(GST_BIN(_pipeline), _source);

        GstPad *srcPad = nullptr;
        (void) gst_element_foreach_src_pad(_source, grabFirstSrcPad, &srcPad);

        if (srcPad) {
            _onNewSourcePad(srcPad);
            gst_clear_object(&srcPad);
        } else {
            (void) g_signal_connect(_source, "pad-added", G_CALLBACK(_onNewPad), this);
        }

        _dotDumper.dump(_pipeline, "pipeline-initial");
        running = (gst_element_set_state(_pipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE);
    } while(0);

    if (!running) {
        qCWarning(GstVideoReceiverLog) << "Failed to start" << _redactedUri();
        _resetPipelineState();

        emit onStartComplete(STATUS_FAIL);
        _scheduleReconnect("start failure");
    } else {
        _dotDumper.dump(_pipeline, "pipeline-started");
        qCDebug(GstVideoReceiverLog) << "Started" << _redactedUri();

        _watchdogTimer->start();
        emit onStartComplete(STATUS_OK);
    }
}

void GstVideoReceiver::stop()
{
    _runOnWorker([this]() {
        // A user stop also cancels a pending reconnect.
        _reconnectTimer->stop();
        _watchdog.resetBackoff();
        emit onStopComplete(_stopPipeline() ? STATUS_OK : STATUS_INVALID_STATE);
    });
}

bool GstVideoReceiver::_stopPipeline()
{
    if (!_pipeline) {
        qCDebug(GstVideoReceiverLog) << "Not running" << _redactedUri();
        // Still drop any state left behind.
        _resetPipelineState();
        return false;
    }

    qCDebug(GstVideoReceiverLog) << "Stopping" << _redactedUri();

    _recordingBranch.finishForTeardown();

    _resetPipelineState();

    qCDebug(GstVideoReceiverLog) << "Stopped" << _redactedUri();

    if (const HwBuffers::PathStats hwStats = HwBuffers::formatPathStats(true); hwStats.totalDelivered > 0) {
        qCInfo(GstVideoReceiverLog).noquote()
            << "HW path stats" << _redactedUri() << hwStats.line + HwBuffers::takeExtraPathStats();
    }

    return true;
}

void GstVideoReceiver::_attachBusHandler()
{
    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
    if (!bus) {
        qCCritical(GstVideoReceiverLog) << "gst_pipeline_get_bus() failed";
        return;
    }
    gst_bus_set_sync_handler(bus, _onBusSyncMessage, this, [](gpointer receiver) {
        static_cast<GstVideoReceiver*>(receiver)->_busHandlerReleased.release();
    });
    gst_object_unref(bus);
    _busHandlerAttached = true;
}

void GstVideoReceiver::_detachBusHandler()
{
    if (!_busHandlerAttached) {
        return;
    }
    _busHandlerAttached = false;

    if (GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline))) {
        gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
        gst_object_unref(bus);
    }
    // GStreamer refcounts the handler per post and releases it once the last in-flight call has returned.
    _busHandlerReleased.acquire();
}

void GstVideoReceiver::_resetPipelineState()
{
    _watchdogTimer->stop();

    if (_pipeline) {
        (void) gst_element_set_state(_pipeline, GST_STATE_NULL);
        (void) gst_element_get_state(_pipeline, nullptr, nullptr, GST_CLOCK_TIME_NONE);
    }

    // Callbacks have stopped: streaming threads are joined and no thread is left in the bus handler. Work they
    // queued for this pipeline is now stale.
    _detachBusHandler();
    _pipelineGeneration.fetch_add(1, std::memory_order_relaxed);

    if (_teeProbeId != 0 && _tee) {
        GstPad* sinkpad = gst_element_get_static_pad(_tee, "sink");
        if (sinkpad) {
            gst_pad_remove_probe(sinkpad, _teeProbeId);
            gst_clear_object(&sinkpad);
        }
    }
    _teeProbeId = 0;

    _removeSourceEosProbe();

    _recordingBranch.reset();

    // The video sink outlives the pipeline (owned by the caller of startDecoding), so its probe
    // and pipeline membership must be dropped even when start() failed before decoding began.
    if (_decoder || _videoSink) {
        _shutdownDecodingBranch();
    }

    _dotDumper.dump(_pipeline, "pipeline-stopped");

    gst_clear_object(&_pipeline);

    // Owned by the (now released) pipeline.
    _decoderValve = nullptr;
    _tee = nullptr;
    _source = nullptr;

    _watchdog.stop();

    if (std::exchange(_streaming, false)) {
        qCDebug(GstVideoReceiverLog) << "Streaming stopped" << _redactedUri();
        emit streamingChanged(false);
    }
}

void GstVideoReceiver::startDecoding(void *sink)
{
    if (!sink) {
        qCCritical(GstVideoReceiverLog) << "VideoSink is NULL";
        return;
    }

    const bool hasWidget = (_widget != nullptr);
    _runOnWorker([this, sink, hasWidget]() { _startDecoding(GST_ELEMENT(sink), hasWidget); });
}

void GstVideoReceiver::_startDecoding(GstElement* videoSink, bool hasWidget)
{
    qCDebug(GstVideoReceiverLog) << "Starting decoding" << _redactedUri();

    if (!hasWidget) {
        qCDebug(GstVideoReceiverLog) << "Video Widget is NULL" << _redactedUri();
        emit onStartDecodingComplete(STATUS_FAIL);
        return;
    }

    if (!_pipeline && _videoSink) {
        // Replace a sink stored before start(); also drops its frame probe.
        _shutdownDecodingBranch();
    }

    if (_videoSink || _decoding) {
        qCDebug(GstVideoReceiverLog) << "Already decoding!" << _redactedUri();
        emit onStartDecodingComplete(STATUS_INVALID_STATE);
        return;
    }

    GstPad *pad = gst_element_get_static_pad(videoSink, "sink");
    if (!pad) {
        qCCritical(GstVideoReceiverLog) << "Unable to find sink pad of video sink" << _redactedUri();
        emit onStartDecodingComplete(STATUS_FAIL);
        return;
    }

    _watchdog.clearSinkFrames();

    _videoSinkProbeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, _videoSinkProbe, this, nullptr);
    gst_clear_object(&pad);

    // The caller keeps its own (non-floating) reference; ours is dropped in _shutdownDecodingBranch().
    _videoSink = GST_ELEMENT(gst_object_ref(videoSink));

    if (!_streaming) {
        emit onStartDecodingComplete(STATUS_OK);
        return;
    }

    _ensureVideoSinkInPipeline();

    if (!_addDecoder(_decoderValve)) {
        qCWarning(GstVideoReceiverLog) << "_addDecoder() failed" << _redactedUri();
        _shutdownDecodingBranch();
        emit onStartDecodingComplete(STATUS_FAIL);
        return;
    }

    g_object_set(_decoderValve,
                 "drop", FALSE,
                 nullptr);

    qCDebug(GstVideoReceiverLog) << "Decoding started" << _redactedUri();

    emit onStartDecodingComplete(STATUS_OK);
}

void GstVideoReceiver::stopDecoding()
{
    if (!_onWorkerThread()) {
        _runOnWorker([this]() { stopDecoding(); });
        return;
    }

    qCDebug(GstVideoReceiverLog) << "Stopping decoding" << _redactedUri();

    // Gate on _videoSink (set by startDecoding) instead of _decoding (which only flips on the first frame),
    // so a stop before the first frame still removes the branch.
    if (!_videoSink) {
        qCDebug(GstVideoReceiverLog) << "Not decoding!" << _redactedUri();
        emit onStopDecodingComplete(STATUS_INVALID_STATE);
        return;
    }

    // The display branch has nothing to flush, so it is torn down at once instead of draining an EOS.
    if (_decoderValve) {
        g_object_set(_decoderValve, "drop", TRUE, nullptr);
    }
    _shutdownDecodingBranch();

    emit onStopDecodingComplete(STATUS_OK);
}

void GstVideoReceiver::startRecording(const QString &videoFile, FILE_FORMAT format)
{
    if (!_onWorkerThread()) {
        _runOnWorker([this, videoFile, format]() { startRecording(videoFile, format); });
        return;
    }

    _recordingBranch.start(videoFile, format);
}

void GstVideoReceiver::stopRecording()
{
    if (!_onWorkerThread()) {
        _runOnWorker([this]() { stopRecording(); });
        return;
    }

    _recordingBranch.stop();
}

void GstVideoReceiver::takeScreenshot(const QString &imageFile)
{
    if (!_onWorkerThread()) {
        _runOnWorker([this, imageFile]() { takeScreenshot(imageFile); });
        return;
    }

    qCDebug(GstVideoReceiverLog) << "taking screenshot" << _redactedUri();

    // FIXME: record screenshot here
    emit onTakeScreenshotComplete(STATUS_NOT_IMPLEMENTED);
}

void GstVideoReceiver::_onWatchdogTick()
{
    if (!_pipeline) {
        return;
    }

    if (++_statsTickCounter >= 10) {
        _statsTickCounter = 0;
        if (const HwBuffers::PathStats hwStats = HwBuffers::formatPathStats(false); hwStats.totalDelivered > 0) {
            qCDebug(GstVideoReceiverLog).noquote() << "HW path live" << _redactedUri() << hwStats.line;
        }
    }

    // Drain QoS updates accumulated since the last tick (see GST_MESSAGE_QOS).
    if (_qosStatsDirty.exchange(false, std::memory_order_acq_rel)) {
        _notifyDecoderStatsChanged();
    }

    const StreamWatchdog::Result result = _watchdog.check(StreamWatchdog::now(), _decoding);
    if (result.stall == StreamWatchdog::Stall::None) {
        return;
    }

    const bool sourceStalled = (result.stall == StreamWatchdog::Stall::Source);
    qCDebug(GstVideoReceiverLog) << (sourceStalled ? "Stream timeout, no frames for"
                                                   : "Video decoder timeout, no frames for")
                                 << result.silentMs << "ms" << _redactedUri();
    _dotDumper.dump(_pipeline, "pipeline-watchdog-timeout");
    emit timeout();
    _onStreamFailure(sourceStalled ? "source watchdog" : "decoder watchdog");
}

void GstVideoReceiver::_onStreamFailure(const char* reason)
{
    qCDebug(GstVideoReceiverLog) << "Stopping after" << reason << _redactedUri();
    (void) _stopPipeline();
    emit onStopComplete(STATUS_FAIL);
    _scheduleReconnect(reason);
}

void GstVideoReceiver::_scheduleReconnect(const char* reason)
{
    if (!_autoReconnect) {
        qCDebug(GstVideoReceiverLog) << "Auto-reconnect disabled — not retrying after" << reason;
        return;
    }

    const std::chrono::seconds delay = _watchdog.nextReconnectDelay();
    qCDebug(GstVideoReceiverLog) << "Scheduling reconnect #" << _watchdog.reconnectAttempts() << "in" << delay.count()
                                 << "s after" << reason << _redactedUri();
    _reconnectTimer->start(delay);
}

void GstVideoReceiver::_reconnect()
{
    if (!_autoReconnect) {
        qCDebug(GstVideoReceiverLog) << "Auto-reconnect disabled while waiting; not reconnecting" << _redactedUri();
        return;
    }

    qCDebug(GstVideoReceiverLog) << "Reconnecting (attempt" << _watchdog.reconnectAttempts() << ")" << _redactedUri();
    _startPipeline(_config);
}

void GstVideoReceiver::dumpPipelineGraph(const QString &tag)
{
    _runOnWorker([this, tag]() {
        if (!_pipeline) {
            qCDebug(GstVideoReceiverLog) << "dumpPipelineGraph: pipeline not running";
            return;
        }
        _dotDumper.snapshot(_pipeline, tag);
    });
}

GstElement *GstVideoReceiver::_makeDecoder()
{
    GstElement *decoder = gst_element_factory_make("decodebin3", nullptr);
    if (!decoder) {
        qCCritical(GstVideoReceiverLog) << "gst_element_factory_make('decodebin3') failed";
    }
    return decoder;
}

void GstVideoReceiver::_onNewSourcePad(GstPad *pad)
{
    // FIXME: check for caps - if this is not video stream (and preferably - one of these which we have to support) then simply skip it
    GstPad* teeSink = gst_element_get_static_pad(_tee, "sink");
    const GstPadLinkReturn linkResult = teeSink ? gst_pad_link(pad, teeSink) : GST_PAD_LINK_WRONG_HIERARCHY;
    gst_clear_object(&teeSink);
    if (linkResult != GST_PAD_LINK_OK) {
        qCWarning(GstVideoReceiverLog) << "Unable to link source:" << gst_pad_link_get_name(linkResult);
        return;
    }

    if (!_streaming) {
        _streaming = true;
        qCDebug(GstVideoReceiverLog) << "Streaming started" << _redactedUri();
        emit streamingChanged(true);
    }

    // Lives as long as the source pad/pipeline (removed in _resetPipelineState), so stopDecoding()
    // doesn't lose source-EOS detection.
    _removeSourceEosProbe();
    _eosProbeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, _eosProbe, this, nullptr);
    if (_eosProbeId != 0) {
        _eosProbePad = GST_PAD_CAST(gst_object_ref(pad));
    }
    if (!_videoSink) {
        return;
    }

    _dotDumper.dump(_pipeline, "pipeline-with-new-source-pad");

    _ensureVideoSinkInPipeline();

    if (!_addDecoder(_decoderValve)) {
        qCWarning(GstVideoReceiverLog) << "_addDecoder() failed";
        _shutdownDecodingBranch();
        return;
    }

    g_object_set(_decoderValve,
                 "drop", FALSE,
                 nullptr);

    qCDebug(GstVideoReceiverLog) << "Decoding started" << _redactedUri();
}

void GstVideoReceiver::_onDecoderElementAdded(GstBin* /*decodebin*/, GstBin* /*subBin*/, GstElement* element,
                                              gpointer data)
{
    GstElementFactory* factory = gst_element_get_factory(element);
    if (!factory || !gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_DECODER)) {
        return;
    }

    // Live streams: once a frame is late, the videodecoder base class advances earliest_time and drops every frame
    // after it. Turned off here, before the decoder sees data, for each decoder decodebin3 plugs, later swaps included.
    if (g_object_class_find_property(G_OBJECT_GET_CLASS(element), "qos")) {
        g_object_set(element, "qos", FALSE, nullptr);
    }

    GstPluginFeature* feature = GST_PLUGIN_FEATURE(factory);
    const QString decoderName = QString::fromUtf8(gst_plugin_feature_get_name(feature));
    QString pluginName = decoderName;
    if (GstPlugin* plugin = gst_plugin_feature_get_plugin(feature)) {
        pluginName = QString::fromUtf8(gst_plugin_get_name(plugin));
        gst_object_unref(plugin);
    }
    qCDebug(GstVideoReceiverLog) << "Decodebin3 selected codec:rank -" << pluginName << "/" << decoderName << "-"
                                 << gst_element_factory_get_klass(factory)
                                 << (GStreamer::isHardwareDecoderFactory(factory) ? "(HW)" : "(SW)") << ":"
                                 << gst_plugin_feature_get_rank(feature) << "- QoS disabled";

    auto* self = static_cast<GstVideoReceiver*>(data);
    self->_runOnWorkerForPipeline([self, decoderName]() {
        {
            QMutexLocker locker(&self->_decoderNameMutex);
            if (decoderName == self->_decoderName) {
                return;
            }
            self->_decoderName = decoderName;
        }
        self->_notifyDecoderStatsChanged();
    });
}

void GstVideoReceiver::_notifyDecoderStatsChanged()
{
    // Emitted on the receiver's own thread, where its QML consumers live.
    QMetaObject::invokeMethod(this, [this]() { emit decoderStatsChanged(); }, Qt::QueuedConnection);
}

void GstVideoReceiver::_onNewDecoderPad(GstPad *pad)
{
    qCDebug(GstVideoReceiverLog) << "_onNewDecoderPad" << _redactedUri();

    _dotDumper.dump(_pipeline, "pipeline-with-new-decoder-pad");

    (void) _addVideoSink(pad);
}

bool GstVideoReceiver::_addDecoder(GstElement *src)
{
    _decoder = _makeDecoder();
    if (!_decoder) {
        qCCritical(GstVideoReceiverLog) << "_makeDecoder() failed";
        return false;
    }

    (void) gst_object_ref(_decoder);
    (void) g_signal_connect(_decoder, "deep-element-added", G_CALLBACK(_onDecoderElementAdded), this);

    (void) gst_bin_add(GST_BIN(_pipeline), _decoder);
    (void) gst_element_sync_state_with_parent(_decoder);

    _dotDumper.dump(_pipeline, "pipeline-with-decoder");

    if (!gst_element_link(src, _decoder)) {
        qCWarning(GstVideoReceiverLog) << "Unable to link decoder";
        gst_element_set_state(_decoder, GST_STATE_NULL);
        (void) gst_element_get_state(_decoder, nullptr, nullptr, GST_CLOCK_TIME_NONE);
        (void) gst_bin_remove(GST_BIN(_pipeline), _decoder);
        gst_clear_object(&_decoder);
        return false;
    }

    GstPad *srcPad = nullptr;
    (void) gst_element_foreach_src_pad(_decoder, grabFirstSrcPad, &srcPad);

    if (srcPad) {
        _onNewDecoderPad(srcPad);
    } else {
        (void) g_signal_connect(_decoder, "pad-added", G_CALLBACK(_onNewPad), this);
    }

    gst_clear_object(&srcPad);
    return true;
}

void GstVideoReceiver::_ensureVideoSinkInPipeline()
{
    if (!_videoSink || !_pipeline) {
        return;
    }

    GstObject *parent = gst_element_get_parent(_videoSink);
    if (parent) {
        gst_object_unref(parent);
        return;
    }

    g_object_set(_videoSink, "sync", !_config.lowLatency, NULL);

    // The sink is never floating here, so the bin takes its own reference; removal drops only that one.
    (void) gst_bin_add(GST_BIN(_pipeline), _videoSink);

    // PAUSED (not READY) triggers downstream caps negotiation before source data arrives.
    (void) gst_element_set_state(_videoSink, GST_STATE_PAUSED);
}

bool GstVideoReceiver::_addVideoSink(GstPad *pad)
{
    _ensureVideoSinkInPipeline();

    GstPad *sinkPad = gst_element_get_static_pad(_videoSink, "sink");
    GstPadLinkReturn linkRet = sinkPad ? gst_pad_link(pad, sinkPad) : GST_PAD_LINK_WRONG_HIERARCHY;
    gst_clear_object(&sinkPad);
    if (linkRet != GST_PAD_LINK_OK) {
        qCWarning(GstVideoReceiverLog) << "Unable to link decoder pad to video sink:" << gst_pad_link_get_name(linkRet);

        // _ensureVideoSinkInPipeline() added it before linking; detach for the next retry.
        GstObject *parent = gst_element_get_parent(_videoSink);
        if (parent) {
            (void) gst_element_set_state(_videoSink, GST_STATE_NULL);
            (void) gst_element_get_state(_videoSink, nullptr, nullptr, GST_CLOCK_TIME_NONE);
            (void) gst_bin_remove(GST_BIN(_pipeline), _videoSink);
            gst_clear_object(&parent);
        }
        return false;
    }

    (void) gst_element_sync_state_with_parent(_videoSink);

    _dotDumper.dump(_pipeline, "pipeline-with-videosink");

    return true;
}

void GstVideoReceiver::_noteTeeFrame()
{
    const quint64 sourceFrames = _watchdog.noteSourceFrame(StreamWatchdog::now());
    if (sourceFrames == 1) {
        qCDebug(GstVideoReceiverLog).noquote() << "Source receiving frames (tee):" << _redactedUri();
    } else if ((sourceFrames % 300) == 0) {
        qCDebug(GstVideoReceiverLog).noquote()
            << "Source flow: teeFrames=" << sourceFrames << "decoding=" << _decoding << _redactedUri();
    }
}

void GstVideoReceiver::_noteVideoSinkFrame()
{
    if (_watchdog.noteSinkFrame(StreamWatchdog::now())) {
        _runOnWorkerForPipeline([this]() { _onFirstVideoFrame(); });
    }
}

void GstVideoReceiver::_onFirstVideoFrame()
{
    if (!_videoSink || _decoding) {
        return;
    }
    _decoding = true;
    qCDebug(GstVideoReceiverLog) << "Decoding started";
    emit decodingChanged(true);
}

void GstVideoReceiver::_shutdownDecodingBranch()
{
    if (_decoder) {
        GstObject *parent = gst_element_get_parent(_decoder);
        if (parent) {
            (void) gst_bin_remove(GST_BIN(parent), _decoder);
            (void) gst_element_set_state(_decoder, GST_STATE_NULL);
            (void) gst_element_get_state(_decoder, nullptr, nullptr, GST_CLOCK_TIME_NONE);
            gst_clear_object(&parent);
        }

        gst_clear_object(&_decoder);
    }

    if (_videoSinkProbeId != 0 && _videoSink) {
        GstPad *sinkpad = gst_element_get_static_pad(_videoSink, "sink");
        if (sinkpad) {
            gst_pad_remove_probe(sinkpad, _videoSinkProbeId);
            gst_clear_object(&sinkpad);
        }
    }
    _videoSinkProbeId = 0;

    _watchdog.clearSinkFrames();

    if (_videoSink) {
        GstObject *parent = gst_element_get_parent(_videoSink);
        if (parent) {
            // Drops only the bin's own reference; _videoSink keeps ours until the clear below.
            (void) gst_bin_remove(GST_BIN(parent), _videoSink);
            (void) gst_element_set_state(_videoSink, GST_STATE_NULL);
            (void) gst_element_get_state(_videoSink, nullptr, nullptr, GST_CLOCK_TIME_NONE);
            gst_clear_object(&parent);
        }
        gst_clear_object(&_videoSink);
    }

    if (_decoding.exchange(false)) {
        qCDebug(GstVideoReceiverLog) << "Decoding stopped";
        emit decodingChanged(false);
    }

    _dotDumper.dump(_pipeline, "pipeline-decoding-stopped");
}

void GstVideoReceiver::_removeSourceEosProbe()
{
    if (_eosProbeId != 0 && _eosProbePad) {
        gst_pad_remove_probe(_eosProbePad, _eosProbeId);
    }
    _eosProbeId = 0;
    gst_clear_object(&_eosProbePad);
}

GstBusSyncReply GstVideoReceiver::_onBusSyncMessage(GstBus* bus, GstMessage* message, gpointer user_data)
{
    // The context bridges answer NEED_CONTEXT synchronously and consume (unref) the requests they handle.
    if (HwBuffers::onBusSyncMessage(bus, message, nullptr) == GST_BUS_DROP) {
        return GST_BUS_DROP;
    }

    auto* self = static_cast<GstVideoReceiver*>(user_data);
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR:
            self->_onBusError(message);
            break;
        case GST_MESSAGE_WARNING:
            logBusWarning(message);
            break;
        case GST_MESSAGE_EOS:
            self->_recordingBranch.onBusEos();
            break;
        case GST_MESSAGE_ELEMENT:
            self->_onBusElement(message);
            break;
        case GST_MESSAGE_STREAM_COLLECTION:
            selectVideoStreams(message);
            break;
        case GST_MESSAGE_QOS:
            self->_onBusQos(message);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            self->_onBusStateChanged(message);
            break;
        case GST_MESSAGE_LATENCY:
            self->_runOnWorkerForPipeline([self]() { (void) gst_bin_recalculate_latency(GST_BIN(self->_pipeline)); });
            // Re-prime sink-side latency tracking after the pipeline recalculation (e.g. RTSP
            // jitter-buffer reconfigure). Controllers live on the GUI thread; hop there to query.
            QMetaObject::invokeMethod(
                self,
                [self]() {
                    for (auto* c : QGCQVideoSinkController::controllersOf(self)) {
                        c->refreshLatency();
                    }
                },
                Qt::QueuedConnection);
            break;
        default:
            break;
    }

    // Every message is handled above and nothing pops this bus; queueing it would grow the bus for the
    // pipeline's lifetime. A sync handler that drops a message owns it.
    gst_message_unref(message);
    return GST_BUS_DROP;
}

void GstVideoReceiver::_onBusError(GstMessage* message)
{
    gchar* debug = nullptr;
    GError* error = nullptr;
    gst_message_parse_error(message, &error, &debug);
    const bool recoverableH265PaciError = isRecoverableH265PaciError(message, error, debug);
    const QString text = QString::fromUtf8(error ? error->message : "(no message)");
    if (debug) {
        qCDebug(GstVideoReceiverLog) << "GStreamer debug:" << debug;
    }
    g_clear_error(&error);
    g_clear_pointer(&debug, g_free);

    if (recoverableH265PaciError) {
        qCWarning(GstVideoReceiverLog) << "Ignoring unsupported H.265 RTP PACI packet from rtph265depay:" << text;
        return;
    }

    if (_recordingBranch.onBusError(message, text)) {
        return;
    }

    qCWarning(GstVideoReceiverLog) << "GStreamer error:" << text;
    // Drops cached GPU devices on a device-loss error.
    HwBuffers::dispatchBusMessage(message);
    // A failing pipeline usually posts several ERRORs; the first one's teardown makes the rest stale.
    _runOnWorkerForPipeline([this]() { _onPipelineError(); });
}

void GstVideoReceiver::_onBusElement(GstMessage* message)
{
    const GstStructure* structure = gst_message_get_structure(message);
    if (!structure) {
        return;
    }

    if (gst_structure_has_name(structure, "qgc-caps-info")) {
        gint w = 0, h = 0;
        const gchar* fmt = gst_structure_get_string(structure, "format");
        gst_structure_get_int(structure, "width", &w);
        gst_structure_get_int(structure, "height", &h);
        const QString format = QString::fromUtf8(fmt ? fmt : "");
        const QSize resolution(w, h);
        // src compared by address only on the GUI thread; never dereferenced (may be gone by then).
        void* src = GST_MESSAGE_SRC(message);
        QMetaObject::invokeMethod(
            this,
            [this, format, resolution, src]() {
                for (auto* c : QGCQVideoSinkController::controllersOf(this)) {
                    if (static_cast<const void*>(c->element()) == src) {
                        c->updateNegotiation(format, resolution);
                    }
                }
                // The sink's negotiated raw caps are always fixed, unlike the encoded caps upstream of the decoder.
                emit videoSizeChanged(resolution);
            },
            Qt::QueuedConnection);
        return;
    }

    _recordingBranch.onBusElement(message);
}

void GstVideoReceiver::_onBusQos(GstMessage* message)
{
    guint64 processed = 0, dropped = 0;
    gst_message_parse_qos_stats(message, nullptr, &processed, &dropped);

    gint64 jitter = 0;
    gdouble proportion = 0;
    gint quality = 0;
    gst_message_parse_qos_values(message, &jitter, &proportion, &quality);

    _processedFrames.store(processed, std::memory_order_relaxed);
    _droppedFrames.store(dropped, std::memory_order_relaxed);
    _currentJitterNs.store(jitter, std::memory_order_relaxed);
    _qosProportion.store(proportion, std::memory_order_relaxed);
    _qosQuality.store(quality, std::memory_order_relaxed);
    // GstBaseSink can post QOS per dropped buffer; defer the emit to the 1 Hz
    // watchdog tick so QML isn't flooded from the streaming thread.
    _qosStatsDirty.store(true, std::memory_order_release);
}

void GstVideoReceiver::_onBusStateChanged(GstMessage* message)
{
    // Only our pipeline is a GstPipeline on its own bus.
    if (!GST_IS_PIPELINE(GST_MESSAGE_SRC(message))) {
        return;
    }
    GstState oldState = GST_STATE_NULL, newState = GST_STATE_NULL;
    gst_message_parse_state_changed(message, &oldState, &newState, nullptr);
    if ((newState == GST_STATE_PLAYING) && (oldState != GST_STATE_PLAYING)) {
        _runOnWorkerForPipeline([this]() { _onPipelinePlaying(); });
    }
}

void GstVideoReceiver::_onPipelinePlaying()
{
    GstClockTime min = 0, max = 0;
    GstQuery* q = gst_query_new_latency();
    if (gst_element_query(_pipeline, q)) {
        gboolean live = FALSE;
        gst_query_parse_latency(q, &live, &min, &max);
    }
    gst_query_unref(q);
    const QString decName = decoderName();
    qCDebug(GstVideoReceiverLog).noquote() << "Pipeline PLAYING:" << _redactedUri()
                                           << "decoder:" << (decName.isEmpty() ? QStringLiteral("(pending)") : decName)
                                           << "min-latency:" << (min / 1000000) << "ms"
                                           << "max-latency:" << (max / 1000000) << "ms";
}

void GstVideoReceiver::_onPipelineError()
{
    _dotDumper.snapshot(_pipeline, QStringLiteral("pipeline-error"));

    _onStreamFailure("pipeline error");
}

void GstVideoReceiver::_onNewPad(GstElement *element, GstPad *pad, gpointer data)
{
    auto* self = static_cast<GstVideoReceiver*>(data);

    // Runs on a streaming thread; the pipeline is only changed on the worker. Data waits at the pad until the
    // worker has linked it, since an unlinked push would fail the stream.
    const gulong blockId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM, holdPad, nullptr, nullptr);
    self->_runOnWorkerForPipeline([self, blockId, padElement = GstElementRef(GST_ELEMENT(gst_object_ref(element))),
                                   newPad = GstPadRef(GST_PAD(gst_object_ref(pad)))]() {
        self->_onPadAdded(padElement.get(), newPad.get());
        gst_pad_remove_probe(newPad.get(), blockId);
    });
}

void GstVideoReceiver::_onPadAdded(GstElement* element, GstPad* pad)
{
    if (element == _source) {
        _onNewSourcePad(pad);
    } else if (element == _decoder) {
        _onNewDecoderPad(pad);
    } else {
        qCDebug(GstVideoReceiverLog) << "Ignoring a pad of a removed element";
    }
}

GstPadProbeReturn GstVideoReceiver::_teeProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Q_UNUSED(pad); Q_UNUSED(info)

    if (user_data) {
        GstVideoReceiver *pThis = static_cast<GstVideoReceiver*>(user_data);
        pThis->_noteTeeFrame();
    }

    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn GstVideoReceiver::_videoSinkProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Q_UNUSED(pad); Q_UNUSED(info)

    if (user_data) {
        static_cast<GstVideoReceiver*>(user_data)->_noteVideoSinkFrame();
    }

    return GST_PAD_PROBE_OK;
}

GstPadProbeReturn GstVideoReceiver::_eosProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Q_UNUSED(pad);

    if (!user_data || !info) {
        return GST_PAD_PROBE_OK;
    }

    const GstEvent* event = gst_pad_probe_info_get_event(info);
    if (event && (GST_EVENT_TYPE(event) == GST_EVENT_EOS)) {
        auto* self = static_cast<GstVideoReceiver*>(user_data);
        self->_runOnWorkerForPipeline([self]() { self->_onStreamFailure("end of stream"); });
    }

    return GST_PAD_PROBE_OK;
}
