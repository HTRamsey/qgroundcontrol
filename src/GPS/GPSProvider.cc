/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "GPSProvider.h"
#include "QGCLoggingCategory.h"
#include "RTCMMavlink.h"

#include <ashtech.h>
#include <base_station.h>
#include <definitions.h>
#include <femtomes.h>
#include <sbf.h>
#include <ubx.h>

#ifdef Q_OS_ANDROID
#include "qserialport.h"
#else
#include <QtSerialPort/QSerialPort>
#endif

QGC_LOGGING_CATEGORY(GPSProviderLog, "qgc.gps.gpsprovider")
QGC_LOGGING_CATEGORY(GPSDriversLog, "qgc.gps.drivers")

GPSProvider::GPSProvider(const QString &device,
                         GPSType type,
                         const rtk_data_s &rtkData,
                         std::atomic_bool &requestStop,
                         QObject *parent)
    : QObject(parent)
    , _worker(std::make_unique<GPSProviderWorker>(device, type, rtkData, requestStop, this))
    , _workerThread(new QThread(this))
{
    // Move the worker to the thread
    _worker->moveToThread(_workerThread);

    // Connect signals and slots
    (void) connect(_workerThread, &QThread::started, _worker.get(), &GPSProviderWorker::process);
    (void) connect(_worker.get(), &GPSProviderWorker::finished, _workerThread, &QThread::quit);
    (void) connect(_worker.get(), &GPSProviderWorker::finished, _worker.get(), &GPSProviderWorker::deleteLater);

    // Relay worker signals to GPSProvider signals
    (void) connect(_worker.get(), &GPSProviderWorker::satelliteInfoUpdate, this, &GPSProvider::satelliteInfoUpdate);
    (void) connect(_worker.get(), &GPSProviderWorker::sensorGnssRelativeUpdate, this, &GPSProvider::sensorGnssRelativeUpdate);
    (void) connect(_worker.get(), &GPSProviderWorker::sensorGpsUpdate, this, &GPSProvider::sensorGpsUpdate);
    (void) connect(_worker.get(), &GPSProviderWorker::RTCMDataUpdate, this, &GPSProvider::RTCMDataUpdate);
    (void) connect(_worker.get(), &GPSProviderWorker::surveyInStatus, this, &GPSProvider::surveyInStatus);
    (void) connect(_worker.get(), &GPSProviderWorker::errorOccurred, this, &GPSProvider::errorOccurred);
    (void) connect(_worker.get(), &GPSProviderWorker::finished, this, &GPSProvider::finished);

    // Start the thread
    _workerThread->start();
}

GPSProvider::~GPSProvider()
{
    stopProvider();

    if (_workerThread->isRunning()) {
        _worker->stopProvider();
        _workerThread->quit();
        if (!_workerThread->wait(5000)) { // Wait up to 5 seconds
            qCWarning(GPSProviderLog) << "GPSProvider thread did not finish in time.";
            _workerThread->terminate();
        }
    }
}

bool GPSProvider::isRunning() const
{
    return _workerThread->isRunning();
}

void GPSProvider::startProvider()
{
    if (!_workerThread->isRunning()) {
        _workerThread->start();
    }
}

void GPSProvider::stopProvider()
{
    if (_workerThread->isRunning()) {
        _worker->stopProvider();
        _workerThread->quit();
        if (!_workerThread->wait(5000)) {
            qCWarning(GPSProviderLog) << "GPSProvider thread did not finish in time.";
            _workerThread->terminate();
        }
    }
}

/*===========================================================================*/

GPSProviderWorker::GPSProviderWorker(
    const QString &device,
    GPSProvider::GPSType type,
    const GPSProvider::rtk_data_s &rtkData,
    std::atomic_bool &requestStop,
    QObject *parent
) : QObject(parent)
  , _device(device)
  , _type(type)
  , _requestStop(requestStop)
  , _rtkData(rtkData)
{
    // Initialize GPS configuration
    _gpsConfig.output_mode = GPSHelper::OutputMode::RTCM;
}

GPSProviderWorker::~GPSProviderWorker()
{
    if (_serial && _serial->isOpen()) {
        _serial->close();
    }
}

void GPSProviderWorker::_publishSatelliteInfo()
{
    emit satelliteInfoUpdate(_satelliteInfo);
}

void GPSProviderWorker::_publishSensorGNSSRelative()
{
    emit sensorGnssRelativeUpdate(_sensorGnssRelative);
}

