#include "TcpGPSTransport.h"

#include "QGCLoggingCategory.h"

#include <QtCore/QThread>
#include <QtNetwork/QTcpSocket>

#include <utility>

QGC_LOGGING_CATEGORY(TcpGPSTransportLog, "GPS.TcpGPSTransport")

TcpGPSTransport::TcpGPSTransport(QString host, quint16 port, std::shared_ptr<std::atomic_bool> requestStop)
    : _host(std::move(host))
    , _port(port)
    , _requestStop(std::move(requestStop))
{
}

TcpGPSTransport::~TcpGPSTransport() = default;

bool TcpGPSTransport::open()
{
    _socket = std::make_unique<QTcpSocket>();

    uint32_t retries = kConnectRetries;
    while (!*_requestStop) {
        _socket->connectToHost(_host, _port);
        if (_socket->waitForConnected(kConnectTimeoutMs)) {
            return true;
        }

        _socket->abort();
        if (retries-- == 0) {
            break;
        }
        qCDebug(TcpGPSTransportLog) << "Cannot connect to" << _host << _port << "... retrying";
        QThread::msleep(500);
    }

    qCWarning(TcpGPSTransportLog) << "GPS: Failed to connect to" << _host << _port << _socket->errorString();
    return false;
}

bool TcpGPSTransport::fatalError() const
{
    return _socket && (_socket->state() == QAbstractSocket::UnconnectedState);
}

int TcpGPSTransport::read(uint8_t *buffer, int length, int timeoutMs)
{
    if (*_requestStop || (_socket->state() != QAbstractSocket::ConnectedState)) {
        return -1; // abort an in-flight configure/receive so disconnect joins promptly
    }
    if (_socket->bytesAvailable() == 0) {
        if (!_socket->waitForReadyRead(timeoutMs)) {
            return (_socket->state() == QAbstractSocket::ConnectedState) ? 0 : -1;
        }
    }
    return static_cast<int>(_socket->read(reinterpret_cast<char *>(buffer), length));
}

int TcpGPSTransport::write(const uint8_t *buffer, int length)
{
    if (*_requestStop) {
        return -1;
    }
    int written = 0;
    while (written < length) {
        const qint64 n = _socket->write(reinterpret_cast<const char *>(buffer) + written, length - written);
        if (n < 0) {
            return -1;
        }
        written += static_cast<int>(n);
        if (!_socket->waitForBytesWritten(kWriteTimeoutMs)) {
            return -1;
        }
    }
    return written;
}

bool TcpGPSTransport::setBaudrate(unsigned baudrate)
{
    Q_UNUSED(baudrate)
    return true;
}
