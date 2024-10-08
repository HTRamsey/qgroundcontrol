/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

/**
 * @file
 *   @brief QGC Video Receiver
 *   @author Gus Grubba <gus@auterion.com>
 */

// Our pipeline look like this:
//
//              +-->queue-->_decoderValve[-->_decoder-->_videoSink]
//              |
// _source-->_tee
//              |
//              +-->queue-->_recorderValve[-->_fileSink]
//

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include <gst/gst.h>

#include "VideoReceiver.h"

class QTimer;

Q_DECLARE_LOGGING_CATEGORY(GstVideoReceiverLog)

class GstVideoWorker : public QThread
{
    Q_OBJECT

    typedef std::function<void()> Task;
public:
    explicit GstVideoWorker(QObject *parent = nullptr);
    ~GstVideoWorker();
    bool needDispatch();
    void dispatch(Task t);
    void shutdown();

private:
    void run() override;

    QQueue<Task> _taskQueue;
    QWaitCondition _taskQueueUpdate;
    QMutex _taskQueueSync;
    bool _shutdown = false;
};

/*===========================================================================*/

class GstVideoReceiver : public VideoReceiver
{
    Q_OBJECT

public:
    explicit GstVideoReceiver(QObject *parent = nullptr);
    ~GstVideoReceiver();

public slots:
    void start(const QString &uri, unsigned timeout, int buffer = 0) override;
    void stop() override;
    void startDecoding(void *sink) override;
    void stopDecoding() override;
    void startRecording(const QString &videoFile, FILE_FORMAT format) override;
    void stopRecording() override;
    void takeScreenshot(const QString &imageFile) override;

private slots:
    void _watchdog();
    void _handleEOS();

private:
    GstElement *_makeSource(const QString &uri);
    GstElement *_makeDecoder(GstCaps *caps = nullptr, GstElement *videoSink = nullptr);
    GstElement *_makeFileSink(const QString &videoFile, FILE_FORMAT format);

    bool _addDecoder(GstElement *src);
    bool _addVideoSink(GstPad *pad);
    bool _needDispatch();
    bool _unlinkBranch(GstElement *from);
    void _dispatchSignal(std::function<void()> emitter);
    void _noteEndOfStream();
    void _noteTeeFrame();
    void _noteVideoSinkFrame();
    void _onNewDecoderPad(GstPad *pad);
    void _onNewSourcePad(GstPad *pad);
    void _shutdownDecodingBranch();
    void _shutdownRecordingBranch();

    static gboolean _filterParserCaps(GstElement *bin, GstPad *pad, GstElement *element, GstQuery *query, gpointer data);
    static gboolean _onBusMessage(GstBus *bus, GstMessage *message, gpointer user_data);
    static gboolean _padProbe(GstElement *element, GstPad *pad, gpointer user_data);
    static GstPadProbeReturn _eosProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _keyframeWatch(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _teeProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static GstPadProbeReturn _videoSinkProbe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    static void _linkPad(GstElement *element, GstPad *pad, gpointer data);
    static void _onNewPad(GstElement *element, GstPad *pad, gpointer data);
    static void _wrapWithGhostPad(GstElement *element, GstPad *pad, gpointer data);

    bool _decoding = false;
    bool _endOfStream = false;
    bool _recording = false;
    bool _removingDecoder = false;
    bool _removingRecorder = false;
    bool _resetVideoSink = true;
    bool _streaming = false;
    GstElement *_decoder = nullptr;
    GstElement *_decoderValve = nullptr;
    GstElement *_fileSink = nullptr;
    GstElement *_pipeline = nullptr;
    GstElement *_recorderValve = nullptr;
    GstElement *_source = nullptr;
    GstElement *_tee = nullptr;
    GstElement *_videoSink = nullptr;
    gulong _teeProbeId = 0;
    gulong _videoSinkProbeId = 0;
    int _buffer = 0;
    qint64 _lastSourceFrameTime = 0;
    qint64 _lastVideoFrameTime = 0;
    QString _uri;
    uint32_t _signalDepth = 0;
    uint64_t _udpReconnect_us = 5000000;
    unsigned _timeout = 0;
    QTimer *_watchdogTimer = nullptr;
    GstVideoWorker *_slotHandler = nullptr;

    static constexpr const char *_kFileMux[FILE_FORMAT_MAX - FILE_FORMAT_MIN] = {
        "matroskamux",
        "qtmux",
        "mp4mux"
    };
};

void *createVideoSink(void *widget);

void initializeVideoReceiver(int argc, char *argv[], int debuglevel);