void GPSProviderWorker::_publishSensorGPS()
{
    emit sensorGpsUpdate(_sensorGps);
}

void GPSProviderWorker::_gotRTCMData(const uint8_t *data, size_t len)
{
    const QByteArray message(reinterpret_cast<const char*>(data), len);
    emit RTCMDataUpdate(message);
}

void GPSProviderWorker::stopProvider()
{
    _requestStop = true;
}

int GPSProviderWorker::_callbackEntry(GPSCallbackType type, void *data1, int data2, void *user)
{
    GPSProviderWorker *gps = reinterpret_cast<GPSProviderWorker*>(user);
    return gps->_callback(type, data1, data2);
}

int GPSProviderWorker::_callback(GPSCallbackType type, void *data1, int data2)
{
    switch (type) {
    case GPSCallbackType::readDeviceData:
        if (_serial->bytesAvailable() == 0) {
            const int timeout = *(reinterpret_cast<int*>(data1));
            if (!_serial->waitForReadyRead(timeout)) {
                return 0;
            }
        }
        return _serial->read(reinterpret_cast<char*>(data1), data2);
    case GPSCallbackType::writeDeviceData:
        if (_serial->write(reinterpret_cast<char*>(data1), data2) >= 0) {
            if (_serial->waitForBytesWritten(-1)) {
                return data2;
            }
        }
        return -1;
    case GPSCallbackType::setBaudrate:
        return (_serial->setBaudRate(data2) ? 0 : -1);
    case GPSCallbackType::gotRTCMMessage:
        _gotRTCMData(reinterpret_cast<uint8_t*>(data1), data2);
        break;
    case GPSCallbackType::surveyInStatus:
    {
        const SurveyInStatus* const status = reinterpret_cast<SurveyInStatus*>(data1);
        qCDebug(GPSProviderLog) << "Position:" << status->latitude << status->longitude << status->altitude;

        const bool valid = status->flags & 1;
        const bool active = (status->flags >> 1) & 1;

        qCDebug(GPSProviderLog) << QString("Survey-in status: %1s cur accuracy: %2mm valid: %3 active: %4").arg(status->duration).arg(status->mean_accuracy).arg(valid).arg(active);
        emit surveyInStatus(status->duration, status->mean_accuracy, status->latitude, status->longitude, status->altitude, valid, active);
        break;
    }
    case GPSCallbackType::setClock:
    case GPSCallbackType::gotRelativePositionMessage:
        // _sensorGnssRelative
    default:
        break;
    }

    return 0;
}

void GPSProviderWorker::process()
{
    qCDebug(GPSProviderLog) << "GPSProviderWorker started.";

#ifdef SIMULATE_RTCM_OUTPUT
    _simulateRTCM = true;
#endif

    if (!_simulateRTCM) {
        if (!_connectSerial()) {
            emit errorOccurred(QString("Failed to connect to serial port: %1").arg(_device));
            emit finished();
            return;
        }

        _gpsDriver = _connectGPS();
        if (!_gpsDriver) {
            emit errorOccurred(QString("Failed to initialize GPS driver for type: %1").arg(static_cast<int>(_type)));
            emit finished();
            return;
        }
    }

    GPSBaseStationSupport *gpsDriver = nullptr;

    while (!_requestStop) {
        if (_simulateRTCM) {
            _sendRTCMData();
            continue;
        }

        if (!_gpsDriver) {
            emit errorOccurred(tr("GPS driver is not initialized."));
            break;
        }

        (void) std::memset(&_sensorGps, 0, sizeof(_sensorGps));

        uint8_t numTries = 0;
        while (!_requestStop && (numTries < 3)) {
            const int helperRet = _gpsDriver->receive(GPSReceiveTimeout);

            if (helperRet > 0) {
                numTries = 0;

                if (helperRet & GPSReceiveType::Position) {
                    _publishSensorGPS();
                    numTries = 0;
                }

                if (helperRet & GPSReceiveType::Satellite) {
                    _publishSatelliteInfo();
                    numTries = 0;
                }
            } else {
                ++numTries;
                QThread::msleep(100);
            }
        }

        if ((_serial->error() != QSerialPort::NoError) && (_serial->error() != QSerialPort::TimeoutError)) {
            emit errorOccurred(tr("Serial port error: %1").arg(_serial->errorString()));
            break;
        }
    }

    if (_gpsDriver) {
        _gpsDriver.reset();
    }

    if (_serial && _serial->isOpen()) {
        _serial->close();
    }

    qCDebug(GPSProviderLog) << "GPSProviderWorker exiting.";
    emit finished();
}

