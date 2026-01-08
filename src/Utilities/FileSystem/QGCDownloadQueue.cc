#include "QGCDownloadQueue.h"
#include "QGCFileDownload.h"

#include <QtCore/QFileInfo>

Q_LOGGING_CATEGORY(QGCDownloadQueueLog, "Utilities.QGCDownloadQueue")

QGCDownloadQueue::QGCDownloadQueue(QObject *parent)
    : QObject(parent)
{
}

QGCDownloadQueue::~QGCDownloadQueue()
{
    cancelAll();
}

void QGCDownloadQueue::addDownload(const QUrl &url, const QString &localPath,
                                    const QString &description, qint64 expectedSize,
                                    bool priority)
{
    QGCDownloadItem item;
    item.url = url;
    item.localPath = localPath;
    item.description = description.isEmpty() ? url.fileName() : description;
    item.expectedSize = expectedSize;

    if (priority) {
        _pendingQueue.prepend(item);
    } else {
        _pendingQueue.enqueue(item);
    }

    _totalExpectedBytes += expectedSize;

    emit pendingCountChanged(pendingCount());
    emit totalCountChanged(totalCount());

    qCDebug(QGCDownloadQueueLog) << "Added download:" << url.toString()
                                  << "priority:" << priority
                                  << "pending:" << pendingCount();

    // Auto-start if already running
    if (_running && !_paused) {
        _startNextDownloads();
    }
}

void QGCDownloadQueue::addDownloads(const QList<QGCDownloadItem> &items)
{
    for (const auto &item : items) {
        _pendingQueue.enqueue(item);
        _totalExpectedBytes += item.expectedSize;
    }

    emit pendingCountChanged(pendingCount());
    emit totalCountChanged(totalCount());

    if (_running && !_paused) {
        _startNextDownloads();
    }
}

void QGCDownloadQueue::start()
{
    if (_running) {
        return;
    }

    _running = true;
    _paused = false;
    emit runningChanged(true);
    emit pausedChanged(false);

    qCDebug(QGCDownloadQueueLog) << "Queue started - pending:" << pendingCount();

    _startNextDownloads();
}

void QGCDownloadQueue::pause()
{
    if (!_running || _paused) {
        return;
    }

    _paused = true;
    emit pausedChanged(true);

    qCDebug(QGCDownloadQueueLog) << "Queue paused - active:" << activeCount()
                                  << "pending:" << pendingCount();
}

void QGCDownloadQueue::resume()
{
    if (!_running || !_paused) {
        return;
    }

    _paused = false;
    emit pausedChanged(false);

    qCDebug(QGCDownloadQueueLog) << "Queue resumed";

    _startNextDownloads();
}

void QGCDownloadQueue::cancelAll()
{
    qCDebug(QGCDownloadQueueLog) << "Cancelling all downloads";

    // Cancel active downloads
    for (auto &active : _activeDownloads) {
        if (active.downloader != nullptr) {
            active.downloader->cancel();
            active.downloader->deleteLater();
        }
    }
    _activeDownloads.clear();

    // Clear pending queue
    _pendingQueue.clear();

    _running = false;
    _paused = false;

    emit runningChanged(false);
    emit pausedChanged(false);
    emit pendingCountChanged(0);
    emit activeCountChanged(0);
}

void QGCDownloadQueue::clearCompleted()
{
    _completedCount = 0;
    _failedCount = 0;
    _failedItems.clear();
    _totalExpectedBytes = 0;
    _totalReceivedBytes = 0;
    _overallProgress = 0.0;

    emit completedCountChanged(0);
    emit failedCountChanged(0);
    emit totalCountChanged(totalCount());
    emit overallProgressChanged(0.0);
}

bool QGCDownloadQueue::removeDownload(const QUrl &url)
{
    // Check pending queue
    for (int i = 0; i < _pendingQueue.size(); ++i) {
        if (_pendingQueue.at(i).url == url) {
            _totalExpectedBytes -= _pendingQueue.at(i).expectedSize;
            _pendingQueue.removeAt(i);
            emit pendingCountChanged(pendingCount());
            emit totalCountChanged(totalCount());
            _updateOverallProgress();
            return true;
        }
    }

    return false;
}

void QGCDownloadQueue::setMaxConcurrent(int max)
{
    if (max < 1) {
        max = 1;
    }
    if (_maxConcurrent == max) {
        return;
    }

    _maxConcurrent = max;
    emit maxConcurrentChanged(max);

    // Start more downloads if we increased the limit
    if (_running && !_paused) {
        _startNextDownloads();
    }
}

