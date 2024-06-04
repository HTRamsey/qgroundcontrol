/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once


#include "LinkInterface.h"
#include "LinkConfiguration.h"

#include <QtCore/QString>

#define DEFAULT_TCP_PORT 5760

class QTcpSocket;

class TCPConfiguration : public LinkConfiguration
{
    Q_OBJECT

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)

public:
    explicit TCPConfiguration(const QString &name, QObject *parent = nullptr);
    explicit TCPConfiguration(TCPConfiguration* source);
    ~TCPConfiguration();

    QString host() const { return m_host; }
    void setHost(const QString &host) { m_host = host; }

    quint16 port() const { return m_port; }
    void setPort(quint16 port) { m_port = port; }

    LinkType type() final { return LinkConfiguration::TypeTcp; }
    void copyFrom(LinkConfiguration* source) final;
    void loadSettings(QSettings &settings, const QString &root) final;
    void saveSettings(QSettings &settings, const QString &root) final;
    QString settingsURL() final { return QStringLiteral("TcpSettings.qml"); }
    QString settingsTitle() final { return QStringLiteral("TCP Link Settings"); }

signals:
    void portChanged();
    void hostChanged();

private:
    QString m_host = QStringLiteral("0.0.0.0");
    quint16 m_port = DEFAULT_TCP_PORT;
};

////////////////////////////////////////////////////////////////////

class TCPLink : public LinkInterface
{
    Q_OBJECT

public:
    explicit TCPLink(SharedLinkConfigurationPtr &config, QObject* parent = nullptr);
    virtual ~TCPLink();

    QTcpSocket* getSocket() { return m_socket; }
    void signalBytesWritten();

    bool isConnected() const override { return m_socketIsConnected; }
    void disconnect() override;

private slots:
    void _writeBytes(const QByteArray data) override;
    void _readBytes();

private:
    bool _connect() override;

    const TCPConfiguration* m_tcpConfig = nullptr;
    QTcpSocket* m_socket = nullptr;
    bool m_socketIsConnected = false;
};
