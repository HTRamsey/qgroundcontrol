/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "LinkConfiguration.h"
#include "LinkInterface.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>
#include <QtNetwork/QHostAddress>

#ifdef QGC_ZEROCONF_ENABLED
#include <dns_sd.h>
#endif

Q_DECLARE_LOGGING_CATEGORY(UDPLinkLog)

class QUdpSocket;

class UDPClient
{
public:
    UDPClient(const QHostAddress &address, quint16 port)
        : address(address)
        , port(port)
    {}

    explicit UDPClient(const UDPClient* other)
        : address(other->address)
        , port(other->port)
    {}

    QHostAddress address;
    quint16 port;
};

class UDPConfiguration : public LinkConfiguration
{
    Q_OBJECT

    Q_PROPERTY(quint16 localPort READ localPort WRITE setLocalPort NOTIFY localPortChanged)
    Q_PROPERTY(QStringList hostList READ hostList NOTIFY hostListChanged)

public:
    explicit UDPConfiguration(const QString& name, QObject *parent = nullptr);
    explicit UDPConfiguration(UDPConfiguration* source);
    ~UDPConfiguration();

    Q_INVOKABLE void addHost(const QString &host);
    Q_INVOKABLE void addHost(const QString &host, quint16 port);
    Q_INVOKABLE void removeHost(const QString &host);

    quint16 localPort() const { return m_localPort; }
    void setLocalPort(quint16 port) { m_localPort = port; }

    QStringList hostList() const { return m_hostList; }
    const QList<std::shared_ptr<UDPClient>> targetHosts() const { return m_targetHosts; }

    LinkType type() override { return LinkConfiguration::TypeUdp; }
    void copyFrom(LinkConfiguration* source) override;
    void loadSettings(QSettings &settings, const QString &root) override;
    void saveSettings(QSettings &settings, const QString &root) override;
    QString settingsURL() override { return QStringLiteral("UdpSettings.qml"); }
    QString settingsTitle() override { return QStringLiteral("UDP Link Settings"); }

signals:
    void localPortChanged();
    void hostListChanged();

private:
    void _updateHostList();
    void _copyFrom(LinkConfiguration *source);

    QList<std::shared_ptr<UDPClient>> m_targetHosts;
    QStringList m_hostList;
    quint16 m_localPort;

    static QString _getIpAddress(const QString &address);
};

class UDPLink : public LinkInterface
{
    Q_OBJECT

public:
    explicit UDPLink(SharedLinkConfigurationPtr &config, QObject *parent = nullptr);
    virtual ~UDPLink();

    void run() override {};

    bool isConnected() const override { return m_connectState; }
    void disconnect() override;

private slots:
    void _writeBytes(const QByteArray data) override;
    void _readBytes();

private:
    bool _connect() override;

    bool _isIpLocal(const QHostAddress &add);
    void _writeDataGram(const QByteArray &data, const std::shared_ptr<UDPClient> target);

    QUdpSocket* m_socket = nullptr;
    UDPConfiguration* m_udpConfig = nullptr;
    QList<QHostAddress> m_localAddresses;
    QList<std::shared_ptr<UDPClient>> m_sessionTargets;
    QMutex m_sessionTargetsMutex;
    bool m_connectState = false;

#ifdef QGC_ZEROCONF_ENABLED
    void _registerZeroconf(uint16_t port, const QString &regType);
    void _deregisterZeroconf();

    DNSServiceRef m_dnssServiceRef = nullptr;
#endif
};
