/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QuaZip.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>

#include <quazip/quagzipfile.h>

QGC_LOGGING_CATEGORY(QuaZipLog, "qgc.compression.quazip")

namespace QuaZip
{

bool inflateGzipFile(const QString &gzippedFileName, const QString &decompressedFilename)
{
    QuaGzipFile inputFile(gzippedFileName);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qCWarning(QuaZipLog) << "open input file failed" << gzippedFileName << inputFile.errorString();
        return false;
    }

    QFile outputFile(decompressedFilename);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(QuaZipLog) << "open output file failed" << outputFile.fileName() << outputFile.errorString();
        return false;
    }

    constexpr qint64 cBufferSize = 1024 * 8; // 8KiB buffer
    QByteArray buffer(cBufferSize, Qt::Uninitialized);

    while (true) {
        const qint64 bytesRead = inputFile.read(buffer.data(), cBufferSize);
        if (bytesRead < 0) {
            qCWarning(QuaZipLog) << "read error:" << inputFile.errorString();
            return false;
        }
        if (bytesRead == 0) {
            // End of stream
            break;
        }

        if (outputFile.write(buffer.constData(), bytesRead) != bytesRead) {
            qCWarning(QuaZipLog) << "output file write failed:" << outputFile.fileName() << outputFile.errorString();
            return false;
        }
    }

    // Files will be closed automatically when the QFile/QIODevice objects go out of scope.
    return true;
}

} // namespace QuaZip
