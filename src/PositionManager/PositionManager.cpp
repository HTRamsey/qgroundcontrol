#include "PositionManager.h"
#include "AppMessages.h"
#include "QGCCorePlugin.h"
#include "SatelliteModel.h"
#include "SimulatedPosition.h"
// #include "QGCSensors.h"
#include "QGCLoggingCategory.h"
#include "SettingsManager.h"
#include "AutoConnectSettings.h"
#include "UdpIODevice.h"

#include <QtCore/QApplicationStatic>
#include <QtCore/QPermissions>
#include <QtCore/QTimer>
#include <QtNetwork/QHostAddress>
#include <QtPositioning/QGeoSatelliteInfoSource>
#include <QtPositioning/QNmeaPositionInfoSource>

#ifndef QGC_NO_SERIAL_LINK
#include "QGCSerialPortInfo.h"
#include <QtSerialPort/QSerialPort>
#endif

QGC_LOGGING_CATEGORY(QGCPositionManagerLog, "PositionManager.QGCPositionManager")

Q_APPLICATION_STATIC(QGCPositionManager, _positionManager);

QGCPositionManager::QGCPositionManager(QObject *parent)
    : QObject(parent)
    , _satelliteModel(new SatelliteModel(this))
{
    qCDebug(QGCPositionManagerLog) << this;
}

QGCPositionManager::~QGCPositionManager()
{
    qCDebug(QGCPositionManagerLog) << this;
}

QGCPositionManager *QGCPositionManager::instance()
{
    return _positionManager();
}

void QGCPositionManager::init()
{
    if (QGC::runningUnitTests()) {
        _simulatedSource = new SimulatedPosition(this);
        _setPositionSource(QGCPositionSource::Simulated);
    } else {
        _checkPermission();

        _nmeaPollTimer = new QTimer(this);
        _nmeaPollTimer->setInterval(kNmeaPollIntervalMs);
        (void) connect(_nmeaPollTimer, &QTimer::timeout, this, &QGCPositionManager::_pollNmeaDevice);
        _nmeaPollTimer->start();
    }
}

void QGCPositionManager::_setupPositionSources()
{
    _defaultSource = QGCCorePlugin::instance()->createPositionSource(this);
    if (_defaultSource) {
        _usingPluginSource = true;
    } else {
        qCDebug(QGCPositionManagerLog) << Q_FUNC_INFO << QGeoPositionInfoSource::availableSources();

        _defaultSource = QGeoPositionInfoSource::createDefaultSource(this);
        if (!_defaultSource) {
            qCWarning(QGCPositionManagerLog) << Q_FUNC_INFO << "No default source available";
            return;
        }
    }

    _setPositionSource(QGCPositionSource::InternalGPS);
    _setupSatelliteSource();
}

void QGCPositionManager::_setupSatelliteSource()
{
    _satelliteSource = QGeoSatelliteInfoSource::createDefaultSource(this);
    if (!_satelliteSource) {
        qCDebug(QGCPositionManagerLog) << "No satellite info source available";
        return;
    }

    (void) connect(_satelliteSource, &QGeoSatelliteInfoSource::satellitesInViewUpdated,
                   this, &QGCPositionManager::_satellitesInViewUpdated);
    (void) connect(_satelliteSource, &QGeoSatelliteInfoSource::satellitesInUseUpdated,
                   this, &QGCPositionManager::_satellitesInUseUpdated);
    _satelliteSource->startUpdates();
}

void QGCPositionManager::_satellitesInViewUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    _satellitesInView = satellites;
    _satelliteModel->updateFromQtPositioning(_satellitesInView, _satellitesInUse);
}

void QGCPositionManager::_satellitesInUseUpdated(const QList<QGeoSatelliteInfo> &satellites)
{
    _satellitesInUse = satellites;
    _satelliteModel->updateFromQtPositioning(_satellitesInView, _satellitesInUse);
}

void QGCPositionManager::_handlePermissionStatus(Qt::PermissionStatus permissionStatus)
{
    if (permissionStatus == Qt::PermissionStatus::Granted) {
        _setupPositionSources();
    } else {
        qCWarning(QGCPositionManagerLog) << Q_FUNC_INFO << "Location Permission Denied";
    }
}

void QGCPositionManager::_checkPermission()
{
    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);

    const Qt::PermissionStatus permissionStatus = QCoreApplication::instance()->checkPermission(locationPermission);
    if (permissionStatus == Qt::PermissionStatus::Undetermined) {
        QCoreApplication::instance()->requestPermission(locationPermission, this, [this](const QPermission &permission) {
            _handlePermissionStatus(permission.status());
        });
    } else {
        _handlePermissionStatus(permissionStatus);
    }
}

