/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "TCPLink.h"
#include <QGCLoggingCategory.h>

#include <QtCore/QList>
#include <QtNetwork/QTcpSocket>

QGC_LOGGING_CATEGORY(TCPLinkLog, "ENABLED.comms.tcplink")

TCPLink::TCPLink(SharedLinkConfigurationPtr &config, QObject* parent)
    : LinkInterface(config, parent)
    , m_tcpConfig(qobject_cast<const TCPConfiguration*>(config.get()))
    , m_socket(new QTcpSocket(this))
{
    qCDebug(TCPLinkLog) << Q_FUNC_INFO << this;

    (void) connect(m_socket, &QTcpSocket::stateChanged, this, [this](QTcpSocket::SocketState state) {
        switch (state) {
            case QTcpSocket::SocketState::UnconnectedState:
                m_socketIsConnected = false;
                emit disconnected();
                qCDebug(TCPLinkLog) << "TcpLink Socket disconnected";
                break;

            case QTcpSocket::SocketState::ConnectingState:
                qCDebug(TCPLinkLog) << "TcpLink Socket connecting...";
                break;

            case QTcpSocket::SocketState::ConnectedState:
                m_socketIsConnected = true;
                emit connected();
                qCDebug(TCPLinkLog) << "TcpLink Socket connected";
                break;

            case QTcpSocket::SocketState::ClosingState:
                qCDebug(TCPLinkLog) << "TcpLink Socket closing...";

            default:
                break;
        }
    }, Qt::QueuedConnection);

    (void) connect(m_socket, &QTcpSocket::connected, this, &TCPLink::connected);
    (void) connect(m_socket, &QTcpSocket::disconnected, this, &TCPLink::disconnected);

    (void) connect(m_socket, &QTcpSocket::errorOccurred, this, [this](QTcpSocket::SocketError error) {
        qCDebug(TCPLinkLog) << error << m_socket->errorString();
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Error on link %1. Error on socket: %2.").arg(m_tcpConfig->name(), m_socket->errorString()));
    }, Qt::AutoConnection);

    (void) connect(m_socket, &QTcpSocket::readyRead, this, &TCPLink::_readBytes);
}

TCPLink::~TCPLink()
{
    qCDebug(TCPLinkLog) << Q_FUNC_INFO << this;
}

void TCPLink::disconnect()
{
    m_socket->disconnectFromHost();
    emit disconnected();
}

bool TCPLink::_connect()
{
    m_socket->connectToHost(m_tcpConfig->host(), m_tcpConfig->port());
    emit connected();
    return true;
}

void TCPLink::_writeBytes(const QByteArray data)
{
    (void) m_socket->write(data);
    emit bytesSent(this, data);
}

void TCPLink::_readBytes()
{
    const qint64 byteCount = m_socket->bytesAvailable();
    if (byteCount > 0) {
        QByteArray buffer;
        buffer.resize(byteCount);
        (void) m_socket->read(buffer.data(), buffer.size());
        emit bytesReceived(this, buffer);
    }
}

////////////////////////////////////////////////////////////////////

TCPConfiguration::TCPConfiguration(const QString &name, QObject* parent)
    : LinkConfiguration(name, parent)
{
    qCDebug(TCPLinkLog) << Q_FUNC_INFO << this;
}

TCPConfiguration::TCPConfiguration(TCPConfiguration* source)
    : LinkConfiguration(source)
{
    qCDebug(TCPLinkLog) << Q_FUNC_INFO << this;

    m_host = source->host();
    m_port = source->port();
}

TCPConfiguration::~TCPConfiguration()
{
    qCDebug(TCPLinkLog) << Q_FUNC_INFO << this;
}

void TCPConfiguration::copyFrom(LinkConfiguration* source)
{
    LinkConfiguration::copyFrom(source);
    const TCPConfiguration* const tcpSource = qobject_cast<const TCPConfiguration*>(source);
    m_port = tcpSource->port();
    m_host = tcpSource->host();
}

void TCPConfiguration::saveSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);
    settings.setValue("host", m_host);
    settings.setValue("port", m_port);
    settings.endGroup();
}

void TCPConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);
    m_host = settings.value("host", m_host).toString();
    m_port = static_cast<quint16>(settings.value("port", DEFAULT_TCP_PORT).toUInt());
    settings.endGroup();
}