bool GPSProviderWorker::_connectSerial()
{
    _serial = std::make_unique<QSerialPort>();
    _serial->setPortName(_device);
    if (!_serial->open(QIODevice::ReadWrite)) {
        // Give the device some time to come up. In some cases the device is not
        // immediately accessible right after startup for some reason. This can take 10-20s.
        uint32_t retries = MaxSerialRetries;
        while ((retries-- > 0) && (_serial->error() == QSerialPort::PermissionError)) {
            qCWarning(GPSProviderLog) << "Cannot open device" << _device << "... retrying";
            QThread::msleep(SerialRetryIntervalMs);
            if (_serial->open(QIODevice::ReadWrite)) {
                _serial->clearError();
                break;
            }
        }

        if (_serial->error() != QSerialPort::NoError) {
            qCWarning(GPSProviderLog) << "GPS: Failed to open Serial Device" << _device << _serial->errorString();
            return false;
        }
    }

    (void) _serial->setBaudRate(QSerialPort::Baud9600);
    (void) _serial->setDataBits(QSerialPort::Data8);
    (void) _serial->setParity(QSerialPort::NoParity);
    (void) _serial->setStopBits(QSerialPort::OneStop);
    (void) _serial->setFlowControl(QSerialPort::NoFlowControl);

    return true;
}

std::unique_ptr<GPSBaseStationSupport> GPSProviderWorker::_createGPSDriver(GPSProvider::GPSType type)
{
    switch (type) {
    case GPSProvider::GPSType::trimble:
        return std::make_unique<GPSDriverAshtech>(&GPSProviderWorker::_callbackEntry, this, &_sensorGps, &_satelliteInfo);
    case GPSProvider::GPSType::septentrio:
        return std::make_unique<GPSDriverSBF>(&GPSProviderWorker::_callbackEntry, this, &_sensorGps, &_satelliteInfo, GPSHeadingOffset);
    case GPSProvider::GPSType::u_blox:
        return std::make_unique<GPSDriverUBX>(GPSDriverUBX::Interface::UART, &GPSProviderWorker::_callbackEntry, this, &_sensorGps, &_satelliteInfo);
    case GPSProvider::GPSType::femto:
        return std::make_unique<GPSDriverFemto>(&GPSProviderWorker::_callbackEntry, this, &_sensorGps, &_satelliteInfo);
    default:
        qCWarning(GPSProviderLog) << "Unsupported GPS Type:" << static_cast<int>(type);
        return nullptr;
    }
}

std::unique_ptr<GPSBaseStationSupport> GPSProviderWorker::_connectGPS()
{
    std::unique_ptr<GPSBaseStationSupport> gpsDriver = _createGPSDriver(_type);
    if (!gpsDriver) {
        return nullptr;
    }

    uint32_t baudrate = 0;
    if (_type == GPSProvider::GPSType::trimble) {
        baudrate = 115200;
    }

    gpsDriver->setSurveyInSpecs(_rtkData.surveyInAccMeters * 10000.f, _rtkData.surveyInDurationSecs);

    if (_rtkData.useFixedBaseLoction) {
        gpsDriver->setBasePosition(_rtkData.fixedBaseLatitude, _rtkData.fixedBaseLongitude, _rtkData.fixedBaseAltitudeMeters, _rtkData.fixedBaseAccuracyMeters * 1000.0f);
    }

    if (gpsDriver->configure(baudrate, _gpsConfig) != 0) {
        qCWarning(GPSProviderLog) << "Failed to configure GPS driver.";
        return nullptr;
    }

    return gpsDriver;
}

void GPSProviderWorker::_sendRTCMData()
{
    RTCMMavlink *const rtcm = new RTCMMavlink(this);

    const int fakeMsgLengths[3] = { 30, 170, 240 };
    std::unique_ptr<uint8_t[]> fakeData = std::make_unique<uint8_t[]>(fakeMsgLengths[2]);

    while (!_requestStop) {
        for (int i = 0; i < 3; ++i) {
            const QByteArray message(reinterpret_cast<const char*>(fakeData.get()), fakeMsgLengths[i]);
            rtcm->RTCMDataUpdate(message);
            QThread::msleep(4);
        }
        QThread::msleep(100);
    }
}
