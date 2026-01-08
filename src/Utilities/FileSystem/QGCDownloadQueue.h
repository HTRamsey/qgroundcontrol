#pragma once

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QUrl>
#include <QtCore/QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(QGCDownloadQueueLog)

class QGCFileDownload;

/// Represents a single download item in the queue
struct QGCDownloadItem
{
    QUrl url;
    QString localPath;
    QString description;
    qint64 expectedSize = 0;  // 0 = unknown

    bool operator==(const QGCDownloadItem &other) const {
        return url == other.url && localPath == other.localPath;
    }
};

/// Manages a queue of file downloads with concurrency control and overall progress.
/// Features:
/// - Concurrent downloads with configurable limit
/// - Overall progress tracking across all downloads
/// - Automatic retry on failure (configurable)
/// - Pause/resume support
/// - Priority queue support
class QGCDownloadQueue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(int completedCount READ completedCount NOTIFY completedCountChanged)
    Q_PROPERTY(int failedCount READ failedCount NOTIFY failedCountChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
    Q_PROPERTY(qreal overallProgress READ overallProgress NOTIFY overallProgressChanged)
    Q_PROPERTY(int maxConcurrent READ maxConcurrent WRITE setMaxConcurrent NOTIFY maxConcurrentChanged)

public:
    explicit QGCDownloadQueue(QObject *parent = nullptr);
    ~QGCDownloadQueue() override;

    /// Add a download to the queue
    /// @param url The URL to download
    /// @param localPath The local file path to save to
    /// @param description Optional description for UI display
    /// @param expectedSize Expected file size (for progress calculation), 0 if unknown
    /// @param priority If true, add to front of queue
    Q_INVOKABLE void addDownload(const QUrl &url, const QString &localPath,
                                  const QString &description = {},
                                  qint64 expectedSize = 0,
                                  bool priority = false);

    /// Add multiple downloads at once
    void addDownloads(const QList<QGCDownloadItem> &items);

    /// Start processing the queue
    Q_INVOKABLE void start();

    /// Pause processing (active downloads continue, no new ones start)
    Q_INVOKABLE void pause();

    /// Resume processing after pause
    Q_INVOKABLE void resume();

    /// Cancel all downloads and clear queue
    Q_INVOKABLE void cancelAll();

    /// Clear completed and failed items from tracking
    Q_INVOKABLE void clearCompleted();

    /// Remove a specific pending download
    Q_INVOKABLE bool removeDownload(const QUrl &url);

    // State queries
    bool isRunning() const { return _running; }
    bool isPaused() const { return _paused; }
    int pendingCount() const { return _pendingQueue.size(); }
    int activeCount() const { return _activeDownloads.size(); }
    int completedCount() const { return _completedCount; }
    int failedCount() const { return _failedCount; }
    int totalCount() const { return pendingCount() + activeCount() + _completedCount + _failedCount; }
    qreal overallProgress() const { return _overallProgress; }

    // Configuration
    int maxConcurrent() const { return _maxConcurrent; }
    void setMaxConcurrent(int max);

    int maxRetries() const { return _maxRetries; }
    void setMaxRetries(int retries) { _maxRetries = retries; }

    /// Get list of pending items (for UI display)
    QList<QGCDownloadItem> pendingItems() const { return QList<QGCDownloadItem>(_pendingQueue.cbegin(), _pendingQueue.cend()); }

    /// Get list of failed items
    QList<QGCDownloadItem> failedItems() const { return _failedItems; }

signals:
    void runningChanged(bool running);
    void pausedChanged(bool paused);
    void pendingCountChanged(int count);
    void activeCountChanged(int count);
    void completedCountChanged(int count);
    void failedCountChanged(int count);
    void totalCountChanged(int count);
    void overallProgressChanged(qreal progress);
    void maxConcurrentChanged(int max);

    /// Emitted when a single download completes successfully
    void downloadCompleted(const QUrl &url, const QString &localPath);

    /// Emitted when a single download fails
    void downloadFailed(const QUrl &url, const QString &errorMessage);

    /// Emitted when a download starts
    void downloadStarted(const QUrl &url);

    /// Emitted when all downloads complete (success or failure)
    void queueFinished(int successful, int failed);

    /// Progress update for a single download
    void downloadProgress(const QUrl &url, qreal progress);

private slots:
    void _onDownloadFinished(bool success, const QString &errorMessage);
    void _onDownloadProgress(qreal progress);

private:
    void _startNextDownloads();
    void _updateOverallProgress();
    void _checkQueueFinished();

    struct ActiveDownload {
        QGCFileDownload *downloader = nullptr;
        QGCDownloadItem item;
        int retryCount = 0;
        qint64 bytesReceived = 0;
    };

    QQueue<QGCDownloadItem> _pendingQueue;
    QList<ActiveDownload> _activeDownloads;
    QList<QGCDownloadItem> _failedItems;

    bool _running = false;
    bool _paused = false;
    int _maxConcurrent = 3;
    int _maxRetries = 2;
    int _completedCount = 0;
    int _failedCount = 0;
    qreal _overallProgress = 0.0;
    qint64 _totalExpectedBytes = 0;
    qint64 _totalReceivedBytes = 0;
};