void QGCDownloadQueue::_startNextDownloads()
{
    while (!_paused && !_pendingQueue.isEmpty() &&
           _activeDownloads.size() < _maxConcurrent) {

        QGCDownloadItem item = _pendingQueue.dequeue();
        emit pendingCountChanged(pendingCount());

        auto *downloader = new QGCFileDownload(this);

        connect(downloader, &QGCFileDownload::finished,
                this, &QGCDownloadQueue::_onDownloadFinished);
        connect(downloader, &QGCFileDownload::progressChanged,
                this, &QGCDownloadQueue::_onDownloadProgress);

        ActiveDownload active;
        active.downloader = downloader;
        active.item = item;
        active.retryCount = 0;
        _activeDownloads.append(active);

        emit activeCountChanged(activeCount());
        emit downloadStarted(item.url);

        qCDebug(QGCDownloadQueueLog) << "Starting download:" << item.url.toString()
                                      << "to:" << item.localPath;

        downloader->setOutputPath(item.localPath);
        downloader->start(item.url.toString());
    }

    _checkQueueFinished();
}

void QGCDownloadQueue::_onDownloadFinished(bool success, const QString &errorMessage)
{
    auto *downloader = qobject_cast<QGCFileDownload *>(sender());
    if (downloader == nullptr) {
        return;
    }

    // Find the active download
    int index = -1;
    for (int i = 0; i < _activeDownloads.size(); ++i) {
        if (_activeDownloads.at(i).downloader == downloader) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        downloader->deleteLater();
        return;
    }

    ActiveDownload active = _activeDownloads.takeAt(index);
    emit activeCountChanged(activeCount());

    if (success) {
        _completedCount++;
        _totalReceivedBytes += active.bytesReceived;
        emit completedCountChanged(_completedCount);
        emit downloadCompleted(active.item.url, active.item.localPath);

        qCDebug(QGCDownloadQueueLog) << "Download completed:" << active.item.url.toString();
    } else {
        // Check if we should retry
        if (active.retryCount < _maxRetries) {
            qCDebug(QGCDownloadQueueLog) << "Download failed, retrying:"
                                          << active.item.url.toString()
                                          << "attempt:" << (active.retryCount + 1);

            active.retryCount++;
            active.downloader = new QGCFileDownload(this);

            connect(active.downloader, &QGCFileDownload::finished,
                    this, &QGCDownloadQueue::_onDownloadFinished);
            connect(active.downloader, &QGCFileDownload::progressChanged,
                    this, &QGCDownloadQueue::_onDownloadProgress);

            _activeDownloads.append(active);
            emit activeCountChanged(activeCount());

            active.downloader->setOutputPath(active.item.localPath);
            active.downloader->start(active.item.url.toString());

            downloader->deleteLater();
            return;
        }

        // Max retries exceeded
        _failedCount++;
        _failedItems.append(active.item);
        emit failedCountChanged(_failedCount);
        emit downloadFailed(active.item.url, errorMessage);

        qCWarning(QGCDownloadQueueLog) << "Download failed after" << _maxRetries
                                        << "retries:" << active.item.url.toString()
                                        << "error:" << errorMessage;
    }

    downloader->deleteLater();

    _updateOverallProgress();
    _startNextDownloads();
}

void QGCDownloadQueue::_onDownloadProgress(qreal progress)
{
    auto *downloader = qobject_cast<QGCFileDownload *>(sender());
    if (downloader == nullptr) {
        return;
    }

    // Find the active download and update bytes received
    for (auto &active : _activeDownloads) {
        if (active.downloader == downloader) {
            if (active.item.expectedSize > 0) {
                active.bytesReceived = static_cast<qint64>(progress * active.item.expectedSize);
            }
            emit downloadProgress(active.item.url, progress);
            break;
        }
    }

    _updateOverallProgress();
}

void QGCDownloadQueue::_updateOverallProgress()
{
    if (_totalExpectedBytes > 0) {
        qint64 currentReceived = _totalReceivedBytes;
        for (const auto &active : _activeDownloads) {
            currentReceived += active.bytesReceived;
        }
        _overallProgress = static_cast<qreal>(currentReceived) / static_cast<qreal>(_totalExpectedBytes);
    } else {
        // Fall back to count-based progress
        const int total = _completedCount + _failedCount + activeCount() + pendingCount();
        if (total > 0) {
            _overallProgress = static_cast<qreal>(_completedCount + _failedCount) / static_cast<qreal>(total);
        } else {
            _overallProgress = 0.0;
        }
    }

    emit overallProgressChanged(_overallProgress);
}

void QGCDownloadQueue::_checkQueueFinished()
{
    if (_running && _pendingQueue.isEmpty() && _activeDownloads.isEmpty()) {
        _running = false;
        emit runningChanged(false);
        emit queueFinished(_completedCount, _failedCount);

        qCDebug(QGCDownloadQueueLog) << "Queue finished - completed:" << _completedCount
                                      << "failed:" << _failedCount;
    }
}