void QGCPositionManager::setNmeaSourceDevice(QIODevice *device)
{
    if (_nmeaSource) {
        _nmeaSource->stopUpdates();
        (void) disconnect(_nmeaSource);

        if (_currentSource == _nmeaSource) {
            _currentSource = nullptr;
        }

        delete _nmeaSource;
        _nmeaSource = nullptr;
    }

    _nmeaSource = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::RealTimeMode, this);
    _nmeaSource->setDevice(device);
    _nmeaSource->setUserEquivalentRangeError(5.1);
    _setPositionSource(QGCPositionManager::NmeaGPS);
}

void QGCPositionManager::_pollNmeaDevice()
{
    AutoConnectSettings *const settings = SettingsManager::instance()->autoConnectSettings();
    const QString portSetting = settings->autoConnectNmeaPort()->cookedValueString();

    if (portSetting.isEmpty() || (portSetting == QLatin1String("Disabled"))) {
        _closeNmeaDevice();
        return;
    }

    if (portSetting == QLatin1String("UDP Port")) {
        _pollUdpNmeaDevice(settings);
        return;
    }

#ifndef QGC_NO_SERIAL_LINK
    _pollSerialNmeaDevice(settings, portSetting);
#endif
}

void QGCPositionManager::_pollUdpNmeaDevice(AutoConnectSettings *settings)
{
#ifndef QGC_NO_SERIAL_LINK
    _closeSerialPort();
#endif

    if (!_nmeaUdpSocket) {
        _nmeaUdpSocket = new UdpIODevice(this);
    }

    const uint16_t udpPort = static_cast<uint16_t>(settings->nmeaUdpPort()->rawValue().toUInt());
    if ((_nmeaUdpSocket->localPort() != udpPort) || (_nmeaUdpSocket->state() != UdpIODevice::BoundState)) {
        _openNmeaUdpPort(udpPort);
    }
}

void QGCPositionManager::_openNmeaUdpPort(uint16_t udpPort)
{
    _nmeaUdpSocket->close();
    if (_nmeaUdpSocket->bind(QHostAddress::AnyIPv4, udpPort)) {
        qCDebug(QGCPositionManagerLog) << "Binding UDP NMEA port" << udpPort;
        setNmeaSourceDevice(_nmeaUdpSocket);
    } else {
        qCWarning(QGCPositionManagerLog) << "Failed to bind UDP NMEA port" << udpPort;
    }
}

void QGCPositionManager::_closeNmeaDevice()
{
#ifndef QGC_NO_SERIAL_LINK
    _closeSerialPort();
#endif
    if (_nmeaUdpSocket) {
        _nmeaUdpSocket->close();
    }
}

#ifndef QGC_NO_SERIAL_LINK
void QGCPositionManager::_pollSerialNmeaDevice(AutoConnectSettings *settings, const QString &portSetting)
{
    if (_nmeaUdpSocket) {
        _nmeaUdpSocket->close();
    }

    const uint32_t baud = settings->autoConnectNmeaBaud()->cookedValue().toUInt();

    if (_nmeaSerialPort && (_nmeaSerialPortName == portSetting)) {
        if (_nmeaSerialBaud != baud) {
            _nmeaSerialBaud = baud;
            _nmeaSerialPort->setBaudRate(static_cast<qint32>(baud));
            qCDebug(QGCPositionManagerLog) << "Configuring NMEA baudrate" << baud;
        }
        return;
    }

    // Only open the configured device once it is physically present, so we don't
    // repeatedly retry a disconnected port.
    bool present = false;
    for (const QGCSerialPortInfo &info : QGCSerialPortInfo::availablePorts()) {
        if (info.systemLocation() == portSetting) {
            present = true;
            break;
        }
    }

    if (present) {
        _openNmeaSerialPort(portSetting, baud);
    }
}

void QGCPositionManager::_openNmeaSerialPort(const QString &portName, uint32_t baud)
{
    auto *const port = new QSerialPort(portName, this);
    port->setBaudRate(static_cast<qint32>(baud));
    if (!port->open(QIODevice::ReadOnly)) {
        qCWarning(QGCPositionManagerLog) << "Failed to open NMEA port" << portName << port->errorString();
        delete port;
        return;
    }

    qCDebug(QGCPositionManagerLog) << "Configuring NMEA port" << portName << "baudrate" << baud;
    // setNmeaSourceDevice() deletes the previous source (which referenced the old
    // port), so the old port is unreferenced and safe to delete afterwards.
    setNmeaSourceDevice(port);
    if (_nmeaSerialPort) {
        _nmeaSerialPort->close();
        delete _nmeaSerialPort;
    }

    _nmeaSerialPort = port;
    _nmeaSerialPortName = portName;
    _nmeaSerialBaud = baud;
}

void QGCPositionManager::_closeSerialPort()
{
    if (_nmeaSerialPort) {
        if (_nmeaSource) {
            _nmeaSource->setDevice(nullptr);
        }
        _nmeaSerialPort->close();
        delete _nmeaSerialPort;
        _nmeaSerialPort = nullptr;
        _nmeaSerialPortName.clear();
        _nmeaSerialBaud = 0;
    }
}
#endif // !QGC_NO_SERIAL_LINK

