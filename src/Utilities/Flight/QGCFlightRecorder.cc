#include "QGCFlightRecorder.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QTextStream>

Q_LOGGING_CATEGORY(QGCFlightRecorderLog, "Utilities.QGCFlightRecorder")

QGCFlightRecorder::QGCFlightRecorder(QObject *parent)
    : QObject(parent)
{
    _playbackTimer.setTimerType(Qt::PreciseTimer);
    connect(&_playbackTimer, &QTimer::timeout, this, &QGCFlightRecorder::_playbackTick);
}

QGCFlightRecorder::~QGCFlightRecorder()
{
    if (_recording) {
        stopRecording();
    }
    if (_playing) {
        stopPlayback();
    }
}

bool QGCFlightRecorder::startRecording(const QString &filename)
{
    if (_recording) {
        qCWarning(QGCFlightRecorderLog) << "Already recording";
        return false;
    }

    _file.setFileName(filename);
    if (!_file.open(QIODevice::WriteOnly)) {
        emit error(QStringLiteral("Failed to open file for recording: %1").arg(filename));
        return false;
    }

    _stream.setDevice(&_file);
    _stream.setVersion(QDataStream::Qt_6_0);

    _records.clear();
    _metadata = RecordingMetadata();
    _metadata.filename = filename;
    _metadata.startTime = QDateTime::currentDateTime();
    _recordingStartTime = QDateTime::currentMSecsSinceEpoch();

    if (!_writeHeader()) {
        _file.close();
        emit error(QStringLiteral("Failed to write recording header"));
        return false;
    }

    _recording = true;
    emit recordingChanged(true);
    qCInfo(QGCFlightRecorderLog) << "Started recording to:" << filename;

    return true;
}

void QGCFlightRecorder::stopRecording()
{
    if (!_recording) {
        return;
    }

    _metadata.endTime = QDateTime::currentDateTime();
    _metadata.duration = QDateTime::currentMSecsSinceEpoch() - _recordingStartTime;
    _metadata.recordCount = _records.size();

    // Update header with final metadata
    _file.seek(0);
    _writeHeader();

    _file.close();
    _recording = false;
    _duration = _metadata.duration;

    emit recordingChanged(false);
    emit durationChanged(_duration);
    qCInfo(QGCFlightRecorderLog) << "Stopped recording. Duration:" << _duration << "ms, Records:" << _records.size();
}

void QGCFlightRecorder::record(const QString &channel, const QVariant &data)
{
    if (!_recording) {
        return;
    }

    DataPoint point;
    point.timestamp = QDateTime::currentMSecsSinceEpoch() - _recordingStartTime;
    point.channel = channel;
    point.data = data;

    _records.append(point);
    _writeRecord(point);

    if (!_metadata.channels.contains(channel)) {
        _metadata.channels.append(channel);
    }

    emit recordCountChanged(_records.size());
}

void QGCFlightRecorder::recordPosition(const QGeoCoordinate &position, double altitude)
{
    QVariantMap posData;
    posData[QStringLiteral("latitude")] = position.latitude();
    posData[QStringLiteral("longitude")] = position.longitude();
    posData[QStringLiteral("altitude")] = altitude > 0 ? altitude : position.altitude();
    record(QStringLiteral("position"), posData);
}

void QGCFlightRecorder::recordAttitude(double roll, double pitch, double yaw)
{
    QVariantMap attData;
    attData[QStringLiteral("roll")] = roll;
    attData[QStringLiteral("pitch")] = pitch;
    attData[QStringLiteral("yaw")] = yaw;
    record(QStringLiteral("attitude"), attData);
}

void QGCFlightRecorder::setMetadata(const QString &key, const QVariant &value)
{
    _metadata.custom[key] = value;
}

