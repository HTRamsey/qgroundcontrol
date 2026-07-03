#include "GPSRtk.h"

#include "GPSEvent.h"
#include "GPSEventModel.h"
#include "GPSProvider.h"
#include "GPSRTKFactGroup.h"
#include "GPSTransport.h"
#include "GPSType.h"
#include "QGCLoggingCategory.h"
#include "RTCMMavlink.h"
#include "RTKSettings.h"
#include "SatelliteModel.h"
#include "SettingsManager.h"
#include "TcpGPSTransport.h"

#ifndef QGC_NO_SERIAL_LINK
#include "SerialGPSTransport.h"
#endif

#include <memory>
#include <utility>

QGC_LOGGING_CATEGORY(GPSRtkLog, "GPS.GPSRtk")

namespace {
struct GPSTypeEntry
{
    QLatin1StringView key;
    GPSType type;
    int manufacturerId;  // RTKSettings::baseReceiverManufacturers enum value
};

constexpr GPSTypeEntry kGPSTypeTable[] = {
    {QLatin1StringView("trimble"), GPSType::trimble, 1},
    {QLatin1StringView("septentrio"), GPSType::septentrio, 2},
    {QLatin1StringView("femtomes"), GPSType::femto, 3},
    {QLatin1StringView("blox"), GPSType::u_blox, 4},
};

constexpr int kMaxGpsEvents = 100;
}  // namespace

GPSRtk::GPSRtk(QObject* parent) : QObject(parent), _gpsRtkFactGroup(new GPSRTKFactGroup(this))
{
    qCDebug(GPSRtkLog) << this;

    (void) qRegisterMetaType<satellite_info_s>("satellite_info_s");
    (void) qRegisterMetaType<sensor_gps_s>("sensor_gps_s");
    (void) qRegisterMetaType<GPSConnectionError>("GPSConnectionError");
    (void) qRegisterMetaType<GPSSurveyInStatus>("GPSSurveyInStatus");
}

GPSRtk::~GPSRtk()
{
    disconnectGPS();

    qCDebug(GPSRtkLog) << this;
}

void GPSRtk::_setConnected(bool connected)
{
    _connected = connected;
    _gpsRtkFactGroup->connected()->setRawValue(connected);
}

void GPSRtk::_onGPSConnect()
{
    _setConnected(true);
    _gpsRtkFactGroup->eventModel()->append(
        GPSEvent::info(GPSEvent::Source::GPS, tr("GPS receiver connected")), kMaxGpsEvents);
}

void GPSRtk::_onGPSDisconnect()
{
    _setConnected(false);
    _gpsRtkFactGroup->satelliteModel()->clear();
    _gpsRtkFactGroup->eventModel()->append(
        GPSEvent::info(GPSEvent::Source::GPS, tr("GPS receiver disconnected")), kMaxGpsEvents);
}

void GPSRtk::_onGPSConnectionError(GPSConnectionError error)
{
    QString message;
    switch (error) {
        case GPSConnectionError::OpenFailed:
            message = tr("Failed to open GPS device");
            break;
        case GPSConnectionError::ConfigFailed:
            message = tr("GPS receiver did not accept configuration");
            break;
        case GPSConnectionError::DeviceError:
            message = tr("GPS device error, connection lost");
            break;
        case GPSConnectionError::None:
            break;
    }

    if (!message.isEmpty()) {
        qCWarning(GPSRtkLog) << message;
        _gpsRtkFactGroup->eventModel()->append(
            GPSEvent::error(GPSEvent::Source::GPS, message), kMaxGpsEvents);
    }

    _gpsRtkFactGroup->lastError()->setRawValue(static_cast<int>(error));
}

void GPSRtk::_onGPSSurveyInStatus(const GPSSurveyInStatus& status)
{
    _gpsRtkFactGroup->currentDuration()->setRawValue(status.durationSecs);
    _gpsRtkFactGroup->currentAccuracy()->setRawValue(static_cast<double>(status.meanAccuracyMM) / 1000.0);
    _gpsRtkFactGroup->currentLatitude()->setRawValue(status.latitude);
    _gpsRtkFactGroup->currentLongitude()->setRawValue(status.longitude);
    _gpsRtkFactGroup->currentAltitude()->setRawValue(status.altitude);
    _gpsRtkFactGroup->valid()->setRawValue(status.valid);
    _gpsRtkFactGroup->active()->setRawValue(status.active);
}