void QGCPositionManager::_positionUpdated(const QGeoPositionInfo &update)
{
    _geoPositionInfo = update;
    _gcsPositioningError = QGeoPositionInfoSource::NoError;

    QGeoCoordinate newGCSPosition(_gcsPosition);

    if (update.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        if ((qAbs(update.coordinate().latitude()) > 0.001) && (qAbs(update.coordinate().longitude()) > 0.001)) {
            _gcsPositionHorizontalAccuracy = update.attribute(QGeoPositionInfo::HorizontalAccuracy);
            if (_gcsPositionHorizontalAccuracy <= kMinHorizonalAccuracyMeters) {
                newGCSPosition.setLatitude(update.coordinate().latitude());
                newGCSPosition.setLongitude(update.coordinate().longitude());
            }
            emit gcsPositionHorizontalAccuracyChanged(_gcsPositionHorizontalAccuracy);
        }
    }

    if (update.hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
        _gcsPositionVerticalAccuracy = update.attribute(QGeoPositionInfo::VerticalAccuracy);
        if (_gcsPositionVerticalAccuracy <= kMinVerticalAccuracyMeters) {
            newGCSPosition.setAltitude(update.coordinate().altitude());
        }
    }

    _gcsPositionAccuracy = sqrt(pow(_gcsPositionHorizontalAccuracy, 2) + pow(_gcsPositionVerticalAccuracy, 2));

    _setGCSPosition(newGCSPosition);

    if (update.hasAttribute(QGeoPositionInfo::DirectionAccuracy)) {
        _gcsDirectionAccuracy = update.attribute(QGeoPositionInfo::DirectionAccuracy);
        if (_gcsDirectionAccuracy <= kMinDirectionAccuracyDegrees) {
            _setGCSHeading(update.attribute(QGeoPositionInfo::Direction));
        }
    } else if (_usingPluginSource) {
        _setGCSHeading(update.attribute(QGeoPositionInfo::Direction));
    }

    emit positionInfoUpdated(update);
}

void QGCPositionManager::_positionError(QGeoPositionInfoSource::Error gcsPositioningError)
{
    qCWarning(QGCPositionManagerLog) << Q_FUNC_INFO << "Positioning error:" << gcsPositioningError;
    _gcsPositioningError = gcsPositioningError;
}

void QGCPositionManager::_setGCSHeading(qreal newGCSHeading)
{
    if (newGCSHeading != _gcsHeading) {
        _gcsHeading = newGCSHeading;
        emit gcsHeadingChanged(_gcsHeading);
    }
}

void QGCPositionManager::_setGCSPosition(const QGeoCoordinate& newGCSPosition)
{
    if (newGCSPosition != _gcsPosition) {
        _gcsPosition = newGCSPosition;
        emit gcsPositionChanged(_gcsPosition);
    }
}

void QGCPositionManager::_setPositionSource(QGCPositionSource source)
{
    if (_currentSource != nullptr) {
        _currentSource->stopUpdates();
        (void) disconnect(_currentSource);

        _geoPositionInfo = QGeoPositionInfo();
        emit positionInfoUpdated(_geoPositionInfo);

        _setGCSPosition(QGeoCoordinate());

        _setGCSHeading(qQNaN());

        _gcsPositionHorizontalAccuracy = std::numeric_limits<qreal>::infinity();
        emit gcsPositionHorizontalAccuracyChanged(_gcsPositionHorizontalAccuracy);
    }

    switch (source) {
    case QGCPositionManager::Log:
        break;
    case QGCPositionManager::Simulated:
        _currentSource = _simulatedSource;
        break;
    case QGCPositionManager::NmeaGPS:
        _currentSource = _nmeaSource;
        break;
    case QGCPositionManager::InternalGPS:
        _currentSource = _defaultSource;
        break;
    case QGCPositionManager::ExternalGPS:
        break;
    default:
        _currentSource = _defaultSource;
        break;
    }

    if (_currentSource != nullptr) {
        _currentSource->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);
        _updateInterval = _currentSource->minimumUpdateInterval();
        #if !defined(Q_OS_DARWIN) && !defined(Q_OS_IOS)
            _currentSource->setUpdateInterval(_updateInterval);
        #endif

        (void) connect(_currentSource, &QGeoPositionInfoSource::positionUpdated, this, &QGCPositionManager::_positionUpdated);
        (void) connect(_currentSource, &QGeoPositionInfoSource::errorOccurred, this, &QGCPositionManager::_positionError);

        // (void) connect(QGCCompass::instance(), &QGCCompass::positionUpdated, this, &QGCPositionManager::_positionUpdated);

        _currentSource->startUpdates();
    }
}
