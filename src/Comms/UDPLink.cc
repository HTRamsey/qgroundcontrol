/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "UDPLink.h"
#include "QGCApplication.h"
#include "SettingsManager.h"
#include "AutoConnectSettings.h"
#include <QGCLoggingCategory.h>

#include <QtCore/QList>
#include <QtCore/QMutexLocker>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkDatagram>

QGC_LOGGING_CATEGORY(UDPLinkLog, "ENABLED.comms.udplink")

static bool containsTarget(const QList<std::shared_ptr<UDPClient>> &list, const QHostAddress &address, quint16 port)
{
    for (const std::shared_ptr<UDPClient> &target : list) {
        if ((target->address == address) && (target->port == port)) {
            return true;
        }
    }

    return false;
}

UDPLink::UDPLink(SharedLinkConfigurationPtr &config, QObject* parent)
    : LinkInterface(config, parent)
    , m_socket(new QUdpSocket(this))
    , m_udpConfig(qobject_cast<UDPConfiguration*>(config.get()))
    , m_localAddresses(QNetworkInterface::allAddresses())
{
    qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;

    if (!m_udpConfig) {
        qCCritical(UDPLinkLog) << "Internal error";
    }

    (void) connect(m_socket, &QUdpSocket::stateChanged, this, [this](QUdpSocket::SocketState state) {
        switch (state) {
            case QUdpSocket::SocketState::BoundState:
                qCDebug(UDPLinkLog) << m_socket->localAddress() << m_socket->localPort();
                break;

            default:
                break;
        }

        qCDebug(UDPLinkLog) << state;
    });

    (void) connect(m_socket, &QUdpSocket::errorOccurred, this, [this](QUdpSocket::SocketError error) {
        qCDebug(UDPLinkLog) << error << m_socket->errorString();
        emit communicationError(QStringLiteral("UDP Link Error"), m_socket->errorString());
    });

    (void) QObject::connect(m_socket, &QUdpSocket::readyRead, this, &UDPLink::_readBytes);

    m_socket->setProxy(QNetworkProxy::NoProxy);
}

UDPLink::~UDPLink()
{
    m_socket->close();

    #ifdef QGC_ZEROCONF_ENABLED
        _deregisterZeroconf();
    #endif

    m_sessionTargets.clear();

    qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

bool UDPLink::_isIpLocal(const QHostAddress &address)
{
    if (address.isLoopback()) {
        return true;
    }

    return m_localAddresses.contains(address);
}

void UDPLink::_writeBytes(const QByteArray data)
{
    QMutexLocker locker(&m_sessionTargetsMutex);

    // Send to all manually targeted systems
    for (const std::shared_ptr<UDPClient> &target : m_udpConfig->targetHosts()) {
        if (!containsTarget(m_sessionTargets, target->address, target->port)) {
            _writeDataGram(data, target);
        }
    }

    // Send to all connected systems
    for (const std::shared_ptr<UDPClient> &target: m_sessionTargets) {
        _writeDataGram(data, target);
    }

    emit bytesSent(this, data);
}

void UDPLink::_writeDataGram(const QByteArray &data, const std::shared_ptr<UDPClient> target)
{
    if (m_socket->writeDatagram(data, target->address, target->port) < 0) {
        qCWarning(UDPLinkLog) << Q_FUNC_INFO << "Error writing to" << target->address << target->port;
    }
}

void UDPLink::_readBytes()
{
    QByteArray databuffer;
    QElapsedTimer timer;
    timer.start();
    while (m_socket->hasPendingDatagrams()) {
        const QNetworkDatagram datagramIn = m_socket->receiveDatagram();

        if (datagramIn.isNull() || datagramIn.data().isEmpty()) {
            continue;
        }

        // qCInfo(UDPLinkLog) << Q_FUNC_INFO << datagramIn.senderAddress() << datagramIn.senderPort() << datagramIn.data().size() << "bytes on interface" << datagramIn.interfaceIndex();

        (void) databuffer.append(datagramIn.data());

        if (databuffer.size() > (10 * 1024) || (timer.elapsed() > 50)) {
            emit bytesReceived(this, databuffer);
            databuffer.clear();
            (void) timer.restart();
        }

        const QHostAddress asender = _isIpLocal(datagramIn.senderAddress()) ? QHostAddress(QHostAddress::SpecialAddress::LocalHost) : datagramIn.senderAddress();

        QMutexLocker locker(&m_sessionTargetsMutex);
        if (!containsTarget(m_sessionTargets, asender, datagramIn.senderPort())) {
            qCDebug(UDPLinkLog) << Q_FUNC_INFO << "Adding target" << asender << datagramIn.senderPort();
            (void) m_sessionTargets.append(std::make_shared<UDPClient>(asender, datagramIn.senderPort()));
        }
        locker.unlock();
    }

    // Send Remaining Data
    if (databuffer.size()) {
        emit bytesReceived(this, databuffer);
    }
}

void UDPLink::disconnect()
{
    m_socket->close();
    m_connectState = false;

    emit disconnected();
}

bool UDPLink::_connect()
{
    const QHostAddress host = QHostAddress::AnyIPv4;
    m_connectState = m_socket->bind(host, m_udpConfig->localPort(), QAbstractSocket::ReuseAddressHint | QUdpSocket::ShareAddress);

    if (m_connectState) {
        m_socket->joinMulticastGroup(QHostAddress("224.0.0.1"));

        #ifdef __mobile__
            m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 64 * 1024);
            m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 128 * 1024);
        #else
            m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 256 * 1024);
            m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 512 * 1024);
            // m_socket->setReadBufferSize(512 * 1024);
            // m_socket->setSocketOption(QAbstractSocket::PathMtuSocketOption, 1500);
            // m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
        #endif

        #ifdef QGC_ZEROCONF_ENABLED
            _registerZeroconf(m_udpConfig->localPort(), "_qgroundcontrol._udp");
        #endif

        emit connected();
    } else {
        emit communicationError(QStringLiteral("UDP Link Error"), QStringLiteral("Error binding UDP port: %1").arg(m_socket->errorString()));
    }

    return m_connectState;
}

