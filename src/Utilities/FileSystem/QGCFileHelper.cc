#include "QGCFileHelper.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>
#include <QtCore/QSaveFile>
#include <QtCore/QStorageInfo>

Q_DECLARE_LOGGING_CATEGORY(QGCFileHelperLog)
Q_LOGGING_CATEGORY(QGCFileHelperLog, "Utilities.QGCFileHelper")

namespace QGCFileHelper {

size_t optimalBufferSize(const QString &path)
{
    // Cache the result - only compute once per process
    static size_t cachedSize = 0;
    if (cachedSize != 0) {
        return cachedSize;
    }

    qint64 blockSize = 0;

    // Try to get block size from specified path
    if (!path.isEmpty()) {
        QStorageInfo storage(path);
        if (storage.isValid()) {
            blockSize = storage.blockSize();
        }
    }

    // Fallback: use root filesystem
    if (blockSize <= 0) {
        blockSize = QStorageInfo::root().blockSize();
    }

    if (blockSize <= 0) {
        cachedSize = kBufferSizeDefault;
    } else {
        // Use 16 blocks, clamped to min/max bounds
        // Typical: 4KB block × 16 = 64KB
        cachedSize = static_cast<size_t>(
            qBound(static_cast<qint64>(kBufferSizeMin),
                   blockSize * 16,
                   static_cast<qint64>(kBufferSizeMax))
        );
    }

    return cachedSize;
}

bool exists(const QString &path)
{
    return path.startsWith(QLatin1String(":/")) || QFile::exists(path);
}

bool ensureDirectoryExists(const QString &path)
{
    QDir dir(path);
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(path);
}

bool ensureParentExists(const QString &filePath)
{
    return ensureDirectoryExists(QFileInfo(filePath).absolutePath());
}

bool atomicWrite(const QString &filePath, const QByteArray &data)
{
    if (filePath.isEmpty()) {
        qCWarning(QGCFileHelperLog) << "atomicWrite: file path is empty";
        return false;
    }

    // Ensure parent directory exists
    if (!ensureParentExists(filePath)) {
        qCWarning(QGCFileHelperLog) << "atomicWrite: failed to create parent directory for:" << filePath;
        return false;
    }

    // QSaveFile writes to temp file, then atomically renames on commit()
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(QGCFileHelperLog) << "atomicWrite: failed to open:" << filePath
                                    << "-" << file.errorString();
        return false;
    }

    if (file.write(data) != data.size()) {
        qCWarning(QGCFileHelperLog) << "atomicWrite: write failed:" << file.errorString();
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        qCWarning(QGCFileHelperLog) << "atomicWrite: commit failed:" << file.errorString();
        return false;
    }

    return true;
}

qint64 availableDiskSpace(const QString &path)
{
    if (path.isEmpty()) {
        return -1;
    }

    // Get storage info for the path
    QStorageInfo storage(path);
    if (!storage.isValid()) {
        // Try parent directory if path doesn't exist yet
        storage = QStorageInfo(QFileInfo(path).absolutePath());
    }

    if (!storage.isValid()) {
        qCDebug(QGCFileHelperLog) << "availableDiskSpace: cannot determine storage for:" << path;
        return -1;
    }

    return storage.bytesAvailable();
}

bool hasSufficientDiskSpace(const QString &path, qint64 requiredBytes, double margin)
{
    if (requiredBytes <= 0) {
        return true;  // Nothing to check or unknown size
    }

    const qint64 bytesAvailable = availableDiskSpace(path);
    if (bytesAvailable < 0) {
        qCDebug(QGCFileHelperLog) << "hasSufficientDiskSpace: cannot determine disk space, proceeding anyway";
        return true;  // Proceed if we can't determine space
    }

    const qint64 bytesRequired = static_cast<qint64>(static_cast<double>(requiredBytes) * margin);

    if (bytesAvailable < bytesRequired) {
        qCWarning(QGCFileHelperLog) << "Insufficient disk space:"
                                    << "required" << bytesRequired << "bytes"
                                    << "(" << requiredBytes << "+" << static_cast<int>((margin - 1.0) * 100) << "% margin)"
                                    << "available" << bytesAvailable << "bytes";
        return false;
    }

    qCDebug(QGCFileHelperLog) << "Disk space check passed:"
                              << "required" << bytesRequired << "bytes"
                              << "available" << bytesAvailable << "bytes";
    return true;
}

} // namespace QGCFileHelper
