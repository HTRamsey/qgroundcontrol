/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCLogging.h"
#include "AppSettings.h"
#include "QGCApplication.h"
#include "QGCLoggingCategory.h"
#include "SettingsManager.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>
#include <QtCore/QGlobalStatic>
#include <QtCore/QPointer>
#include <QtCore/QSaveFile>
#include <QtCore/QStringListModel>
#include <QtCore/QTextStream>

QGC_LOGGING_CATEGORY(QGCLoggingLog, "Utilities.QGCLogging")

Q_GLOBAL_STATIC(QGCLogging, _qgcLogging)

static QtMessageHandler defaultHandler = nullptr;

static void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Format the message using Qt's pattern
    const QString message = qFormatLogMessage(type, context, msg);

    // Filter out Qt Quick internals - use QLatin1String to avoid QString construction
    if (QGCLogging::instance()) {
        const QLatin1String category(context.category);
        if (!category.startsWith(QLatin1String("qt.quick"))) {
            QGCLogging::instance()->log(message);
        }
    }

    // Call the previous handler if it exists
    if (defaultHandler) {
        defaultHandler(type, context, msg);
    }
}

QGCLogging *QGCLogging::instance()
{
    return _qgcLogging();
}

QGCLogging::QGCLogging(QObject *parent)
    : QStringListModel(parent)
{
    qCDebug(QGCLoggingLog) << "QGCLogging initialized";

    _flushTimer.setInterval(kFlushIntervalMSecs);
    _flushTimer.setSingleShot(false);
    (void) connect(&_flushTimer, &QTimer::timeout, this, &QGCLogging::_flushToDisk);
    _flushTimer.start();

    // Connect the emitLog signal to threadsafeLog slot
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
    const Qt::ConnectionType conntype = Qt::QueuedConnection;
#else
    const Qt::ConnectionType conntype = Qt::AutoConnection;
#endif
    (void) connect(this, &QGCLogging::emitLog, this, &QGCLogging::_threadsafeLog, conntype);
}

QGCLogging::~QGCLogging()
{
    qCDebug(QGCLoggingLog) << "QGCLogging shutting down - flushing pending logs";

    // Stop timer to prevent any new flushes during shutdown
    _flushTimer.stop();

    // Perform final flush of any pending logs
    _flushToDisk();

    // Close log file
    if (_logFile.isOpen()) {
        _logFile.close();
    }
}

void QGCLogging::installHandler()
{
    // Define the format for qDebug/qWarning/etc output
    qSetMessagePattern(QStringLiteral("%{time process}%{if-warning} Warning:%{endif}%{if-critical} Critical:%{endif} %{message} - %{category} - (%{function}:%{line})"));

    // Install our custom handler
    defaultHandler = qInstallMessageHandler(msgHandler);
}

void QGCLogging::log(const QString &message)
{
    // Emit the signal so threadsafeLog runs in the correct thread
    if (!_ioError) {
        emit emitLog(message);
    }
}

void QGCLogging::_threadsafeLog(const QString &message)
{
    // Notify view of new row
    const int line = rowCount();
    (void) QStringListModel::insertRows(line, 1);
    (void) setData(index(line, 0), message, Qt::DisplayRole);

    // Trim old entries to cap memory usage
    if (rowCount() > kMaxLogRows) {
        const int removeCount = rowCount() - kMaxLogRows;
        beginRemoveRows(QModelIndex(), 0, removeCount - 1);
        (void) removeRows(0, removeCount);
        endRemoveRows();
    }

    // Queue for disk flush
    _pendingDiskWrites.append(message);
}

