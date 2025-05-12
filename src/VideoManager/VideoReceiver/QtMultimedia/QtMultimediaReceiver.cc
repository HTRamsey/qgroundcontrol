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

#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimedia/QMediaFormat>
#include <QtMultimedia/QMediaMetaData>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaRecorder>
#include <QtMultimedia/QVideoFrame>
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
    // qCDebug(QtMultimediaReceiverLog) << this;

    _captureSession->setRecorder(_mediaRecorder);
    _captureSession->setVideoFrameInput(_frameInput);

    (void) connect(_mediaPlayer, &QMediaPlayer::playingChanged, this, &QtMultimediaReceiver::streamingChanged);
    (void) connect(_mediaPlayer, &QMediaPlayer::hasVideoChanged, this, &QtMultimediaReceiver::decodingChanged);
    (void) connect(_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState newState) {
        if (newState == QMediaPlayer::PlaybackState::PlayingState) {
            _frameTimer.start();
        } else if (newState == QMediaPlayer::PlaybackState::StoppedState) {
            _frameTimer.stop();
        }
    });
    (void) connect(_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        switch (status) {
        case QMediaPlayer::MediaStatus::LoadingMedia:
            _streamDevice = _mediaPlayer->sourceDevice();
            break;
        default:
            break;
        }
    });
    (void) connect(_mediaPlayer, &QMediaPlayer::metaDataChanged, this, []() {
        /*const QMediaMetaData metaData = _mediaPlayer->metaData();
        const QVariant resolution = metaData.value(QMediaMetaData::Key::Resolution);
        const QSize videoSize = resolution.toSize();*/
    });
    (void) connect(_mediaPlayer, &QMediaPlayer::bufferProgressChanged, this, [](float filled) {
        qCDebug(QtMultimediaReceiverLog) << "Buffer Progress:" << filled;
    });
    (void) connect(_mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        switch (error) {
        case QMediaPlayer::Error::NetworkError:
            break;
        default:
            break;
        }

        qCWarning(QtMultimediaReceiverLog) << errorString;
    });

    // _mediaRecorder->setEncodingMode(QMediaRecorder::EncodingMode::AverageBitRateEncoding);
    // _mediaRecorder->setQuality(QMediaRecorder::Quality::HighQuality);
    // _mediaRecorder->setVideoBitRate()
    _mediaRecorder->setVideoFrameRate(0);
    _mediaRecorder->setVideoResolution(QSize());
    (void) connect(_mediaRecorder, &QMediaRecorder::recorderStateChanged, this, [this](QMediaRecorder::RecorderState state) {
        const bool nowRecording = (state == QMediaRecorder::RecordingState);
        if (nowRecording && !_recording) {
            _recording = true;
            emit recordingStarted(_mediaRecorder->actualLocation().toString());
            emit recordingChanged(true);
            emit onStartRecordingComplete(STATUS_OK);
        } else if (!nowRecording && _recording) {
            _recording = false;
            emit recordingChanged(false);
            emit onStopRecordingComplete(STATUS_OK);
        }
    });
    (void) connect(_mediaRecorder, &QMediaRecorder::errorOccurred, this, [this](QMediaRecorder::Error error, const QString &errorString) {
        switch (error) {
        case QMediaRecorder::Error::OutOfSpaceError:
            break;
        default:
            break;
        }

        qCWarning(QtMultimediaReceiverLog) << errorString;
    });

    _frameTimer.setSingleShot(true);
    _frameTimer.setTimerType(Qt::PreciseTimer);
    (void) connect(&_frameTimer, &QTimer::timeout, this, &QtMultimediaReceiver::timeout);
}

QtMultimediaReceiver::~QtMultimediaReceiver()
{
    // qCDebug(QtMultimediaReceiverLog) << this;
}

bool QtMultimediaReceiver::enabled()
{
#ifdef QGC_QT_STREAMING
    return true;
#else
    return false;
#endif
}

void *QtMultimediaReceiver::createVideoSink(QQuickItem *widget, QObject *parent)
{
    Q_UNUSED(parent);

    QVideoSink *videoSink = nullptr;
    if (widget) {
        QQuickVideoOutput *const videoOutput = reinterpret_cast<QQuickVideoOutput*>(widget);
        videoSink = videoOutput->videoSink();
    }

    return videoSink;
}

void QtMultimediaReceiver::releaseVideoSink(void *sink)
{
    /*if (!sink) {
        return;
    }

    QVideoSink* const videoSink = reinterpret_cast<QVideoSink*>(sink);
    videoSink->deleteLater();*/
}

VideoReceiver *QtMultimediaReceiver::createVideoReceiver(QObject *parent)
{
    Q_UNUSED(parent);
    return new QtMultimediaReceiver(nullptr);
}

void QtMultimediaReceiver::start(uint32_t timeout)
{
    if (_mediaPlayer->isPlaying()) {
        emit onStartComplete(STATUS_INVALID_STATE);
        return;
    }

    if (_uri.isEmpty()) {
        emit onStartComplete(STATUS_INVALID_URL);
        return;
    }

    _mediaPlayer->setSource(QUrl::fromUserInput(_uri));

    _frameTimer.setInterval(timeout);

    // QAbstractVideoBuffer *buffer = _videoSink->videoFrame()->videoBuffer();

    /*if (!_mediaPlayer->hasVideo()) {
        emit onStartComplete(STATUS_FAIL);
    }*/

    _mediaPlayer->play();

    emit onStartComplete(STATUS_OK);
}

