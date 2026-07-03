#pragma once

#include "GPSDriver.h"  // facade; also publishes GPSReceiverConfig + the GNSS data structs relayed below
#include "GPSType.h"

#include <QtCore/QByteArray>
#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtCore/QThread>

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>

class GPSTransport;

enum class GPSConnectionError
{
    None,
    OpenFailed,   ///< serial device could not be opened
    ConfigFailed, ///< receiver did not accept configuration
    DeviceError,  ///< fatal serial error after a working connection
};
Q_DECLARE_METATYPE(GPSConnectionError)

class GPSProvider : public QThread
{
    Q_OBJECT

public:
    /// Builds the byte-link the driver pumps. Invoked once on the worker thread in
    /// run(), so transports with thread affinity (e.g. QSerialPort) are created there.
    using TransportFactory = std::function<std::unique_ptr<GPSTransport>()>;

    GPSProvider(TransportFactory makeTransport, GPSType type, const GPSReceiverConfig &config, std::shared_ptr<std::atomic_bool> requestStop, QObject *parent = nullptr);

signals:
    void satelliteInfoUpdate(const satellite_info_s &message);
    void sensorGpsUpdate(const sensor_gps_s &message);
    void RTCMDataUpdate(const QByteArray &message);
    void surveyInStatus(const GPSSurveyInStatus &status);
    void connectionError(GPSConnectionError error);
    void connectionEstablished();

private:
    void run() final;

    TransportFactory _makeTransport;
    GPSType _type;
    std::shared_ptr<std::atomic_bool> _requestStop;
    GPSReceiverConfig _config{};

    static constexpr uint32_t kGPSReceiveTimeout = 1200;
    static constexpr uint32_t kConfigRetryDelayMs = 500;
    static constexpr uint8_t kMaxIdleReceiveCycles = 3;
};
