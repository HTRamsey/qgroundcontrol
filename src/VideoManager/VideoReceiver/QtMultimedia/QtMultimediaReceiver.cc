// QtMultimediaReceiver.cpp
/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QtMultimediaReceiver.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimedia/QMediaFormat>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaRecorder>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimedia/QVideoFrameInput>
#include <QtMultimedia/QVideoSink>
#include <QtMultimediaQuick/private/qquickvideooutput_p.h>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickItemGrabResult>

QGC_LOGGING_CATEGORY(QtMultimediaReceiverLog, "qgc.videomanager.videoreceiver.qtmultimedia.qtmultimediareceiver")

QtMultimediaReceiver::QtMultimediaReceiver(QObject *parent)
    : VideoReceiver(parent)
    , _mediaPlayer(new QMediaPlayer(this))
    , _captureSession(new QMediaCaptureSession(this))
    , _mediaRecorder(new QMediaRecorder(this))
    , _frameInput(new QVideoFrameInput(this))
{
    // Wire up capture pipeline for recording frames we inject.
    _captureSession->setRecorder(_mediaRecorder);
    _captureSession->setVideoFrameInput(_frameInput);   // Qt 6.8+

    // Playback → app signals
    connect(_mediaPlayer, &QMediaPlayer::playingChanged, this, &QtMultimediaReceiver::streamingChanged);
    connect(_mediaPlayer, &QMediaPlayer::hasVideoChanged, this, &QtMultimediaReceiver::decodingChanged);
    connect(_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState s) {
        if (s == QMediaPlayer::PlayingState) _frameTimer.start();
        else if (s == QMediaPlayer::StoppedState) _frameTimer.stop();
    });
    connect(_mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString &err) {
        qCWarning(QtMultimediaReceiverLog) << "Playback error:" << err;
    });

    // Recorder state and errors
    connect(_mediaRecorder, &QMediaRecorder::recorderStateChanged, this, &QtMultimediaReceiver::_onRecorderStateChanged);
    connect(_mediaRecorder, &QMediaRecorder::errorOccurred, this, &QtMultimediaReceiver::_onRecorderError);

    // Watchdog: no frames within timeout → emit timeout and stop()
    _frameTimer.setSingleShot(true);
    _frameTimer.setTimerType(Qt::PreciseTimer);
    connect(&_frameTimer, &QTimer::timeout, this, [this]() {
        emit timeout();
        stop();
    });
}

QtMultimediaReceiver::~QtMultimediaReceiver() = default;

bool QtMultimediaReceiver::enabled()
{
#ifdef QGC_QT_STREAMING
    return true;
#else
    return false;
#endif
}

void *QtMultimediaReceiver::createVideoSink(QObject * /*parent*/, QQuickItem *widget)
{
    if (!widget) return nullptr;
    auto *videoOutput = qobject_cast<QQuickVideoOutput *>(widget);
    return videoOutput ? videoOutput->videoSink() : nullptr;
}

void QtMultimediaReceiver::releaseVideoSink(void * /*sink*/)
{
    // QQuickVideoOutput owns its QVideoSink; nothing to do.
}

VideoReceiver *QtMultimediaReceiver::createVideoReceiver(QObject * /*parent*/)
{
    return new QtMultimediaReceiver(nullptr);
}

QUrl QtMultimediaReceiver::_normalizeForQtBackend(const QString &in)
{
    if (in.isEmpty()) return {};
    // Pass-through for common cases
    if (in.startsWith("rtsp://", Qt::CaseInsensitive) ||
        in.startsWith("tcp://",  Qt::CaseInsensitive) ||
        QFileInfo(in).exists()) {
        return QUrl::fromUserInput(in);
    }

    // Best-effort mappings (still may need SDP for RTP/UDP cases)
    if (in.startsWith("udp265://", Qt::CaseInsensitive)) {
        QString s = in;
        s.replace(QRegularExpression("^udp265://", QRegularExpression::CaseInsensitiveOption), "udp://");
        return QUrl::fromUserInput(s);
    }
    if (in.startsWith("mpegts://", Qt::CaseInsensitive)) {
        QString s = in;
        s.replace(0, 9, "udp://"); // mpegts:// -> udp://
        return QUrl::fromUserInput(s);
    }
    if (in.startsWith("udp://", Qt::CaseInsensitive)) {
        return QUrl::fromUserInput(in);
    }

    return QUrl::fromUserInput(in);
}

void QtMultimediaReceiver::start(uint32_t timeout)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (_mediaPlayer->isPlaying()) {
        emit onStartComplete(STATUS_INVALID_STATE);
        return;
    }
    if (_uri.isEmpty()) {
        emit onStartComplete(STATUS_INVALID_URL);
        return;
    }

    _frameTimer.setInterval(static_cast<int>(timeout));
    _frameTimer.start();

    _mediaPlayer->setSource(_normalizeForQtBackend(_uri));
    _mediaPlayer->play();

    emit onStartComplete(STATUS_OK);
}