void QtMultimediaReceiver::stop()
{
    if (!_mediaPlayer->isPlaying()) {
        emit onStopComplete(STATUS_INVALID_STATE);
        return;
    }

    if (_mediaPlayer->source().isEmpty()) {
        emit onStopComplete(STATUS_FAIL);
        return;
    }

    _mediaPlayer->stop();

    emit onStopComplete(STATUS_OK);
}

void QtMultimediaReceiver::startDecoding(void *sink)
{
    if (!sink) {
        emit onStartDecodingComplete(STATUS_FAIL);
        return;
    }

    if (_videoSink) {
        qCWarning(QtMultimediaReceiverLog) << "VideoSink is already set";
    }

    if (_videoSizeUpdater) {
        qCWarning(QtMultimediaReceiverLog) << "VideoSizeConnection is already set";
    }

    _videoSink = reinterpret_cast<QVideoSink*>(sink);
    _videoSizeUpdater = connect(_videoSink, &QVideoSink::videoSizeChanged, this, [this]() {
        emit videoSizeChanged(_videoSink->videoSize());
    });
    _videoFrameUpdater = connect(_videoSink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (frame.isValid()) {
            _frameTimer.start();
            // _mediaPlayer->setVideoSink(_videoSink);
            if (_recording) {
                _frameInput->sendVideoFrame(frame);
            }
        }
    });
    _rhi = _videoSink->rhi();
    _videoSink->setSubtitleText("");

    _mediaPlayer->setVideoSink(_videoSink);

    emit onStartDecodingComplete(STATUS_OK);
}

void QtMultimediaReceiver::stopDecoding()
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_videoSink) {
        qCWarning(QtMultimediaReceiverLog) << "VideoSink is NULL";
        emit onStartDecodingComplete(STATUS_INVALID_STATE);
        return;
    }

    (void) disconnect(_videoSizeUpdater);
    _mediaPlayer->setVideoSink(nullptr);
    _videoSink = nullptr;

    qCDebug(QtMultimediaReceiverLog) << "Stopped Decoding";

    emit onStopDecodingComplete(STATUS_OK);
}

void QtMultimediaReceiver::startRecording(const QString &videoFile, FILE_FORMAT format)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_mediaRecorder->isAvailable()) {
        qCWarning(QtMultimediaReceiverLog) << "Recording Unavailable";
        emit onStartRecordingComplete(STATUS_FAIL);
        return;
    }

    switch (format) {
    case FILE_FORMAT_MKV:
        _mediaRecorder->setMediaFormat(QMediaFormat::FileFormat::Matroska);
        break;
    case FILE_FORMAT_MOV:
        _mediaRecorder->setMediaFormat(QMediaFormat::FileFormat::QuickTime);
        break;
    case FILE_FORMAT_MP4:
        _mediaRecorder->setMediaFormat(QMediaFormat::FileFormat::MPEG4);
        break;
    default:
        // QMediaFormat::AVI, WMV, Ogg, WebM
        _mediaRecorder->setMediaFormat(QMediaFormat::FileFormat::UnspecifiedFormat);
        break;
    }

    _mediaRecorder->setOutputLocation(QUrl::fromLocalFile(videoFile));
    _recordingOutput = _mediaRecorder->outputLocation().toLocalFile();
    _mediaRecorder->record();

    qCDebug(QtMultimediaReceiverLog) << "Recording";

    emit onStartRecordingComplete(STATUS_OK);
}

void QtMultimediaReceiver::stopRecording()
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    _mediaRecorder->stop();

    qCDebug(QtMultimediaReceiverLog) << "Stopped Recording";

    emit onStopRecordingComplete(STATUS_OK);
}

void QtMultimediaReceiver::takeScreenshot(const QString &imageFile)
{
    qCDebug(QtMultimediaReceiverLog) << Q_FUNC_INFO;

    if (!_videoSink) {
        qCWarning(QtMultimediaReceiverLog) << "Video Sink is NULL";
        emit onTakeScreenshotComplete(STATUS_FAIL);
        return;
    }

    const QVideoFrame frame = _videoSink->videoFrame();
    if (!frame.isValid() || !frame.isReadable()) {
        qCWarning(QtMultimediaReceiverLog) << "Screenshot Frame is Invalid";
        emit onTakeScreenshotComplete(STATUS_FAIL);
        return;
    }

    // const QVideoFrameFormat frameFormat = frame.surfaceFormat();
    // const QImage frameImage = frame.toImage();

    _videoOutput = reinterpret_cast<QQuickVideoOutput*>(_mediaPlayer->videoOutput());
    if (!_videoOutput) {
        emit onTakeScreenshotComplete(STATUS_FAIL);
        return;
    }

    const QSize targetSize = _mediaRecorder->videoResolution();
    QSharedPointer<QQuickItemGrabResult> screenshot = _videoOutput->grabToImage(targetSize);
    (void) connect(&screenshot, &QQuickItemGrabResult::ready, this, [this, screenshot, imageFile]() {
        screenshot->saveToFile(imageFile);
        emit onTakeScreenshotComplete(STATUS_OK);
    }

    qCDebug(QtMultimediaReceiverLog) << "Screenshot";

    emit onTakeScreenshotComplete(STATUS_NOT_IMPLEMENTED);
}