bool QGCFlightRecorder::loadRecording(const QString &filename)
{
    if (_recording || _playing) {
        qCWarning(QGCFlightRecorderLog) << "Cannot load while recording or playing";
        return false;
    }

    _file.setFileName(filename);
    if (!_file.open(QIODevice::ReadOnly)) {
        emit error(QStringLiteral("Failed to open file: %1").arg(filename));
        return false;
    }

    _stream.setDevice(&_file);
    _stream.setVersion(QDataStream::Qt_6_0);

    if (!_readHeader()) {
        _file.close();
        emit error(QStringLiteral("Invalid recording file format"));
        return false;
    }

    _records.clear();

    while (!_stream.atEnd()) {
        DataPoint point;
        _stream >> point.timestamp >> point.channel >> point.data;
        _records.append(point);
    }

    _file.close();

    _metadata.filename = filename;
    _metadata.recordCount = _records.size();
    _duration = _records.isEmpty() ? 0 : _records.last().timestamp;
    _position = 0;
    _currentPlaybackIndex = 0;

    emit durationChanged(_duration);
    emit recordCountChanged(_records.size());
    qCInfo(QGCFlightRecorderLog) << "Loaded recording:" << filename << "Duration:" << _duration << "ms, Records:" << _records.size();

    return true;
}

void QGCFlightRecorder::startPlayback()
{
    if (_playing || _records.isEmpty()) {
        return;
    }

    _playing = true;
    _playbackStartTime = QDateTime::currentMSecsSinceEpoch();
    _playbackStartPosition = _position;
    _playbackTimer.start(10);  // 100Hz update rate

    emit playingChanged(true);
    qCInfo(QGCFlightRecorderLog) << "Started playback at" << _playbackSpeed << "x speed";
}

void QGCFlightRecorder::pausePlayback()
{
    if (!_playing) {
        return;
    }

    _playbackTimer.stop();
    _playing = false;
    emit playingChanged(false);
    qCDebug(QGCFlightRecorderLog) << "Paused playback at position:" << _position << "ms";
}

void QGCFlightRecorder::stopPlayback()
{
    pausePlayback();
    _position = 0;
    _currentPlaybackIndex = 0;
    emit positionChanged(0);
}

void QGCFlightRecorder::seekTo(qint64 position)
{
    if (position < 0) {
        position = 0;
    }
    if (position > _duration) {
        position = _duration;
    }

    _position = position;
    _playbackStartPosition = position;
    _playbackStartTime = QDateTime::currentMSecsSinceEpoch();

    // Find the right playback index
    _currentPlaybackIndex = 0;
    for (int i = 0; i < _records.size(); ++i) {
        if (_records[i].timestamp >= position) {
            _currentPlaybackIndex = i;
            break;
        }
        _currentPlaybackIndex = i + 1;
    }

    emit positionChanged(_position);
}

void QGCFlightRecorder::seekToPercent(double percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    seekTo(static_cast<qint64>(_duration * percent / 100.0));
}

void QGCFlightRecorder::setPlaybackSpeed(double speed)
{
    if (speed <= 0) {
        speed = 0.1;
    }
    if (speed > 16.0) {
        speed = 16.0;
    }

    if (qFuzzyCompare(_playbackSpeed, speed)) {
        return;
    }

    // Adjust playback timing for new speed
    if (_playing) {
        _playbackStartPosition = _position;
        _playbackStartTime = QDateTime::currentMSecsSinceEpoch();
    }

    _playbackSpeed = speed;
    emit playbackSpeedChanged(speed);
}

double QGCFlightRecorder::progressPercent() const
{
    if (_duration == 0) {
        return 0;
    }
    return static_cast<double>(_position) / static_cast<double>(_duration) * 100.0;
}

QStringList QGCFlightRecorder::channels() const
{
    QSet<QString> channelSet;
    for (const DataPoint &point : _records) {
        channelSet.insert(point.channel);
    }
    return channelSet.values();
}

QList<QGCFlightRecorder::DataPoint> QGCFlightRecorder::recordsForChannel(const QString &channel) const
{
    QList<DataPoint> result;
    for (const DataPoint &point : _records) {
        if (point.channel == channel) {
            result.append(point);
        }
    }
    return result;
}

