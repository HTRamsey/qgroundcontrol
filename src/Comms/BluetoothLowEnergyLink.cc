/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "BluetoothLowEnergyLink.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QPermissions>

QGC_LOGGING_CATEGORY(BleLinkLog, "qgc.comms.blelink")

/*===================== BleConfiguration =====================*/

BleConfiguration::BleConfiguration(const QString &name, QObject *parent)
    : LinkConfiguration(name, parent)
    , _agent(new QBluetoothDeviceDiscoveryAgent(this))
{
    qCDebug(BleLinkLog) << this;

    _initAgent();
}

BleConfiguration::BleConfiguration(const BleConfiguration *copy, QObject *parent)
    : LinkConfiguration(copy, parent)
    , _device(copy->_device)
    , _agent(new QBluetoothDeviceDiscoveryAgent(this))
{
    qCDebug(BleLinkLog) << this;

    copyFrom(copy);
    _initAgent();
}

BleConfiguration::~BleConfiguration()
{
    stopScan();

    qCDebug(BleLinkLog) << this;
}

void BleConfiguration::_initAgent()
{
    _agent->setLowEnergyDiscoveryTimeout(5000);
    // _agent->setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
    (void) connect(_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BleConfiguration::_deviceDiscovered);
    (void) connect(_agent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &BleConfiguration::scanningChanged);
    (void) connect(_agent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BleConfiguration::scanningChanged);
    (void) connect(_agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &BleConfiguration::_onScanError);
}

QString BleConfiguration::settingsTitle() const
{
    return tr("BLE Link Settings");
}

void BleConfiguration::startScan()
{
    _devices.clear();
    _nameList.clear(); emit nameListChanged();

    // Only look for LE devices
    _agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    emit scanningChanged();
}

void BleConfiguration::stopScan() const
{
    if (_agent->isActive()) _agent->stop();
}

bool BleConfiguration::scanning() const
{
    return _agent->isActive();
}

void BleConfiguration::setDevice(const QString &name)
{
    for (const auto& d : _devices) {
        if (d.name == name) {
            _device = d;
            emit deviceChanged();
            return;
        }
    }
}

void BleConfiguration::copyFrom(const LinkConfiguration *source)
{
    Q_ASSERT(source);
    LinkConfiguration::copyFrom(source);
    const auto* o = qobject_cast<const BleConfiguration*>(source);
    Q_ASSERT(o);
    _device = o->_device;
    emit deviceChanged();
}

void BleConfiguration::loadSettings(QSettings &s, const QString &root)
{
    s.beginGroup(root);
    _device.name = s.value("deviceName", _device.name).toString();
#ifdef Q_OS_IOS
    _device.uuid = QUuid(s.value("uuid", _device.uuid.toString()).toString());
#else
    _device.address = QBluetoothAddress(s.value("address", _device.address.toString()).toString());
#endif
    s.endGroup();
}

void BleConfiguration::saveSettings(QSettings &s, const QString &root) const
{
    s.beginGroup(root);
    s.setValue("deviceName", _device.name);
#ifdef Q_OS_IOS
    s.setValue("uuid", _device.uuid.toString());
#else
    s.setValue("address", _device.address.toString());
#endif
    s.endGroup();
}

void BleConfiguration::_deviceDiscovered(const QBluetoothDeviceInfo &info)
{
    // Filter: LE devices with name
    if (!info.name().isEmpty() && info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        BleDevice d;
        d.name = info.name();
#ifdef Q_OS_IOS
        d.uuid = info.deviceUuid();
#else
        d.address = info.address();
#endif
        if (!_devices.contains(d)) {
            _devices.append(d);
            _nameList.append(d.name);
            emit nameListChanged();
        }
    }
}

void BleConfiguration::_onScanError(QBluetoothDeviceDiscoveryAgent::Error)
{
    emit errorOccurred(_agent->errorString());
}

/*===================== BleWorker =====================*/

BleWorker::BleWorker(const BleConfiguration *cfg, QObject *parent)
    : QObject(parent)
    , _cfg(cfg)
{
    qCDebug(BleLinkLog) << this;
}

BleWorker::~BleWorker()
{
    disconnectLink();

    qCDebug(BleLinkLog) << this;
}

bool BleWorker::isConnected() const
{
    return _connected;
}

void BleWorker::setup()
{
    // nothing heavy here; controller is created on connect
}

void BleWorker::connectLink()
{
    if (_connected) {
        return;
    }

#ifdef Q_OS_IOS
    if (!_cfg->device().uuid.isNull()) {
        _ctl = QLowEnergyController::createCentral(_cfg->device().uuid, this);
    } else {
        emit errorOccurred(tr("No BLE device selected"));
        return;
    }
#else
    if (!_cfg->device().address.isNull()) {
        _ctl = QLowEnergyController::createCentral(_cfg->device().address, this);
    } else {
        emit errorOccurred(tr("No BLE device selected"));
        return;
    }
#endif
    _ctl->setRemoteName(_cfg->deviceName());

    (void) connect(_ctl, &QLowEnergyController::stateChanged, this, &BleWorker::_ctrlStateChanged);
    (void) connect(_ctl, &QLowEnergyController::errorOccurred, this, &BleWorker::_ctrlErrorOccurred);
    (void) connect(_ctl, &QLowEnergyController::serviceDiscovered, this, &BleWorker::_serviceDiscovered);
    (void) connect(_ctl, &QLowEnergyController::discoveryFinished, this, &BleWorker::_discoveryFinished);
    (void) connect(_ctl, &QLowEnergyController::mtuChanged, this, [this](int mtu) {
        _mtu = mtu;
    });

    _servicesFound.clear();
    _ctl->connectToDevice();
}

