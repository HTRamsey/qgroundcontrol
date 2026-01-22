#include "QGCRetryQueue.h"

#include <QtCore/QTimer>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QUuid>
#include <QtCore/QDateTime>

#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(QGCRetryQueueLog, "Utilities.QGCRetryQueue")

QGCRetryQueue::QGCRetryQueue(QObject *parent)
    : QObject(parent)
    , _processTimer(new QTimer(this))
{
    _processTimer->setSingleShot(true);
    connect(_processTimer, &QTimer::timeout, this, &QGCRetryQueue::_processNext);
}

QGCRetryQueue::QGCRetryQueue(const QString &persistPath, QObject *parent)
    : QGCRetryQueue(parent)
{
    setPersistPath(persistPath);
    load();
}

QGCRetryQueue::~QGCRetryQueue()
{
    save();
}

void QGCRetryQueue::setProcessor(Processor processor)
{
    _processor = std::move(processor);
    _scheduleProcessing();
}

void QGCRetryQueue::setPersistPath(const QString &path)
{
    _persistPath = path;
}

QString QGCRetryQueue::enqueue(const QVariant &data, int priority)
{
    Item item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.data = data;
    item.priority = priority;
    item.createdAt = QDateTime::currentMSecsSinceEpoch();
    item.nextRetry = item.createdAt;

    // Insert by priority (higher priority first)
    int insertPos = 0;
    for (int i = 0; i < _queue.size(); ++i) {
        if (_queue.at(i).priority < priority) {
            insertPos = i;
            break;
        }
        insertPos = i + 1;
    }

    _queue.insert(insertPos, item);
    emit countChanged(count());
    emit itemEnqueued(item.id);

    qCDebug(QGCRetryQueueLog) << "Enqueued item" << item.id << "priority:" << priority;

    _scheduleProcessing();
    save();

    return item.id;
}

bool QGCRetryQueue::remove(const QString &id)
{
    for (int i = 0; i < _queue.size(); ++i) {
        if (_queue.at(i).id == id) {
            _queue.removeAt(i);
            emit countChanged(count());
            save();
            qCDebug(QGCRetryQueueLog) << "Removed item" << id;
            return true;
        }
    }
    return false;
}

void QGCRetryQueue::clear()
{
    _queue.clear();
    emit countChanged(0);
    save();
    qCDebug(QGCRetryQueueLog) << "Cleared queue";
}

void QGCRetryQueue::retryNow()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (Item &item : _queue) {
        item.nextRetry = now;
    }
    _scheduleProcessing();
}

void QGCRetryQueue::setOnline(bool online)
{
    if (_online == online) {
        return;
    }

    _online = online;
    emit onlineChanged(online);

    if (online) {
        retryNow();
    }

    qCDebug(QGCRetryQueueLog) << "Online:" << online;
}

void QGCRetryQueue::setPaused(bool paused)
{
    if (_paused == paused) {
        return;
    }

    _paused = paused;
    emit pausedChanged(paused);

    if (!paused) {
        _scheduleProcessing();
    }

    qCDebug(QGCRetryQueueLog) << "Paused:" << paused;
}

QGCRetryQueue::Item QGCRetryQueue::item(const QString &id) const
{
    for (const Item &item : _queue) {
        if (item.id == id) {
            return item;
        }
    }
    return {};
}

void QGCRetryQueue::_scheduleProcessing()
{
    if (_paused || !_online || _processing || _queue.isEmpty() || !_processor) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 nextRetry = std::numeric_limits<qint64>::max();

    for (const Item &item : _queue) {
        if (item.nextRetry < nextRetry) {
            nextRetry = item.nextRetry;
        }
    }

    const qint64 delay = std::max(0LL, nextRetry - now);
    _processTimer->start(static_cast<int>(std::min(delay, static_cast<qint64>(INT_MAX))));
}