#ifdef QGC_ZEROCONF_ENABLED
void UDPLink::_registerZeroconf(uint16_t port, const QString &regType)
{
    qCDebug(UDPLinkLog) << Q_FUNC_INFO;

    const DNSServiceErrorType result = DNSServiceRegister(
        &m_dnssServiceRef, 0, 0, 0,
        regType.toStdString().c_str(),
        NULL,
        NULL,
        htons(port),
        0,
        NULL,
        NULL,
        NULL
    );

    if (result != kDNSServiceErr_NoError) {
        m_dnssServiceRef = NULL;
        emit communicationError(QStringLiteral("UDP Link Error"), QStringLiteral("Error registering Zeroconf"));
    }
}

void UDPLink::_deregisterZeroconf()
{
    qCDebug(UDPLinkLog) << Q_FUNC_INFO;

    if (m_dnssServiceRef) {
        DNSServiceRefDeallocate(m_dnssServiceRef);
        m_dnssServiceRef = NULL;
    }
}
#endif // QGC_ZEROCONF_ENABLED

////////////////////////////////////////////////////////////////////

UDPConfiguration::UDPConfiguration(const QString &name, QObject* parent)
    : LinkConfiguration(name, parent)
{
    AutoConnectSettings* const settings = qgcApp()->toolbox()->settingsManager()->autoConnectSettings();
    m_localPort = settings->udpListenPort()->rawValue().toInt();
    const QString targetHostIP = settings->udpTargetHostIP()->rawValue().toString();
    if (!targetHostIP.isEmpty()) {
        const quint16 targetHostPort = settings->udpTargetHostPort()->rawValue().toUInt();
        addHost(targetHostIP, targetHostPort);
    }
}

UDPConfiguration::UDPConfiguration(UDPConfiguration* source)
    : LinkConfiguration(source)
{
    qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;

    _copyFrom(source);
}

UDPConfiguration::~UDPConfiguration()
{
    m_targetHosts.clear();

    qCDebug(UDPLinkLog) << Q_FUNC_INFO << this;
}

void UDPConfiguration::copyFrom(LinkConfiguration* source)
{
    LinkConfiguration::copyFrom(source);
    _copyFrom(source);
}