GPSType GPSRtk::_resolveGPSType(QStringView gps_type, int& manufacturerId) const
{
    GPSType type = GPSType::u_blox;
    manufacturerId = 4;  // u-blox by default
    for (const GPSTypeEntry& entry : kGPSTypeTable) {
        if (gps_type.contains(entry.key, Qt::CaseInsensitive)) {
            type = entry.type;
            manufacturerId = entry.manufacturerId;
            break;
        }
    }
    return type;
}

void GPSRtk::connectGPS(const QString& device, QStringView gps_type)
{
#ifndef QGC_NO_SERIAL_LINK
    int manufacturerId = 0;
    const GPSType type = _resolveGPSType(gps_type, manufacturerId);
    qCDebug(GPSRtkLog) << "Connecting serial GPS device" << device << gps_type;

    auto requestStop = std::make_shared<std::atomic_bool>(false);
    GPSProvider::TransportFactory makeTransport = [device, requestStop]() -> std::unique_ptr<GPSTransport> {
        return std::make_unique<SerialGPSTransport>(device, requestStop);
    };
    _startProvider(std::move(makeTransport), std::move(requestStop), type, manufacturerId);
#else
    Q_UNUSED(device)
    Q_UNUSED(gps_type)
    qCWarning(GPSRtkLog) << "Serial RTK is unavailable in this build (QGC_NO_SERIAL_LINK)";
#endif
}

void GPSRtk::connectGPS(const QString& host, quint16 port, QStringView gps_type)
{
    int manufacturerId = 0;
    const GPSType type = _resolveGPSType(gps_type, manufacturerId);
    qCDebug(GPSRtkLog) << "Connecting network GPS base" << host << port << gps_type;

    auto requestStop = std::make_shared<std::atomic_bool>(false);
    GPSProvider::TransportFactory makeTransport = [host, port, requestStop]() -> std::unique_ptr<GPSTransport> {
        return std::make_unique<TcpGPSTransport>(host, port, requestStop);
    };
    _startProvider(std::move(makeTransport), std::move(requestStop), type, manufacturerId);
}

void GPSRtk::_startProvider(GPSProvider::TransportFactory makeTransport, std::shared_ptr<std::atomic_bool> requestStop,
                           GPSType type, int manufacturerId)
{
    RTKSettings* const rtkSettings = SettingsManager::instance()->rtkSettings();
    rtkSettings->baseReceiverManufacturers()->setRawValue(manufacturerId);

    disconnectGPS();

    // Adopt the new provider's flag only after disconnectGPS() has signalled the prior
    // provider to stop via its own (now distinct) flag.
    _requestGpsStop = std::move(requestStop);
    _gpsRtkFactGroup->lastError()->setRawValue(static_cast<int>(GPSConnectionError::None));
    const bool useFixedBase =
        static_cast<BaseModeDefinition::Mode>(rtkSettings->useFixedBasePosition()->rawValue().toInt()) ==
        BaseModeDefinition::Mode::BaseFixed;
    const GPSReceiverConfig rtkConfig = {
        .useFixedBase = useFixedBase,
        .surveyInAccMeters = rtkSettings->surveyInAccuracyLimit()->rawValue().toDouble(),
        .surveyInDurationSecs = rtkSettings->surveyInMinObservationDuration()->rawValue().toInt(),
        .fixedBaseLatitude = rtkSettings->fixedBasePositionLatitude()->rawValue().toDouble(),
        .fixedBaseLongitude = rtkSettings->fixedBasePositionLongitude()->rawValue().toDouble(),
        .fixedBaseAltitudeMeters = rtkSettings->fixedBasePositionAltitude()->rawValue().toFloat(),
        .fixedBaseAccuracyMeters = rtkSettings->fixedBasePositionAccuracy()->rawValue().toFloat(),
    };
    _gpsProvider = new GPSProvider(std::move(makeTransport), type, rtkConfig, _requestGpsStop, this);
    // Forward corrections through the GPSManager-injected shared RTCMMavlink so
    // serial and NTRIP sources share one GPS_RTCM_DATA sequence-id domain.
    if (_rtcmMavlink) {
        (void) connect(_gpsProvider, &GPSProvider::RTCMDataUpdate, _rtcmMavlink, &RTCMMavlink::RTCMDataUpdate);
    } else {
        qCWarning(GPSRtkLog) << "Shared RTCMMavlink unavailable; RTK corrections will not be forwarded";
    }
    (void) connect(_gpsProvider, &GPSProvider::satelliteInfoUpdate, this, &GPSRtk::_satelliteInfoUpdate);
    (void) connect(_gpsProvider, &GPSProvider::sensorGpsUpdate, this, &GPSRtk::_sensorGpsUpdate);
    (void) connect(_gpsProvider, &GPSProvider::surveyInStatus, this, &GPSRtk::_onGPSSurveyInStatus);
    (void) connect(_gpsProvider, &GPSProvider::connectionError, this, &GPSRtk::_onGPSConnectionError);
    (void) connect(_gpsProvider, &GPSProvider::connectionEstablished, this, &GPSRtk::_onGPSConnect);
    (void) connect(_gpsProvider, &GPSProvider::finished, this, &GPSRtk::_onGPSDisconnect);

    // Start the thread only after every signal is wired, so no early emission is lost.
    (void) QMetaObject::invokeMethod(_gpsProvider, "start", Qt::AutoConnection);
}