void QGCRetryQueue::_processNext()
{
    if (_paused || !_online || _queue.isEmpty() || !_processor) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Find first item ready for processing
    int readyIndex = -1;
    for (int i = 0; i < _queue.size(); ++i) {
        if (_queue.at(i).nextRetry <= now) {
            readyIndex = i;
            break;
        }
    }

    if (readyIndex < 0) {
        _scheduleProcessing();
        return;
    }

    _processing = true;
    emit processingChanged(true);

    Item &item = _queue[readyIndex];
    item.attempts++;
    item.lastAttempt = now;

    qCDebug(QGCRetryQueueLog) << "Processing item" << item.id
                               << "attempt:" << item.attempts;

    bool success = false;
    try {
        success = _processor(item.data);
    } catch (const std::exception &e) {
        item.lastError = QString::fromUtf8(e.what());
    } catch (...) {
        item.lastError = tr("Unknown error");
    }

    if (success) {
        const QString id = item.id;
        _queue.removeAt(readyIndex);
        emit countChanged(count());
        emit itemCompleted(id);
        qCDebug(QGCRetryQueueLog) << "Item completed" << id;
    } else {
        if (item.attempts >= _maxAttempts) {
            const QString id = item.id;
            const QString error = item.lastError;
            _queue.removeAt(readyIndex);
            emit countChanged(count());
            emit itemDropped(id, tr("Max attempts reached: %1").arg(error));
            qCWarning(QGCRetryQueueLog) << "Item dropped after max attempts" << id;
        } else {
            item.nextRetry = now + _calculateDelay(item.attempts);
            emit itemFailed(item.id, item.lastError);
            qCDebug(QGCRetryQueueLog) << "Item failed, will retry" << item.id
                                       << "in" << (item.nextRetry - now) << "ms";
        }
    }

    save();

    _processing = false;
    emit processingChanged(false);

    _scheduleProcessing();
}

int QGCRetryQueue::_calculateDelay(int attempts) const
{
    const double delay = _initialDelayMs * std::pow(_backoffMultiplier, attempts - 1);
    return static_cast<int>(std::min(delay, static_cast<double>(_maxDelayMs)));
}

QGCRetryQueue::Item *QGCRetryQueue::_findItem(const QString &id)
{
    for (Item &item : _queue) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

bool QGCRetryQueue::save()
{
    if (_persistPath.isEmpty()) {
        return true;
    }

    QJsonArray array;
    for (const Item &item : _queue) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = item.id;
        obj[QStringLiteral("data")] = QJsonValue::fromVariant(item.data);
        obj[QStringLiteral("attempts")] = item.attempts;
        obj[QStringLiteral("priority")] = item.priority;
        obj[QStringLiteral("createdAt")] = item.createdAt;
        obj[QStringLiteral("lastAttempt")] = item.lastAttempt;
        obj[QStringLiteral("nextRetry")] = item.nextRetry;
        obj[QStringLiteral("lastError")] = item.lastError;
        array.append(obj);
    }

    QFile file(_persistPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(QGCRetryQueueLog) << "Failed to save queue:" << file.errorString();
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
    return true;
}

bool QGCRetryQueue::load()
{
    if (_persistPath.isEmpty()) {
        return true;
    }

    QFile file(_persistPath);
    if (!file.exists()) {
        return true;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(QGCRetryQueueLog) << "Failed to load queue:" << file.errorString();
        return false;
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(QGCRetryQueueLog) << "Failed to parse queue:" << error.errorString();
        return false;
    }

    _queue.clear();
    const QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        const QJsonObject obj = val.toObject();
        Item item;
        item.id = obj[QStringLiteral("id")].toString();
        item.data = obj[QStringLiteral("data")].toVariant();
        item.attempts = obj[QStringLiteral("attempts")].toInt();
        item.priority = obj[QStringLiteral("priority")].toInt();
        item.createdAt = obj[QStringLiteral("createdAt")].toVariant().toLongLong();
        item.lastAttempt = obj[QStringLiteral("lastAttempt")].toVariant().toLongLong();
        item.nextRetry = obj[QStringLiteral("nextRetry")].toVariant().toLongLong();
        item.lastError = obj[QStringLiteral("lastError")].toString();
        _queue.append(item);
    }

    emit countChanged(count());
    qCDebug(QGCRetryQueueLog) << "Loaded" << _queue.size() << "items from" << _persistPath;

    _scheduleProcessing();
    return true;
}
