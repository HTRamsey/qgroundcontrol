#include "QGCLogRotator.h"

#include <QtCore/QFile>
#include <QtCore/QDirIterator>
#include <QtCore/QProcess>
#include <algorithm>

Q_LOGGING_CATEGORY(QGCLogRotatorLog, "Utilities.QGCLogRotator")

QGCLogRotator::QGCLogRotator(QObject *parent)
    : QObject(parent)
{
    connect(&_timer, &QTimer::timeout, this, &QGCLogRotator::_check);
}

QGCLogRotator::~QGCLogRotator()
{
    stop();
}

void QGCLogRotator::setLogDirectory(const QString &directory)
{
    if (_logDirectory == directory) {
        return;
    }

    _logDirectory = directory;
    emit logDirectoryChanged(directory);

    QDir dir(directory);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
}

void QGCLogRotator::setLogFilePattern(const QString &pattern)
{
    _pattern = pattern;
}

void QGCLogRotator::setMaxFileSize(qint64 bytes)
{
    if (_maxFileSize == bytes) {
        return;
    }

    _maxFileSize = bytes;
    emit maxFileSizeChanged(bytes);
}

void QGCLogRotator::setMaxFiles(int count)
{
    if (_maxFiles == count) {
        return;
    }

    _maxFiles = count;
    emit maxFilesChanged(count);
}

void QGCLogRotator::setMaxAge(int days)
{
    if (_maxAgeDays == days) {
        return;
    }

    _maxAgeDays = days;
    emit maxAgeChanged(days);
}

void QGCLogRotator::setCompressOldLogs(bool compress)
{
    if (_compressOldLogs == compress) {
        return;
    }

    _compressOldLogs = compress;
    emit compressOldLogsChanged(compress);
}

void QGCLogRotator::setCheckIntervalSeconds(int seconds)
{
    _checkIntervalSecs = seconds;
    if (_running) {
        _timer.setInterval(seconds * 1000);
    }
}

void QGCLogRotator::start()
{
    if (_running) {
        return;
    }

    if (_logDirectory.isEmpty()) {
        emit error(QStringLiteral("Log directory not set"));
        return;
    }

    _running = true;
    _timer.start(_checkIntervalSecs * 1000);
    emit runningChanged(true);

    qCInfo(QGCLogRotatorLog) << "Started log rotation for:" << _logDirectory;

    // Initial check
    _check();
}

void QGCLogRotator::stop()
{
    if (!_running) {
        return;
    }

    _timer.stop();
    _running = false;
    emit runningChanged(false);

    qCInfo(QGCLogRotatorLog) << "Stopped log rotation";
}

bool QGCLogRotator::rotateNow(const QString &logFile)
{
    QString fileToRotate = logFile;
    if (fileToRotate.isEmpty()) {
        fileToRotate = _currentLogFile;
    }

    if (fileToRotate.isEmpty()) {
        return false;
    }

    return _rotateFile(fileToRotate);
}

bool QGCLogRotator::cleanupNow()
{
    _deleteExpiredFiles();
    _enforceMaxFiles();
    return true;
}

bool QGCLogRotator::compressFile(const QString &filePath)
{
    if (filePath.endsWith(QStringLiteral(".gz"))) {
        return true;  // Already compressed
    }

    const QString compressedPath = filePath + QStringLiteral(".gz");

#ifdef Q_OS_WIN
    // On Windows, use a simple copy (no native gzip)
    // In a real implementation, you'd use zlib or similar
    return QFile::copy(filePath, compressedPath) && QFile::remove(filePath);
#else
    QProcess gzip;
    gzip.start(QStringLiteral("gzip"), {QStringLiteral("-f"), filePath});

    if (!gzip.waitForFinished(30000)) {
        emit error(QStringLiteral("Compression timeout: %1").arg(filePath));
        return false;
    }

    if (gzip.exitCode() != 0) {
        emit error(QStringLiteral("Compression failed: %1").arg(QString::fromUtf8(gzip.readAllStandardError())));
        return false;
    }

    emit logCompressed(compressedPath);
    qCDebug(QGCLogRotatorLog) << "Compressed:" << filePath;
    return true;
#endif
}

QList<QGCLogRotator::LogFileInfo> QGCLogRotator::logFiles() const
{
    return _scanLogFiles();
}

qint64 QGCLogRotator::totalLogSize() const
{
    qint64 total = 0;
    for (const LogFileInfo &info : _scanLogFiles()) {
        total += info.size;
    }
    return total;
}

int QGCLogRotator::logFileCount() const
{
    return static_cast<int>(_scanLogFiles().size());
}

void QGCLogRotator::setCurrentLogFile(const QString &path)
{
    _currentLogFile = path;
}

bool QGCLogRotator::needsRotation(const QString &logFile) const
{
    const QFileInfo info(logFile);
    if (!info.exists()) {
        return false;
    }

    return info.size() >= _maxFileSize;
}

bool QGCLogRotator::isFileExpired(const QString &logFile) const
{
    const QFileInfo info(logFile);
    if (!info.exists()) {
        return false;
    }

    const QDateTime expireDate = QDateTime::currentDateTime().addDays(-_maxAgeDays);
    return info.lastModified() < expireDate;
}

