#include "QGCRateLimiter.h"

#include <QtCore/QTimer>

Q_LOGGING_CATEGORY(QGCRateLimiterLog, "Utilities.QGCRateLimiter")

QGCRateLimiter::QGCRateLimiter(int maxRequests, int windowMs, QObject *parent)
    : QObject(parent)
    , _maxRequests(maxRequests)
    , _windowMs(windowMs)
    , _tokens(maxRequests)
    , _lastRefillTime(QDateTime::currentMSecsSinceEpoch())
{
    _processTimer = new QTimer(this);
    _processTimer->setSingleShot(true);
    connect(_processTimer, &QTimer::timeout, this, &QGCRateLimiter::_processQueue);

    _refillTimer = new QTimer(this);
    connect(_refillTimer, &QTimer::timeout, this, &QGCRateLimiter::_refillTokens);
    _refillTimer->start(_windowMs / _maxRequests);  // Refill interval
}

QGCRateLimiter::~QGCRateLimiter()
{
    clearPending();
}

void QGCRateLimiter::setMaxRequests(int max)
{
    if (max < 1) max = 1;
    if (_maxRequests == max) return;

    _maxRequests = max;
    _tokens = qMin(_tokens, _maxRequests + _burstCapacity);
    emit maxRequestsChanged(max);
    emit availableTokensChanged(availableTokens());

    // Update refill timer
    _refillTimer->setInterval(_windowMs / _maxRequests);
}

void QGCRateLimiter::setWindowMs(int ms)
{
    if (ms < 1) ms = 1;
    if (_windowMs == ms) return;

    _windowMs = ms;
    emit windowMsChanged(ms);

    // Update refill timer
    _refillTimer->setInterval(_windowMs / _maxRequests);
}

int QGCRateLimiter::availableTokens() const
{
    // Use sliding window for accurate count
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 windowStart = now - _windowMs;

    int recentRequests = 0;
    for (qint64 timestamp : _requestTimestamps) {
        if (timestamp >= windowStart) {
            recentRequests++;
        }
    }

    return qMax(0, _maxRequests + _burstCapacity - recentRequests);
}

int QGCRateLimiter::timeUntilAvailable() const
{
    if (availableTokens() > 0) {
        return 0;
    }

    if (_requestTimestamps.isEmpty()) {
        return 0;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 oldestInWindow = _requestTimestamps.first();
    const qint64 expiresAt = oldestInWindow + _windowMs;

    return qMax(0, static_cast<int>(expiresAt - now));
}

bool QGCRateLimiter::execute(Operation operation, bool dropIfLimited)
{
    if (!operation) {
        return false;
    }

    _updateTokens();

    if (availableTokens() > 0) {
        _recordRequest();
        operation();
        emit operationExecuted();
        return true;
    }

    if (dropIfLimited) {
        qCDebug(QGCRateLimiterLog) << "Operation dropped (rate limited)";
        emit operationDropped();
        return false;
    }

    // Queue the operation
    _pendingOperations.enqueue(operation);
    emit pendingCountChanged(pendingCount());
    emit operationQueued();

    // Schedule processing
    if (!_processTimer->isActive()) {
        _processTimer->start(timeUntilAvailable());
    }

    qCDebug(QGCRateLimiterLog) << "Operation queued, pending:" << pendingCount()
                                << "next available in:" << timeUntilAvailable() << "ms";
    return false;
}

bool QGCRateLimiter::tryExecute(Operation operation)
{
    if (!operation) {
        return false;
    }

    _updateTokens();

    if (availableTokens() > 0) {
        _recordRequest();
        operation();
        emit operationExecuted();
        return true;
    }

    return false;
}

void QGCRateLimiter::clearPending()
{
    const int count = _pendingOperations.size();
    _pendingOperations.clear();

    if (count > 0) {
        emit pendingCountChanged(0);
        qCDebug(QGCRateLimiterLog) << "Cleared" << count << "pending operations";
    }
}

void QGCRateLimiter::reset()
{
    clearPending();
    _requestTimestamps.clear();
    _tokens = _maxRequests + _burstCapacity;
    _lastRefillTime = QDateTime::currentMSecsSinceEpoch();

    emit availableTokensChanged(availableTokens());
    emit limitedChanged(false);

    qCDebug(QGCRateLimiterLog) << "Rate limiter reset";
}

void QGCRateLimiter::_processQueue()
{
    _updateTokens();

    while (!_pendingOperations.isEmpty() && availableTokens() > 0) {
        Operation op = _pendingOperations.dequeue();
        emit pendingCountChanged(pendingCount());

        _recordRequest();
        op();
        emit operationExecuted();
    }

    // Schedule next processing if there are still pending operations
    if (!_pendingOperations.isEmpty()) {
        _processTimer->start(timeUntilAvailable());
    }
}

void QGCRateLimiter::_refillTokens()
{
    _updateTokens();
}

void QGCRateLimiter::_recordRequest()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    _requestTimestamps.append(now);

    const bool wasLimited = isLimited();
    emit availableTokensChanged(availableTokens());

    if (wasLimited != isLimited()) {
        emit limitedChanged(isLimited());
    }
}

void QGCRateLimiter::_updateTokens()
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 windowStart = now - _windowMs;

    // Remove expired timestamps
    while (!_requestTimestamps.isEmpty() && _requestTimestamps.first() < windowStart) {
        _requestTimestamps.removeFirst();
    }

    const int oldAvailable = availableTokens();
    _lastRefillTime = now;

    if (oldAvailable != availableTokens()) {
        emit availableTokensChanged(availableTokens());

        if (oldAvailable == 0 && availableTokens() > 0) {
            emit limitedChanged(false);
        }
    }
}
