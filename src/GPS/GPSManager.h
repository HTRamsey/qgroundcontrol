#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

class GPSRtk;
class NTRIPManager;
class FactGroup;

/// Single entry point for the GCS-attached GPS/RTK/NTRIP subsystem. Owns the
/// RTK provider (GPSRtk) and the NTRIP caster client (NTRIPManager), and exposes
/// a small RTK facade so callers depend on GPSManager rather than GPSRtk internals.
class GPSManager : public QObject
{
    Q_OBJECT

public:
    GPSManager(QObject *parent = nullptr);
    ~GPSManager();

    static GPSManager *instance();

    /// Post-construction init (call after SettingsManager is ready). Brings up NTRIP.
    void init();

    GPSRtk *gpsRtk() { return _gpsRtk; }
    NTRIPManager *ntripManager() { return _ntripManager; }

    void connectSerialRTK(const QString &device, QStringView gpsType);
    void connectNetworkRTK(const QString &host, quint16 port, QStringView gpsType);
    void disconnectRTK();
    bool rtkConnected() const;
    FactGroup *gpsRtkFactGroup();

private:
    GPSRtk *_gpsRtk = nullptr;
    NTRIPManager *_ntripManager = nullptr;
};
