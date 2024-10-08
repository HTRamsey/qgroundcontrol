/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "UdpIODevice.h"
#include "QGCLoggingCategory.h"

QGC_LOGGING_CATEGORY(UdpIODeviceLog, "qgc.comms.udpiodevice")

UdpIODevice::UdpIODevice(QObject *parent)
    : QUdpSocket(parent)
{
    // qCDebug(UdpIODeviceLog) << Q_FUNC_INFO << this;

    (void) connect(this, &QUdpSocket::readyRead, this, &UdpIODevice::_readAvailableData);
}

UdpIODevice::~UdpIODevice()
{
    // qCDebug(UdpIODeviceLog) << Q_FUNC_INFO << this;
}

qint64 UdpIODevice::readLineData(char *data, qint64 maxSize)
{
    int length = _buffer.indexOf('\n') + 1; // add 1 to include the '\n'
    if (length == 0) {
        return 0;
    }

    length = std::min(length, static_cast<int>(maxSize));
    std::copy(_buffer.data(), _buffer.data() + length, data);

    // trim buffer to remove consumed line
    _buffer = _buffer.right(_buffer.size() - length);

    return length;
}

void UdpIODevice::_readAvailableData()
{
    while (hasPendingDatagrams()) {
        const int previousSize = _buffer.size();
        const int readSize = pendingDatagramSize();
        if (readSize > 0) {
            _buffer.resize(static_cast<int>(_buffer.size() + readSize));
            (void) readDatagram((_buffer.data() + previousSize), readSize);
        }
    }
}