void BleWorker::disconnectLink()
{
    if (_svc) {
        if (_rxCccd.isValid()) {
            _svc->writeDescriptor(_rxCccd, QByteArray::fromHex("0000")); // disable notify
        }
        _svc->deleteLater();
        _svc = nullptr;
    }
    if (_ctl) {
        _ctl->disconnectFromDevice();
        _ctl->deleteLater();
        _ctl = nullptr;
    }
    _connected = false;
    emit disconnected();
}

void BleWorker::_ctrlStateChanged(QLowEnergyController::ControllerState s)
{
    switch (s) {
    case QLowEnergyController::ConnectedState:
        _ctl->discoverServices();
        break;
    case QLowEnergyController::DiscoveredState:
        // handled by discoveryFinished
        break;
    default:
        break;
    }
}

void BleWorker::_ctrlErrorOccurred(QLowEnergyController::Error)
{
    emit errorOccurred(_ctl ? _ctl->errorString() : QStringLiteral("BLE controller error"));
}

void BleWorker::_serviceDiscovered(const QBluetoothUuid &uuid)
{
    _servicesFound.append(uuid);
}

void BleWorker::_discoveryFinished()
{
    if (_servicesFound.isEmpty()) {
        emit errorOccurred(QStringLiteral("No GATT services found"));
        disconnectLink();
        return;
    }

    // Prefer Nordic NUS if present
    static const QBluetoothUuid kNusService(QStringLiteral("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
    QBluetoothUuid chosen = _servicesFound.contains(kNusService) ? kNusService : _servicesFound.constFirst();

    _svc = _ctl->createServiceObject(chosen, this);
    if (!_svc) {
        emit errorOccurred(QStringLiteral("Failed to create service object"));
        disconnectLink();
        return;
    }

    (void) connect(_svc, &QLowEnergyService::stateChanged, this, &BleWorker::_serviceStateChanged);
    (void) connect(_svc, &QLowEnergyService::errorOccurred, this, &BleWorker::_serviceError);
    (void) connect(_svc, &QLowEnergyService::characteristicChanged, this, &BleWorker::_charChanged);

    _svc->discoverDetails();
}

void BleWorker::_serviceStateChanged(QLowEnergyService::ServiceState s)
{
    if (s == QLowEnergyService::RemoteServiceDiscovered) {
        if (!_selectRxTx(_svc)) {
            emit errorOccurred(QStringLiteral("No suitable RX/TX characteristics"));
            disconnectLink();
            return;
        }
        _enableNotify(_svc, _rxChar);
        _connected = true;
        emit connected();
    }
}

void BleWorker::_serviceError(QLowEnergyService::ServiceError)
{
    emit errorOccurred(_svc ? _svc->error() : QStringLiteral("BLE service error"));
}

static bool _hasProp(const QLowEnergyCharacteristic &c, QLowEnergyCharacteristic::PropertyType p)
{
    return (c.properties() & p) == p;
}

bool BleWorker::_selectRxTx(QLowEnergyService *svc)
{
    // Try Nordic NUS first (common in the wild)
    const QLowEnergyCharacteristic nusRx = svc->characteristic(QBluetoothUuid(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e"))); // NUS RX: notify
    const QLowEnergyCharacteristic nusTx = svc->characteristic(QBluetoothUuid(QStringLiteral("6e400002-b5a3-f393-e0a9-e50e24dcca9e"))); // NUS TX: write/wwr
    if (nusRx.isValid() && nusTx.isValid() && _hasProp(nusRx, QLowEnergyCharacteristic::Notify)
        && (_hasProp(nusTx, QLowEnergyCharacteristic::WriteNoResponse) || _hasProp(nusTx, QLowEnergyCharacteristic::Write))) {
        _rxChar = nusRx;
        _txChar = nusTx;
        return true;
    }

    // Generic fallback: first (Notify or Indicate) + first (Write or WriteNoResp)
    QLowEnergyCharacteristic rx, tx;
    const auto chars = svc->characteristics();
    for (const auto &c : chars) {
        if (!rx.isValid() && (_hasProp(c, QLowEnergyCharacteristic::Notify) || _hasProp(c, QLowEnergyCharacteristic::Indicate))) {
            rx = c;
        }
        if (!tx.isValid() && (_hasProp(c, QLowEnergyCharacteristic::WriteNoResponse) || _hasProp(c, QLowEnergyCharacteristic::Write))) {
            tx = c;
        }
        if (rx.isValid() && tx.isValid()) {
            break;
        }
    }
    if (rx.isValid() && tx.isValid()) {
        _rxChar = rx; _txChar = tx;
        return true;
    }
    return false;
}

void BleWorker::_enableNotify(QLowEnergyService *svc, const QLowEnergyCharacteristic &rx)
{
    _rxCccd = rx.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
    if (_rxCccd.isValid()) {
        // Enable notifications
        svc->writeDescriptor(_rxCccd, QByteArray::fromHex("0100"));
    }
}

int BleWorker::_maxWritePayload() const
{
    // ATT MTU payload is (MTU - 3). If we don't know MTU yet, assume 20.
    const int pdu = (_mtu > 3) ? (_mtu - 3) : 20;
    return qBound(1, pdu, 244); // keep it sane
}

void BleWorker::writeData(const QByteArray &data)
{
    if (!_svc || !_txChar.isValid()) {
        emit errorOccurred(QStringLiteral("BLE not ready"));
        return;
    }
    const int chunk = _maxWritePayload();
    int off = 0;
    const bool useNoResp = _hasProp(_txChar, QLowEnergyCharacteristic::WriteNoResponse);
    while (off < data.size()) {
        const QByteArray part = data.mid(off, chunk);
        _svc->writeCharacteristic(_txChar, part,
                                  useNoResp ? QLowEnergyService::WriteWithoutResponse
                                            : QLowEnergyService::WriteWithResponse);
        off += part.size();
    }
    emit dataSent(data);
}

void BleWorker::_charChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    if (c == _rxChar && !value.isEmpty()) {
        emit dataReceived(value);
    }
}

/*===================== BleLink =====================*/

BleLink::BleLink(SharedLinkConfigurationPtr &config, QObject *parent)
    : LinkInterface(config, parent)
    , _cfg(qobject_cast<const BleConfiguration*>(config.get()))
    , _worker(new BleWorker(_cfg))
    , _thread(new QThread(this))
{
    qCDebug(BleLinkLog) << this;

    _checkPermission();

    _thread->setObjectName(QStringLiteral("BLE_%1").arg(_cfg->name()));
    _worker->moveToThread(_thread);

    (void) connect(_thread, &QThread::started, _worker, &BleWorker::setup);
    (void) connect(_thread, &QThread::finished, _worker, &QObject::deleteLater);

    (void) connect(_worker, &BleWorker::connected, this, &BleLink::_onConnected, Qt::QueuedConnection);
    (void) connect(_worker, &BleWorker::disconnected, this, &BleLink::_onDisconnected, Qt::QueuedConnection);
    (void) connect(_worker, &BleWorker::errorOccurred, this, &BleLink::_onError, Qt::QueuedConnection);
    (void) connect(_worker, &BleWorker::dataReceived, this, &BleLink::_onRx, Qt::QueuedConnection);
    (void) connect(_worker, &BleWorker::dataSent, this, &BleLink::_onTx, Qt::QueuedConnection);

    _thread->start();
}

BleLink::~BleLink()
{
    (void) QMetaObject::invokeMethod(_worker, "disconnectLink", Qt::BlockingQueuedConnection);
    _thread->quit();
    (void) _thread->wait();

    qCDebug(BleLinkLog) << this;
}

bool BleLink::isConnected() const
{
    return _worker->isConnected();
}

bool BleLink::_connect()
{
    return QMetaObject::invokeMethod(_worker, "connectLink", Qt::QueuedConnection);
}

void BleLink::disconnect()
{
    (void) QMetaObject::invokeMethod(_worker, "disconnectLink", Qt::QueuedConnection);
}

void BleLink::_writeBytes(const QByteArray& bytes)
{
    (void) QMetaObject::invokeMethod(_worker, "writeData", Qt::QueuedConnection, Q_ARG(QByteArray, bytes));
}

void BleLink::_onConnected()
{
    emit connected();
}

void BleLink::_onDisconnected()
{
    emit disconnected();
}

void BleLink::_onRx(const QByteArray &d)
{
    emit bytesReceived(this, d);
}

void BleLink::_onTx(const QByteArray &d)
{
    emit bytesSent(this, d);
}

void BleLink::_onError(const QString &e)
{
    qCWarning(BleLinkLog) << "BLE error:" << e;
    emit communicationError(tr("BLE Link Error"), tr("Link %1: %2").arg(_cfg->name(), e));
}

void BleLink::_checkPermission()
{
    QBluetoothPermission p;
    p.setCommunicationModes(QBluetoothPermission::Access);

    const auto st = QCoreApplication::instance()->checkPermission(p);
    if (st == Qt::PermissionStatus::Undetermined) {
        QCoreApplication::instance()->requestPermission(p, this, [this](const QPermission& perm){
            _handlePermissionStatus(perm.status());
        });
    } else {
        _handlePermissionStatus(st);
    }
}

void BleLink::_handlePermissionStatus(Qt::PermissionStatus st)
{
    if (st != Qt::PermissionStatus::Granted) {
        _onError(QStringLiteral("Bluetooth Permission Denied"));
        _onDisconnected();
    }
}
