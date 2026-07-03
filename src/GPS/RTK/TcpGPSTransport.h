#pragma once

#include "GPSTransport.h"

#include <QtCore/QString>

#include <atomic>
#include <cstdint>
#include <memory>

class QTcpSocket;

/// GPSTransport over a TCP connection to a network-attached RTK receiver / base
/// station. Owns the socket and must be constructed on the thread that pumps the
/// driver — QTcpSocket has thread affinity.
class TcpGPSTransport : public GPSTransport
{
public:
    TcpGPSTransport(QString host, quint16 port, std::shared_ptr<std::atomic_bool> requestStop);
    ~TcpGPSTransport() override;

    /// Connect to the endpoint, retrying briefly if the base station isn't up yet.
    /// Aborts the retry promptly if requestStop is set.
    bool open() override;

    /// True once the connection has dropped and the receive loop should stop.
    bool fatalError() const override;

    int read(uint8_t *buffer, int length, int timeoutMs) override;
    int write(const uint8_t *buffer, int length) override;

    /// No-op: baud rate is meaningless over TCP. Always succeeds.
    bool setBaudrate(unsigned baudrate) override;

private:
    static constexpr int kConnectTimeoutMs = 5000;
    static constexpr int kWriteTimeoutMs = 500;
    static constexpr uint32_t kConnectRetries = 12;

    QString _host;
    quint16 _port;
    std::shared_ptr<std::atomic_bool> _requestStop;
    std::unique_ptr<QTcpSocket> _socket;
};