void GPSRtk::disconnectGPS()
{
    if (_gpsProvider) {
        if (_requestGpsStop) {
            *_requestGpsStop = true;
        }
        if (_gpsProvider->wait(kGPSThreadDisconnectTimeout)) {
            _gpsProvider->deleteLater();
        } else {
            qCWarning(GPSRtkLog) << "GPS thread did not exit in time; deferring cleanup to finished()";
            // Sever ALL of the provider's connections (including RTCMDataUpdate -> shared
            // RTCMMavlink), not just those to this, so an abandoned provider can't keep
            // injecting corrections or flipping facts after a reconnect.
            (void) _gpsProvider->disconnect();
            (void) connect(_gpsProvider, &QThread::finished, _gpsProvider, &QObject::deleteLater);
        }
        _gpsProvider = nullptr;
    }

    // Drop our handle to the flag; an abandoned provider/transport keeps its own copy
    // alive (shared ownership), so its reference never dangles.
    _requestGpsStop.reset();

    _setConnected(false);
}

bool GPSRtk::connected() const
{
    return _connected;
}

FactGroup* GPSRtk::gpsRtkFactGroup()
{
    return _gpsRtkFactGroup;
}

GPSRtk::SatelliteCounts GPSRtk::countSatellites(const satellite_info_s& msg)
{
    SatelliteCounts counts;
    counts.inView = qMin(msg.count, satellite_info_s::SAT_INFO_MAX_SATELLITES);
    for (uint8_t i = 0; i < counts.inView; ++i) {
        if (msg.used[i]) {
            ++counts.used;
        }
    }
    return counts;
}

void GPSRtk::_satelliteInfoUpdate(const satellite_info_s& msg)
{
    const SatelliteCounts counts = countSatellites(msg);
    qCDebug(GPSRtkLog) << Q_FUNC_INFO << QStringLiteral("%1 in view, %2 used").arg(counts.inView).arg(counts.used);
    _gpsRtkFactGroup->numSatellites()->setRawValue(counts.inView);
    _gpsRtkFactGroup->numSatellitesUsed()->setRawValue(counts.used);
    _gpsRtkFactGroup->satelliteModel()->updateFromDriverInfo(msg);
}

void GPSRtk::_sensorGpsUpdate(const sensor_gps_s& msg)
{
    qCDebug(GPSRtkLog) << Q_FUNC_INFO
                       << QStringLiteral("alt=%1, long=%2, lat=%3")
                              .arg(msg.altitude_msl_m)
                              .arg(msg.longitude_deg)
                              .arg(msg.latitude_deg);
}
