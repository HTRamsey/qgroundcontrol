#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>
#include <QtPositioning/QGeoSatelliteInfo>
#include <QtQmlIntegration/QtQmlIntegration>

#include <cstdint>

class QNmeaPositionInfoSource;
class QGeoSatelliteInfoSource;
class QGCCompass;
class QTimer;
class QSerialPort;
class UdpIODevice;
class AutoConnectSettings;
class SatelliteModel;

class QGCPositionManager : public QObject
{
    Q_OBJECT
    friend class PositionManagerTest;
    QML_ELEMENT
    QML_UNCREATABLE("")
    Q_MOC_INCLUDE("SatelliteModel.h")

    Q_PROPERTY(QGeoCoordinate gcsPosition                   READ gcsPosition                    NOTIFY gcsPositionChanged)
    Q_PROPERTY(qreal          gcsHeading                    READ gcsHeading                     NOTIFY gcsHeadingChanged)
    Q_PROPERTY(qreal          gcsPositionHorizontalAccuracy READ gcsPositionHorizontalAccuracy  NOTIFY gcsPositionHorizontalAccuracyChanged)
    Q_PROPERTY(SatelliteModel *satelliteModel               READ satelliteModel                 CONSTANT)

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

    /// Satellites in view of the GCS-local receiver (internal GPS or NMEA source).
    SatelliteModel *satelliteModel() { return _satelliteModel; }

    void setNmeaSourceDevice(QIODevice *device);

signals:
    void gcsPositionChanged(QGeoCoordinate gcsPosition);
    void gcsHeadingChanged(qreal gcsHeading);
    void positionInfoUpdated(QGeoPositionInfo update);
    void gcsPositionHorizontalAccuracyChanged(qreal gcsPositionHorizontalAccuracy);

private slots:
    void _positionUpdated(const QGeoPositionInfo &update);
    void _positionError(QGeoPositionInfoSource::Error gcsPositioningError);
    void _satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites);
    void _satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &satellites);

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
    void _setupSatelliteSource();
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

    QGeoSatelliteInfoSource *_satelliteSource = nullptr;
    SatelliteModel *_satelliteModel = nullptr;
    QList<QGeoSatelliteInfo> _satellitesInView;
    QList<QGeoSatelliteInfo> _satellitesInUse;

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
