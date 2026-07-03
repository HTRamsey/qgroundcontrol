#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include <atomic>
#include <memory>

#include "GPSProvider.h"
#include "satellite_info.h"
#include "sensor_gps.h"

class GPSRTKFactGroup;
class FactGroup;
class RTCMMavlink;

class GPSRtk : public QObject
{
    Q_OBJECT

public:
    explicit GPSRtk(QObject* parent = nullptr);
    ~GPSRtk();

    /// Connect to a serial-attached RTK receiver. No-op in a QGC_NO_SERIAL_LINK build.
    void connectGPS(const QString& device, QStringView gps_type);
    /// Connect to a network-attached RTK receiver / base station over TCP.
    void connectGPS(const QString& host, quint16 port, QStringView gps_type);
    void disconnectGPS();
    bool connected() const;
    FactGroup* gpsRtkFactGroup();

    /// Shared RTCM→MAVLink forwarder, injected by GPSManager so serial-RTK and NTRIP
    /// corrections share one GPS_RTCM_DATA sequence-id domain.
    void setRtcmMavlink(RTCMMavlink* rtcmMavlink) { _rtcmMavlink = rtcmMavlink; }

    struct SatelliteCounts
    {
        uint8_t inView = 0;
        int used = 0;
    };

    /// Clamp count to the array bound and tally used-in-solution satellites.
    static SatelliteCounts countSatellites(const satellite_info_s& msg);

private slots:
    void _satelliteInfoUpdate(const satellite_info_s& msg);
    void _sensorGpsUpdate(const sensor_gps_s& msg);
    void _onGPSConnect();
    void _onGPSDisconnect();
    void _onGPSConnectionError(GPSConnectionError error);
    void _onGPSSurveyInStatus(const GPSSurveyInStatus& status);

private:
    void _setConnected(bool connected);
    GPSType _resolveGPSType(QStringView gps_type, int& manufacturerId) const;
    void _startProvider(GPSProvider::TransportFactory makeTransport, std::shared_ptr<std::atomic_bool> requestStop,
                        GPSType type, int manufacturerId);

    GPSProvider* _gpsProvider = nullptr;
    GPSRTKFactGroup* _gpsRtkFactGroup = nullptr;
    RTCMMavlink* _rtcmMavlink = nullptr;

    bool _connected = false;
    // Per-provider stop flag. Shared ownership so an abandoned (slow-to-exit) provider
    // keeps its own flag alive while a freshly started provider gets a distinct one.
    std::shared_ptr<std::atomic_bool> _requestGpsStop;

    static constexpr uint32_t kGPSThreadDisconnectTimeout = 2000;
};