void QtMultimediaReceiver::stop()
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_mediaPlayer->isPlaying()) {
        emit onStopComplete(STATUS_INVALID_STATE);
        return;
    }

    _mediaPlayer->stop();
    emit onStopComplete(STATUS_OK);
}

void QtMultimediaReceiver::startDecoding(void *sink)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!sink) {
        qCCritical(QtMultimediaReceiverLog) << "VideoSink is NULL";
        emit onStartDecodingComplete(STATUS_FAIL);
        return;
    }

    if (_videoSink) {
        qCWarning(QtMultimediaReceiverLog) << "VideoSink already set";
    }

    _videoSink = reinterpret_cast<QVideoSink *>(sink);

    // Size updates
    _videoSizeUpdater = connect(_videoSink, &QVideoSink::videoSizeChanged, this, [this]() {
        emit videoSizeChanged(_videoSink->videoSize());
    });

    // New frames: restart watchdog and feed recorder (if active)
    _videoFrameUpdater = connect(_videoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &f) {
        if (!f.isValid()) return;
        _frameTimer.start();
        if (_recording) {
            _frameInput->sendVideoFrame(f);
        }
    });

    _mediaPlayer->setVideoSink(_videoSink);

    emit onStartDecodingComplete(STATUS_OK);
}

void QtMultimediaReceiver::stopDecoding()
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_videoSink) {
        emit onStopDecodingComplete(STATUS_INVALID_STATE);
        return;
    }

    disconnect(_videoSizeUpdater);
    disconnect(_videoFrameUpdater);
    _mediaPlayer->setVideoSink(nullptr);
    _videoSink = nullptr;

    emit onStopDecodingComplete(STATUS_OK);
}

void QtMultimediaReceiver::startRecording(const QString &videoFile, FILE_FORMAT format)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_mediaRecorder->isAvailable()) {
        qCWarning(QtMultimediaReceiverLog) << "Recording unavailable";
        emit onStartRecordingComplete(STATUS_FAIL);
        return;
    }

    switch (format) {
    case FILE_FORMAT_MKV: _mediaRecorder->setMediaFormat(QMediaFormat::Matroska); break;
    case FILE_FORMAT_MOV: _mediaRecorder->setMediaFormat(QMediaFormat::QuickTime); break;
    case FILE_FORMAT_MP4: _mediaRecorder->setMediaFormat(QMediaFormat::MPEG4);    break;
    default:              _mediaRecorder->setMediaFormat(QMediaFormat::UnspecifiedFormat); break;
    }

    _mediaRecorder->setOutputLocation(QUrl::fromLocalFile(videoFile));
    _mediaRecorder->record();
    // Completion/recordingStarted signals are emitted from _onRecorderStateChanged()
}

void QtMultimediaReceiver::stopRecording()
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_recording) {
        emit onStopRecordingComplete(STATUS_INVALID_STATE);
        return;
    }
    _mediaRecorder->stop();
    // Completion emitted from _onRecorderStateChanged()
}

void QtMultimediaReceiver::takeScreenshot(const QString &imageFile)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_widget) {
        qCWarning(QtMultimediaReceiverLog) << "Widget is NULL";
        emit onTakeScreenshotComplete(STATUS_FAIL);
        return;
    }

    auto *vo = qobject_cast<QQuickVideoOutput *>(_widget);
    if (!vo) {
        qCWarning(QtMultimediaReceiverLog) << "Widget is not QQuickVideoOutput";
        emit onTakeScreenshotComplete(STATUS_FAIL);
        return;
    }

    QSharedPointer<QQuickItemGrabResult> grab = vo->grabToImage();
    connect(grab.data(), &QQuickItemGrabResult::ready, this, [this, grab, imageFile]() {
        grab->saveToFile(imageFile);
        emit onTakeScreenshotComplete(STATUS_OK);
    });
}

/*=========== Recorder callbacks ===========*/

void QtMultimediaReceiver::_onRecorderStateChanged()
{
    const bool nowRecording = (_mediaRecorder->recorderState() == QMediaRecorder::RecordingState);

    if (nowRecording && !_recording) {
        _recording = true;
        _recordingOutput = _mediaRecorder->actualLocation().toLocalFile();
        emit recordingStarted(_recordingOutput);
        emit recordingChanged(true);
        emit onStartRecordingComplete(STATUS_OK);
    } else if (!nowRecording && _recording) {
        _recording = false;
        emit recordingChanged(false);
        emit onStopRecordingComplete(STATUS_OK);
    }
}

void QtMultimediaReceiver::_onRecorderError()
{
    qCWarning(QtMultimediaReceiverLog) << "Recorder error:" << _mediaRecorder->errorString();
    if (!_recording) {
        emit onStartRecordingComplete(STATUS_FAIL);
    } else {
        _recording = false;
        emit recordingChanged(false);
        emit onStopRecordingComplete(STATUS_FAIL);
    }
}
