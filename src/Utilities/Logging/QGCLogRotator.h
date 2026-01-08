#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCLogRotatorLog)

/// Log file rotation and cleanup manager.
///
/// Handles automatic rotation of log files based on size or time,
/// compression of old logs, and cleanup of expired logs.
///
/// Example usage:
/// @code
/// QGCLogRotator rotator;
/// rotator.setLogDirectory("/path/to/logs");
/// rotator.setMaxFileSize(10 * 1024 * 1024);  // 10 MB
/// rotator.setMaxFiles(5);
/// rotator.setMaxAge(7);  // 7 days
/// rotator.start();
/// @endcode
class QGCLogRotator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString logDirectory READ logDirectory WRITE setLogDirectory NOTIFY logDirectoryChanged)
    Q_PROPERTY(qint64 maxFileSize READ maxFileSize WRITE setMaxFileSize NOTIFY maxFileSizeChanged)
    Q_PROPERTY(int maxFiles READ maxFiles WRITE setMaxFiles NOTIFY maxFilesChanged)
    Q_PROPERTY(int maxAge READ maxAge WRITE setMaxAge NOTIFY maxAgeChanged)
    Q_PROPERTY(bool compressOldLogs READ compressOldLogs WRITE setCompressOldLogs NOTIFY compressOldLogsChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    struct LogFileInfo {
        QString filename;
        QString fullPath;
        qint64 size;
        QDateTime created;
        QDateTime modified;
        bool compressed;
        int rotationIndex;  // 0 = current, 1 = first rotation, etc.
    };

    explicit QGCLogRotator(QObject *parent = nullptr);
    ~QGCLogRotator() override;

    // Configuration
    QString logDirectory() const { return _logDirectory; }
    void setLogDirectory(const QString &directory);

    QString logFilePattern() const { return _pattern; }
    void setLogFilePattern(const QString &pattern);  // e.g., "app.log" or "app-*.log"

    qint64 maxFileSize() const { return _maxFileSize; }
    void setMaxFileSize(qint64 bytes);

    int maxFiles() const { return _maxFiles; }
    void setMaxFiles(int count);

    int maxAge() const { return _maxAgeDays; }
    void setMaxAge(int days);

    bool compressOldLogs() const { return _compressOldLogs; }
    void setCompressOldLogs(bool compress);

    int checkIntervalSeconds() const { return _checkIntervalSecs; }
    void setCheckIntervalSeconds(int seconds);

    // Control
    void start();
    void stop();
    bool isRunning() const { return _running; }

    // Manual operations
    Q_INVOKABLE bool rotateNow(const QString &logFile = {});
    Q_INVOKABLE bool cleanupNow();
    Q_INVOKABLE bool compressFile(const QString &filePath);

    // Status
    QList<LogFileInfo> logFiles() const;
    qint64 totalLogSize() const;
    int logFileCount() const;
    QString currentLogFile() const { return _currentLogFile; }
    void setCurrentLogFile(const QString &path);

    // Monitoring
    bool needsRotation(const QString &logFile) const;
    bool isFileExpired(const QString &logFile) const;

signals:
    void logDirectoryChanged(const QString &directory);
    void maxFileSizeChanged(qint64 bytes);
    void maxFilesChanged(int count);
    void maxAgeChanged(int days);
    void compressOldLogsChanged(bool compress);
    void runningChanged(bool running);
    void logRotated(const QString &oldPath, const QString &newPath);
    void logCompressed(const QString &path);
    void logDeleted(const QString &path);
    void error(const QString &message);

private slots:
    void _check();

private:
    bool _rotateFile(const QString &filePath);
    void _compressOldFiles();
    void _deleteExpiredFiles();
    void _enforceMaxFiles();
    QString _generateRotatedName(const QString &basePath, int index) const;
    QList<LogFileInfo> _scanLogFiles() const;

    QTimer _timer;
    QString _logDirectory;
    QString _pattern = QStringLiteral("*.log");
    QString _currentLogFile;
    qint64 _maxFileSize = 10 * 1024 * 1024;  // 10 MB default
    int _maxFiles = 5;
    int _maxAgeDays = 30;
    int _checkIntervalSecs = 60;
    bool _compressOldLogs = true;
    bool _running = false;
};