void QGCLogRotator::_check()
{
    // Check current log file for rotation
    if (!_currentLogFile.isEmpty() && needsRotation(_currentLogFile)) {
        _rotateFile(_currentLogFile);
    }

    // Check other log files
    const QList<LogFileInfo> files = _scanLogFiles();
    for (const LogFileInfo &info : files) {
        if (!info.compressed && info.rotationIndex > 0 && needsRotation(info.fullPath)) {
            _rotateFile(info.fullPath);
        }
    }

    // Compress old files
    if (_compressOldLogs) {
        _compressOldFiles();
    }

    // Cleanup
    _deleteExpiredFiles();
    _enforceMaxFiles();
}

bool QGCLogRotator::_rotateFile(const QString &filePath)
{
    const QFileInfo info(filePath);
    if (!info.exists()) {
        return false;
    }

    // Shift existing rotated files
    for (int i = _maxFiles - 1; i >= 1; --i) {
        const QString oldName = _generateRotatedName(filePath, i);
        const QString newName = _generateRotatedName(filePath, i + 1);

        if (QFile::exists(oldName)) {
            if (QFile::exists(newName)) {
                QFile::remove(newName);
            }
            QFile::rename(oldName, newName);
        }

        // Also handle compressed versions
        const QString oldNameGz = oldName + QStringLiteral(".gz");
        const QString newNameGz = newName + QStringLiteral(".gz");

        if (QFile::exists(oldNameGz)) {
            if (QFile::exists(newNameGz)) {
                QFile::remove(newNameGz);
            }
            QFile::rename(oldNameGz, newNameGz);
        }
    }

    // Rotate current file
    const QString rotatedName = _generateRotatedName(filePath, 1);
    if (QFile::exists(rotatedName)) {
        QFile::remove(rotatedName);
    }

    if (!QFile::rename(filePath, rotatedName)) {
        emit error(QStringLiteral("Failed to rotate: %1").arg(filePath));
        return false;
    }

    emit logRotated(filePath, rotatedName);
    qCInfo(QGCLogRotatorLog) << "Rotated:" << filePath << "->" << rotatedName;

    return true;
}

void QGCLogRotator::_compressOldFiles()
{
    const QList<LogFileInfo> files = _scanLogFiles();

    for (const LogFileInfo &info : files) {
        if (!info.compressed && info.rotationIndex >= 1) {
            compressFile(info.fullPath);
        }
    }
}

void QGCLogRotator::_deleteExpiredFiles()
{
    const QDateTime expireDate = QDateTime::currentDateTime().addDays(-_maxAgeDays);
    const QList<LogFileInfo> files = _scanLogFiles();

    for (const LogFileInfo &info : files) {
        if (info.modified < expireDate) {
            if (QFile::remove(info.fullPath)) {
                emit logDeleted(info.fullPath);
                qCInfo(QGCLogRotatorLog) << "Deleted expired log:" << info.fullPath;
            }
        }
    }
}

void QGCLogRotator::_enforceMaxFiles()
{
    QList<LogFileInfo> files = _scanLogFiles();

    // Sort by modification time (newest first)
    std::sort(files.begin(), files.end(), [](const LogFileInfo &a, const LogFileInfo &b) {
        return a.modified > b.modified;
    });

    // Delete excess files
    while (files.size() > _maxFiles) {
        const LogFileInfo &oldest = files.last();
        if (QFile::remove(oldest.fullPath)) {
            emit logDeleted(oldest.fullPath);
            qCDebug(QGCLogRotatorLog) << "Deleted excess log:" << oldest.fullPath;
        }
        files.removeLast();
    }
}

QString QGCLogRotator::_generateRotatedName(const QString &basePath, int index) const
{
    const QFileInfo info(basePath);
    const QString base = info.completeBaseName();
    const QString suffix = info.suffix();
    const QString dir = info.absolutePath();

    if (suffix.isEmpty()) {
        return QStringLiteral("%1/%2.%3").arg(dir, base, QString::number(index));
    }
    return QStringLiteral("%1/%2.%3.%4").arg(dir, base, QString::number(index), suffix);
}

QList<QGCLogRotator::LogFileInfo> QGCLogRotator::_scanLogFiles() const
{
    QList<LogFileInfo> result;

    if (_logDirectory.isEmpty()) {
        return result;
    }

    QDir dir(_logDirectory);
    if (!dir.exists()) {
        return result;
    }

    QStringList filters;
    filters << _pattern;
    filters << _pattern + QStringLiteral(".gz");

    QString rotatedPattern = _pattern;
    rotatedPattern.replace(QStringLiteral("*"), QStringLiteral("*.?"));
    filters << rotatedPattern;
    filters << rotatedPattern + QStringLiteral(".gz");

    const QFileInfoList entries = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    for (const QFileInfo &entry : entries) {
        LogFileInfo info;
        info.filename = entry.fileName();
        info.fullPath = entry.absoluteFilePath();
        info.size = entry.size();
        info.created = entry.birthTime();
        info.modified = entry.lastModified();
        info.compressed = entry.suffix() == QStringLiteral("gz");

        // Determine rotation index from filename
        const QString baseName = info.compressed ? QFileInfo(entry.completeBaseName()).completeBaseName()
                                                  : entry.completeBaseName();
        const QStringList parts = baseName.split('.');
        if (parts.size() > 1) {
            bool ok = false;
            const int idx = parts.last().toInt(&ok);
            info.rotationIndex = ok ? idx : 0;
        } else {
            info.rotationIndex = 0;
        }

        result.append(info);
    }

    return result;
}
