#include "GPSManager.h"
#include "GPSRtk.h"
#include "NTRIPManager.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QApplicationStatic>

QGC_LOGGING_CATEGORY(GPSManagerLog, "GPS.GPSManager")

Q_APPLICATION_STATIC(GPSManager, _gpsManager);

GPSManager::GPSManager(QObject *parent)
    : QObject(parent)
    , _gpsRtk(new GPSRtk(this))
    , _ntripManager(new NTRIPManager(this))
{
    qCDebug(GPSManagerLog) << this;
}

GPSManager::~GPSManager()
{
    qCDebug(GPSManagerLog) << this;
}

GPSManager *GPSManager::instance()
{
    return _gpsManager();
}

void GPSManager::init()
{
    _ntripManager->init();
    // Coordinator-level wiring: both RTK and NTRIP corrections flow through the
    // same RTCMMavlink so they share one GPS_RTCM_DATA sequence-id domain.
    _gpsRtk->setRtcmMavlink(_ntripManager->rtcmMavlink());
}

void GPSManager::connectSerialRTK(const QString &device, QStringView gpsType)
{
    _gpsRtk->connectGPS(device, gpsType);
}

void GPSManager::connectNetworkRTK(const QString &host, quint16 port, QStringView gpsType)
{
    _gpsRtk->connectGPS(host, port, gpsType);
}

void GPSManager::disconnectRTK()
{
    _gpsRtk->disconnectGPS();
}

bool GPSManager::rtkConnected() const
{
    return _gpsRtk->connected();
}

FactGroup *GPSManager::gpsRtkFactGroup()
{
    return _gpsRtk->gpsRtkFactGroup();
}