void QGCLogging::_rotateLogs()
{
    // Close the current log
    _logFile.close();

    // Full path without extension
    const QString basePath = _logFile.fileName();    // e.g. "/path/QGCConsole.log"
    const QFileInfo fileInfo(basePath);
    const QString dir = fileInfo.absolutePath();
    const QString name = fileInfo.baseName();        // "QGCConsole"
    const QString ext = fileInfo.completeSuffix();   // "log"

    // Rotate existing backups: QGCConsole.4.log → QGCConsole.5.log, …
    for (int i = kMaxBackupFiles - 1; i >= 1; --i) {
        const QString from = QStringLiteral("%1/%2.%3.%4").arg(dir, name).arg(i).arg(ext);
        const QString to = QStringLiteral("%1/%2.%3.%4").arg(dir, name).arg(i+1).arg(ext);
        if (QFile::exists(to) && !QFile::remove(to)) {
            qCWarning(QGCLoggingLog) << "Failed to remove old log backup:" << to;
            // Continue anyway - better partial rotation than nothing
        }
        if (QFile::exists(from) && !QFile::rename(from, to)) {
            qCWarning(QGCLoggingLog) << "Failed to rotate log from" << from << "to" << to;
        }
    }

    // Move the just‐closed log to ".1"
    const QString firstBackup = QStringLiteral("%1/%2.1.%3").arg(dir, name, ext);
    if (QFile::exists(firstBackup) && !QFile::remove(firstBackup)) {
        qCWarning(QGCLoggingLog) << "Failed to remove first backup:" << firstBackup;
    }
    if (!QFile::rename(basePath, firstBackup)) {
        qCWarning(QGCLoggingLog) << "Failed to rename current log to backup:" << basePath << "->" << firstBackup;
    }

    // Re‑open a fresh log file
    _logFile.setFileName(basePath);
    if (!_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        _ioError = true;
        qgcApp()->showAppMessage(tr("Unable to reopen log file %1: %2").arg(_logFile.fileName(), _logFile.errorString()));
    }
}

void QGCLogging::_flushToDisk()
{
    if (_pendingDiskWrites.isEmpty() || _ioError) {
        return;
    }

    // Ensure log output enabled and file open
    if (!_logFile.isOpen()) {
        if (!qgcApp()->logOutput()) {
            _pendingDiskWrites.clear();
            return;
        }

        const QString saveDirPath = SettingsManager::instance()->appSettings()->crashSavePath();
        const QDir saveDir(saveDirPath);
        const QString saveFilePath = saveDir.absoluteFilePath("QGCConsole.log");

        _logFile.setFileName(saveFilePath);
        if (!_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            _ioError = true;
            qgcApp()->showAppMessage(tr("Open console log output file failed %1 : %2").arg(_logFile.fileName(), _logFile.errorString()));
            return;
        }
    }

    // Write all pending lines - use direct write for better performance
    int writtenCount = 0;
    for (const QString &line : std::as_const(_pendingDiskWrites)) {
        const QByteArray data = line.toUtf8() + '\n';
        const qint64 written = _logFile.write(data);
        if (written != data.size()) {
            _ioError = true;
            qCWarning(QGCLoggingLog) << "Error writing to log file:" << _logFile.errorString();
            break;
        }
        writtenCount++;
    }

    (void) _logFile.flush();

    // Only remove successfully written messages to prevent data loss
    if (writtenCount > 0) {
        _pendingDiskWrites.erase(_pendingDiskWrites.begin(),
                                 _pendingDiskWrites.begin() + writtenCount);
    }

    // Check size after writing to avoid TOCTOU race - rotate on next flush if needed
    if (_logFile.size() >= kMaxLogFileSize) {
        _rotateLogs();
    }
}

void QGCLogging::writeMessages(const QString &destFile)
{
    // Snapshot current logs on GUI thread
    const QStringList logs = stringList();

    // Use QPointer for safe 'this' access in case object is destroyed before completion
    QPointer<QGCLogging> self(this);

    // Set up future watcher to handle completion
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [self, watcher]() {
        if (self) {
            emit self->writeFinished(watcher->result());
        }
        watcher->deleteLater();
    });

    // Run the file write in a separate thread
    auto future = QtConcurrent::run([logs, destFile]() -> bool {
        QSaveFile file(destFile);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qCWarning(QGCLoggingLog) << "write failed:" << file.errorString();
            return false;
        }
        QTextStream out(&file);
        for (const QString &line : logs) {
            out << line << '\n';
        }
        return (out.status() == QTextStream::Ok) && file.commit();
    });

    watcher->setFuture(future);
    emit writeStarted();
}