QGCFlightRecorder::DataPoint QGCFlightRecorder::recordAt(int index) const
{
    if (index >= 0 && index < _records.size()) {
        return _records[index];
    }
    return {};
}

bool QGCFlightRecorder::exportToCsv(const QString &filename, const QStringList &channelsToExport) const
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "timestamp,channel,data\n";

    for (const DataPoint &point : _records) {
        if (!channelsToExport.isEmpty() && !channelsToExport.contains(point.channel)) {
            continue;
        }

        QString dataStr;
        if (point.data.typeId() == QMetaType::QVariantMap) {
            const QVariantMap map = point.data.toMap();
            QStringList parts;
            for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                parts.append(QStringLiteral("%1=%2").arg(it.key(), it.value().toString()));
            }
            dataStr = parts.join(';');
        } else {
            dataStr = point.data.toString();
        }

        out << point.timestamp << "," << point.channel << ",\"" << dataStr << "\"\n";
    }

    file.close();
    qCInfo(QGCFlightRecorderLog) << "Exported to CSV:" << filename;
    return true;
}

bool QGCFlightRecorder::exportToJson(const QString &filename) const
{
    QJsonObject root;
    root[QStringLiteral("version")] = kVersion;
    root[QStringLiteral("startTime")] = _metadata.startTime.toString(Qt::ISODate);
    root[QStringLiteral("endTime")] = _metadata.endTime.toString(Qt::ISODate);
    root[QStringLiteral("duration")] = _duration;

    QJsonArray records;
    for (const DataPoint &point : _records) {
        QJsonObject record;
        record[QStringLiteral("t")] = point.timestamp;
        record[QStringLiteral("c")] = point.channel;
        record[QStringLiteral("d")] = QJsonValue::fromVariant(point.data);
        records.append(record);
    }
    root[QStringLiteral("records")] = records;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.close();

    qCInfo(QGCFlightRecorderLog) << "Exported to JSON:" << filename;
    return true;
}

void QGCFlightRecorder::_playbackTick()
{
    if (!_playing || _records.isEmpty()) {
        return;
    }

    const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - _playbackStartTime;
    _position = _playbackStartPosition + static_cast<qint64>(elapsed * _playbackSpeed);

    if (_position >= _duration) {
        _position = _duration;
        emit positionChanged(_position);
        pausePlayback();
        emit playbackFinished();
        return;
    }

    emit positionChanged(_position);

    // Emit all data points up to current position
    while (_currentPlaybackIndex < _records.size()) {
        const DataPoint &point = _records[_currentPlaybackIndex];
        if (point.timestamp > _position) {
            break;
        }

        emit dataPlayed(point.channel, point.data, point.timestamp);
        ++_currentPlaybackIndex;
    }
}

bool QGCFlightRecorder::_writeHeader()
{
    _stream << kMagicNumber << kVersion;
    _stream << _metadata.startTime << _metadata.endTime << _metadata.duration;
    _stream << _metadata.channels << _metadata.custom;
    return _stream.status() == QDataStream::Ok;
}

bool QGCFlightRecorder::_readHeader()
{
    quint32 magic;
    quint16 version;

    _stream >> magic >> version;

    if (magic != kMagicNumber) {
        qCWarning(QGCFlightRecorderLog) << "Invalid magic number";
        return false;
    }

    if (version != kVersion) {
        qCWarning(QGCFlightRecorderLog) << "Unsupported version:" << version;
        return false;
    }

    _stream >> _metadata.startTime >> _metadata.endTime >> _metadata.duration;
    _stream >> _metadata.channels >> _metadata.custom;

    return _stream.status() == QDataStream::Ok;
}

void QGCFlightRecorder::_writeRecord(const DataPoint &record)
{
    _stream << record.timestamp << record.channel << record.data;
}
