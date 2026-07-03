#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtQmlIntegration/QtQmlIntegration>

#include <cstdint>

class QNmeaPositionInfoSource;
class QGCCompass;
class QTimer;
class QSerialPort;
class UdpIODevice;
class AutoConnectSettings;

class QGCPositionManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_PROPERTY(QGeoCoordinate gcsPosition                   READ gcsPosition                    NOTIFY gcsPositionChanged)
    Q_PROPERTY(qreal          gcsHeading                    READ gcsHeading                     NOTIFY gcsHeadingChanged)
    Q_PROPERTY(qreal          gcsPositionHorizontalAccuracy READ gcsPositionHorizontalAccuracy  NOTIFY gcsPositionHorizontalAccuracyChanged)

public:
    explicit QGCPositionManager(QObject *parent = nullptr);
    ~QGCPositionManager();

    /// Gets the singleton instance of AudioOutput.
    ///     @return The singleton instance.
    static QGCPositionManager *instance();

    void init();
    QGeoCoordinate gcsPosition() const { return _gcsPosition; }
    qreal gcsHeading() const { return _gcsHeading; }
    qreal gcsPositionHorizontalAccuracy() const { return _gcsPositionHorizontalAccuracy; }
    QGeoPositionInfo geoPositionInfo() const { return _geoPositionInfo; }
    QGeoPositionInfoSource::Error gcsPositioningError() const { return _gcsPositioningError; }

    int updateInterval() const { return _updateInterval; }

    void setNmeaSourceDevice(QIODevice *device);

signals:
    void gcsPositionChanged(QGeoCoordinate gcsPosition);
    void gcsHeadingChanged(qreal gcsHeading);
    void positionInfoUpdated(QGeoPositionInfo update);
    void gcsPositionHorizontalAccuracyChanged(qreal gcsPositionHorizontalAccuracy);

private slots:
    void _positionUpdated(const QGeoPositionInfo &update);
    void _positionError(QGeoPositionInfoSource::Error gcsPositioningError);

private:
    enum QGCPositionSource {
        Simulated,
        InternalGPS,
        Log,
        NmeaGPS,
        ExternalGPS
    };

    void _setPositionSource(QGCPositionSource source);
    void _setupPositionSources();
    void _handlePermissionStatus(Qt::PermissionStatus permissionStatus);
    void _checkPermission();
    void _setGCSHeading(qreal newGCSHeading);
    void _setGCSPosition(const QGeoCoordinate &newGCSPosition);

    void _pollNmeaDevice();
    void _pollUdpNmeaDevice(AutoConnectSettings *settings);
    void _openNmeaUdpPort(uint16_t udpPort);
    void _closeNmeaDevice();
#ifndef QGC_NO_SERIAL_LINK
    void _pollSerialNmeaDevice(AutoConnectSettings *settings, const QString &portSetting);
    void _openNmeaSerialPort(const QString &portName, uint32_t baud);
    void _closeSerialPort();
#endif

    bool _usingPluginSource = false;
    int _updateInterval = 0;

    QGeoPositionInfo _geoPositionInfo;
    QGeoPositionInfoSource::Error  _gcsPositioningError = QGeoPositionInfoSource::NoError;

    QGeoCoordinate _gcsPosition;
    qreal _gcsHeading = qQNaN();
    qreal _gcsPositionHorizontalAccuracy = std::numeric_limits<qreal>::infinity();
    qreal _gcsPositionVerticalAccuracy = std::numeric_limits<qreal>::infinity();
    qreal _gcsPositionAccuracy = std::numeric_limits<qreal>::infinity();
    qreal _gcsDirectionAccuracy = std::numeric_limits<qreal>::infinity();

    QGeoPositionInfoSource *_currentSource = nullptr;
    QGeoPositionInfoSource *_defaultSource = nullptr;
    QNmeaPositionInfoSource *_nmeaSource = nullptr;
    QGeoPositionInfoSource *_simulatedSource = nullptr;

    QGCCompass *_compass = nullptr;

    QTimer *_nmeaPollTimer = nullptr;
    UdpIODevice *_nmeaUdpSocket = nullptr;
#ifndef QGC_NO_SERIAL_LINK
    QSerialPort *_nmeaSerialPort = nullptr;
    QString _nmeaSerialPortName;
    uint32_t _nmeaSerialBaud = 0;
#endif

    static constexpr qreal kMinHorizonalAccuracyMeters = 100.;
    static constexpr qreal kMinVerticalAccuracyMeters = 10.;
    static constexpr qreal kMinDirectionAccuracyDegrees = 30.;
    static constexpr int kNmeaPollIntervalMs = 1000;
};
