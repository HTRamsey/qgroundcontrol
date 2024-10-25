/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>

#include <gps_helper.h>

#include "satellite_info.h"
#include "sensor_gnss_relative.h"
#include "sensor_gps.h"

Q_DECLARE_LOGGING_CATEGORY(GPSProviderLog)

class QSerialPort;
class GPSBaseStationSupport;
class GPSProviderWorker;

class GPSProvider : public QObject
{
    Q_OBJECT

public:
    enum class GPSType {
        u_blox,
        trimble,
        septentrio,
        femto
    };

    struct rtk_data_s {
        double surveyInAccMeters = 0;
        int surveyInDurationSecs = 0;
        bool useFixedBaseLoction = false;
        double fixedBaseLatitude = 0.;
        double fixedBaseLongitude = 0.;
        float fixedBaseAltitudeMeters = 0.f;
        float fixedBaseAccuracyMeters = 0.f;
    };

    GPSProvider(const QString &device, GPSType type, const rtk_data_s &rtkData, std::atomic_bool &requestStop, QObject *parent = nullptr);
    ~GPSProvider();

    bool isRunning() const;

signals:
    void satelliteInfoUpdate(const satellite_info_s &message);
    void sensorGnssRelativeUpdate(const sensor_gnss_relative_s &message);
    void sensorGpsUpdate(const sensor_gps_s &message);
    void RTCMDataUpdate(const QByteArray &message);
    void surveyInStatus(float duration, float accuracyMM, double latitude, double longitude, float altitude, bool valid, bool active);
    void errorOccurred(const QString &error);
    void finished();

public slots:
    void startProvider();
    void stopProvider();

private:
    std::unique_ptr<GPSProviderWorker> _worker;
    QThread *_workerThread = nullptr;
};

/*===========================================================================*/

class GPSProviderWorker : public QObject
{
    Q_OBJECT

public:
    GPSProviderWorker(const QString &device,
                      GPSProvider::GPSType type,
                      const GPSProvider::rtk_data_s &rtkData,
                      std::atomic_bool &requestStop,
                      QObject *parent = nullptr);
    ~GPSProviderWorker();

signals:
    void satelliteInfoUpdate(const satellite_info_s &message);
    void sensorGnssRelativeUpdate(const sensor_gnss_relative_s &message);
    void sensorGpsUpdate(const sensor_gps_s &message);
    void RTCMDataUpdate(const QByteArray &message);
    void surveyInStatus(float duration, float accuracyMM, double latitude, double longitude, float altitude, bool valid, bool active);
    void errorOccurred(const QString &error);
    void finished();

public slots:
    void process();
    void stopProvider();

private:
    bool _connectSerial();
    std::unique_ptr<GPSBaseStationSupport> _connectGPS();
    void _publishSensorGPS();
    void _publishSatelliteInfo();
    void _publishSensorGNSSRelative();
    void _gotRTCMData(const uint8_t *data, size_t len);
    void _sendRTCMData();
    static int _callbackEntry(GPSCallbackType type, void *data1, int data2, void *user);
    int _callback(GPSCallbackType type, void *data1, int data2);
    std::unique_ptr<GPSBaseStationSupport> _createGPSDriver(GPSProvider::GPSType type);

    static constexpr int MaxSerialRetries = 60;
    static constexpr int SerialRetryIntervalMs = 500;
    static constexpr uint32_t GPSReceiveTimeout = 1200;
    static constexpr float GPSHeadingOffset = 5.0f;

    QString _device;
    GPSProvider::GPSType _type;
    std::atomic_bool &_requestStop;
    GPSProvider::rtk_data_s _rtkData;
    GPSHelper::GPSConfig _gpsConfig;

    satellite_info_s _satelliteInfo;
    sensor_gnss_relative_s _sensorGnssRelative;
    sensor_gps_s _sensorGps;

    std::unique_ptr<QSerialPort> _serial;
    std::unique_ptr<GPSBaseStationSupport> _gpsDriver;

    bool _simulateRTCM = false;

    enum GPSReceiveType {
        Position = 1,
        Satellite = 2
    };
};
