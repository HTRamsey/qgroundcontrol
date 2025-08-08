/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtBluetooth/QLowEnergyDescriptor>
#include <QtCore/QLoggingCategory>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include "LinkConfiguration.h"
#include "LinkInterface.h"

Q_DECLARE_LOGGING_CATEGORY(BleLinkLog)

/*===========================================================================*/

struct BleDevice
{
    QString name;
#ifdef Q_OS_IOS
    QUuid uuid;
#else
    QBluetoothAddress address;
#endif
    bool operator==(const BleDevice &o) const {
#ifdef Q_OS_IOS
        return name == o.name && uuid == o.uuid;
#else
        return name == o.name && address == o.address;
#endif
    }
};

/*===========================================================================*/

class BleConfiguration : public LinkConfiguration
{
    Q_OBJECT
    Q_PROPERTY(QString     deviceName READ deviceName NOTIFY deviceChanged)
    Q_PROPERTY(QString     address    READ address    NOTIFY deviceChanged)
    Q_PROPERTY(QStringList nameList   READ nameList   NOTIFY nameListChanged)
    Q_PROPERTY(bool        scanning   READ scanning   NOTIFY scanningChanged)

public:
    explicit BleConfiguration(const QString &name, QObject *parent = nullptr);
    explicit BleConfiguration(const BleConfiguration *copy, QObject *parent = nullptr);
    ~BleConfiguration() override;

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void stopScan() const;
    Q_INVOKABLE void setDevice(const QString &name);

    LinkType type() const override { return LinkConfiguration::TypeBluetooth; } // reuse BT type bucket
    void copyFrom(const LinkConfiguration *source) override;
    void loadSettings(QSettings &settings, const QString &root) override;
    void saveSettings(QSettings &settings, const QString &root) const override;
    QString settingsURL() const override { return QStringLiteral("BleSettings.qml"); }
    QString settingsTitle() const override;

    const BleDevice &device() const { return _device; }
    QString deviceName() const { return _device.name; }
#ifdef Q_OS_IOS
    QString address() const { return QString(); }
#else
    QString address() const { return _device.address.toString(); }
#endif
    QStringList nameList() const { return _nameList; }
    bool scanning() const;

signals:
    void deviceChanged();
    void nameListChanged();
    void scanningChanged();
    void errorOccurred(const QString &errorString);

private slots:
    void _deviceDiscovered(const QBluetoothDeviceInfo &info);
    void _onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    void _initAgent();

    QList<BleDevice> _devices;
    BleDevice _device{};
    QStringList _nameList;
    QBluetoothDeviceDiscoveryAgent *_agent = nullptr;
};

/*===========================================================================*/

class BleWorker : public QObject
{
    Q_OBJECT
public:
    explicit BleWorker(const BleConfiguration *cfg, QObject *parent = nullptr);
    ~BleWorker() override;

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &errorString);
    void dataReceived(const QByteArray &data);
    void dataSent(const QByteArray &data);

public slots:
    void setup();
    void connectLink();
    void disconnectLink();
    void writeData(const QByteArray &data);

private slots:
    void _ctrlStateChanged(QLowEnergyController::ControllerState s);
    void _ctrlErrorOccurred(QLowEnergyController::Error e);
    void _serviceDiscovered(const QBluetoothUuid &uuid);
    void _discoveryFinished();

    void _serviceStateChanged(QLowEnergyService::ServiceState s);
    void _serviceError(QLowEnergyService::ServiceError e);
    void _charChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);

private:
    bool _selectRxTx(QLowEnergyService *svc);
    void _enableNotify(QLowEnergyService *svc, const QLowEnergyCharacteristic &rx);
    int  _maxWritePayload() const; // ATT_MTU - 3, default 20

    const BleConfiguration *_cfg = nullptr;

    QLowEnergyController *_ctl = nullptr;
    QList<QBluetoothUuid> _servicesFound;

    QPointer<QLowEnergyService> _svc = nullptr;
    QLowEnergyCharacteristic _rxChar;
    QLowEnergyCharacteristic _txChar;
    QLowEnergyDescriptor _rxCccd;

    int _mtu = 23; // default ATT MTU
    bool _connected = false;
};

/*===========================================================================*/

class BleLink : public LinkInterface
{
    Q_OBJECT
public:
    explicit BleLink(SharedLinkConfigurationPtr &config, QObject *parent = nullptr);
    ~BleLink() override;

    bool isConnected() const override;
    bool isSecureConnection() const override { return false; }

public slots:
    void disconnect() override;

private slots:
    void _onConnected();
    void _onDisconnected();
    void _onError(const QString &error);
    void _onRx(const QByteArray &data);
    void _onTx(const QByteArray &data);

private:
    bool _connect() override;
    void _writeBytes(const QByteArray &bytes) override;

    void _checkPermission();
    void _handlePermissionStatus(Qt::PermissionStatus st);

    const BleConfiguration *_cfg = nullptr;
    BleWorker *_worker = nullptr;
    QThread *_thread = nullptr;
};