void UDPConfiguration::_copyFrom(LinkConfiguration* source)
{
    const UDPConfiguration* udpSource = qobject_cast<UDPConfiguration*>(source);
    if (!udpSource) {
        qCWarning(UDPLinkLog) << Q_FUNC_INFO << "Internal error";
        return;
    }

    m_localPort = udpSource->localPort();
    m_targetHosts.clear();

    for (const std::shared_ptr<UDPClient> &target : udpSource->targetHosts()) {
        if (!containsTarget(m_targetHosts, target->address, target->port)) {
            (void) m_targetHosts.append(std::make_shared<UDPClient>(target.get()));
            _updateHostList();
        }
    }
}

void UDPConfiguration::addHost(const QString &host)
{
    if (host.contains(":")) {
        const QStringList hostInfo = host.split(":");
        addHost(hostInfo.constFirst(), hostInfo.constLast().toUInt());
    } else {
        addHost(host, m_localPort);
    }
}

void UDPConfiguration::addHost(const QString &host, quint16 port)
{
    const QString ipAdd = _getIpAddress(host);
    if (ipAdd.isEmpty()) {
        qCWarning(UDPLinkLog) << Q_FUNC_INFO << "Could not resolve host:" << host << "port:" << port;
        return;
    }

    const QHostAddress address(ipAdd);
    if (!containsTarget(m_targetHosts, address, port)) {
        (void) m_targetHosts.append(std::make_shared<UDPClient>(address, port));
        _updateHostList();
    }
}

void UDPConfiguration::removeHost(const QString &host)
{
    if (host.contains(":")) {
        const QHostAddress address = QHostAddress(_getIpAddress(host.split(":").constFirst()));
        const quint16 port = host.split(":").constLast().toUInt();
        for (uint32_t i = 0; i < m_targetHosts.size(); i++) {
            std::shared_ptr<UDPClient> target = m_targetHosts.at(i);
            if ((target->address == address) && (target->port == port)) {
                m_targetHosts.removeAt(i);
                target.reset();
                _updateHostList();
                return;
            }
        }
    }

    _updateHostList();

    qCWarning(UDPLinkLog) << Q_FUNC_INFO << "Could not remove unknown host:" << host;
}

void UDPConfiguration::saveSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);

    settings.setValue("hostCount", m_targetHosts.size());
    settings.setValue("port", m_localPort);
    for (uint32_t i = 0; i < m_targetHosts.size(); i++) {
        const std::shared_ptr<UDPClient> target = m_targetHosts.at(i);
        const QString hkey = QString("host%1").arg(i);
        settings.setValue(hkey, target->address.toString());
        const QString pkey = QString("port%1").arg(i);
        settings.setValue(pkey, target->port);
    }

    settings.endGroup();
}

void UDPConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    m_targetHosts.clear();

    settings.beginGroup(root);

    AutoConnectSettings* const autoConnectSettings = qgcApp()->toolbox()->settingsManager()->autoConnectSettings();
    m_localPort = static_cast<quint16>(settings.value("port", autoConnectSettings->udpListenPort()->rawValue().toUInt()).toUInt());

    const uint32_t hostCount = settings.value("hostCount", 0).toUInt();
    for (uint32_t i = 0; i < hostCount; i++) {
        const QString hkey = QString("host%1").arg(i);
        const QString pkey = QString("port%1").arg(i);
        if(settings.contains(hkey) && settings.contains(pkey)) {
            addHost(settings.value(hkey).toString(), settings.value(pkey).toUInt());
        }
    }

    settings.endGroup();

    _updateHostList();
}

void UDPConfiguration::_updateHostList()
{
    m_hostList.clear();
    for (const std::shared_ptr<UDPClient> &target : m_targetHosts) {
        const QString host = target->address.toString() + ":" + QString::number(target->port);
        (void) m_hostList.append(host);
    }

    emit hostListChanged();
}

QString UDPConfiguration::_getIpAddress(const QString &address)
{
    const QHostAddress host(address);
    if (!host.isNull()) {
        return address;
    }

    const QHostInfo info = QHostInfo::fromName(address);
    if (info.error() == QHostInfo::NoError) {
        const QList<QHostAddress> hostAddresses = info.addresses();
        for (const QHostAddress &hostAddress : hostAddresses) {
            if (hostAddress.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
                return hostAddress.toString();
            }
        }
    }

    return QString();
}
