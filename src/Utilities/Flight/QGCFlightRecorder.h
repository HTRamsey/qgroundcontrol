#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>
#include <QtPositioning/QGeoCoordinate>

Q_DECLARE_LOGGING_CATEGORY(QGCFlightRecorderLog)

/// Flight data recorder for telemetry logging and playback.
///
/// Records timestamped telemetry data points during flight and supports
/// playback at variable speeds.
///
/// Example usage:
/// @code
/// QGCFlightRecorder recorder;
/// recorder.startRecording("/path/to/flight.qgclog");
/// recorder.record("position", QVariant::fromValue(position));
/// recorder.record("altitude", altitude);
/// recorder.stopRecording();
///
/// // Later, for playback:
/// recorder.loadRecording("/path/to/flight.qgclog");
/// recorder.setPlaybackSpeed(2.0);  // 2x speed
/// recorder.startPlayback();
/// @endcode
class QGCFlightRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(double playbackSpeed READ playbackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged)
    Q_PROPERTY(int recordCount READ recordCount NOTIFY recordCountChanged)

public:
    struct DataPoint {
        qint64 timestamp;      // Milliseconds since recording start
        QString channel;       // Data channel name (e.g., "position", "attitude")
        QVariant data;         // The actual data
    };

    struct RecordingMetadata {
        QString filename;
        QDateTime startTime;
        QDateTime endTime;
        qint64 duration;       // milliseconds
        int recordCount;
        QStringList channels;
        QVariantMap custom;
    };

    explicit QGCFlightRecorder(QObject *parent = nullptr);
    ~QGCFlightRecorder() override;

    // Recording
    bool startRecording(const QString &filename);
    void stopRecording();
    bool isRecording() const { return _recording; }
    void record(const QString &channel, const QVariant &data);
    void recordPosition(const QGeoCoordinate &position, double altitude = 0);
    void recordAttitude(double roll, double pitch, double yaw);
    void setMetadata(const QString &key, const QVariant &value);

    // Playback
    bool loadRecording(const QString &filename);
    void startPlayback();
    void pausePlayback();
    void stopPlayback();
    void seekTo(qint64 position);
    void seekToPercent(double percent);
    bool isPlaying() const { return _playing; }

    // Playback control
    double playbackSpeed() const { return _playbackSpeed; }
    void setPlaybackSpeed(double speed);
    qint64 duration() const { return _duration; }
    qint64 position() const { return _position; }
    double progressPercent() const;

    // Data access
    int recordCount() const { return _records.size(); }
    QStringList channels() const;
    QList<DataPoint> recordsForChannel(const QString &channel) const;
    DataPoint recordAt(int index) const;
    RecordingMetadata metadata() const { return _metadata; }

    // Export
    bool exportToCsv(const QString &filename, const QStringList &channels = {}) const;
    bool exportToJson(const QString &filename) const;

signals:
    void recordingChanged(bool recording);
    void playingChanged(bool playing);
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void playbackSpeedChanged(double speed);
    void recordCountChanged(int count);
    void dataPlayed(const QString &channel, const QVariant &data, qint64 timestamp);
    void playbackFinished();
    void error(const QString &message);

private slots:
    void _playbackTick();

private:
    bool _writeHeader();
    bool _readHeader();
    void _writeRecord(const DataPoint &record);

    QFile _file;
    QDataStream _stream;
    QTimer _playbackTimer;

    bool _recording = false;
    bool _playing = false;
    qint64 _recordingStartTime = 0;
    qint64 _duration = 0;
    qint64 _position = 0;
    double _playbackSpeed = 1.0;
    int _currentPlaybackIndex = 0;
    qint64 _playbackStartTime = 0;
    qint64 _playbackStartPosition = 0;

    QList<DataPoint> _records;
    RecordingMetadata _metadata;

    static constexpr quint32 kMagicNumber = 0x51474346;  // "QGCF"
    static constexpr quint16 kVersion = 1;
};
