/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QLoggingCategory>
#include <QtNetwork/QUdpSocket>

Q_DECLARE_LOGGING_CATEGORY(UdpIODeviceLog)

/// QUdpSocket implementation of canReadLine() readLineData() in server mode.
/// The UdpIODevice class works almost exactly as a QUdpSocket, but
/// also implements canReadLine() and readLineData() while in the bound state.
/// Regular QUdpSocket only allows to use these QIODevice interfaces when using
/// connectToHost(), which means it is working as a client instead of server.
class UdpIODevice: public QUdpSocket
{
    Q_OBJECT

public:
    UdpIODevice(QObject *parent = nullptr);
    ~UdpIODevice();

    bool canReadLine() const { return (_buffer.indexOf('\n') > -1); }
    qint64 readLineData(char *data, qint64 maxSize);

private slots:
    void _readAvailableData();

private:
    QByteArray _buffer;
};
