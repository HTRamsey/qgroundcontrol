#include "QGCNetworkRetry.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QDateTime>

QGC_LOGGING_CATEGORY(QGCNetworkRetryLog, "Utilities.QGCNetworkRetry")

// ============================================================================
// Construction / Destruction
// ============================================================================

QGCNetworkRetry::QGCNetworkRetry(QObject *parent)
    : QObject(parent)
    , _retryTimer(new QTimer(this))
    , _delayUpdateTimer(new QTimer(this))
    , _rng(static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch()))
{
    _retryTimer->setSingleShot(true);
    connect(_retryTimer, &QTimer::timeout, this, &QGCNetworkRetry::_onRetryTimer);

    _delayUpdateTimer->setInterval(100);  // Update every 100ms
    connect(_delayUpdateTimer, &QTimer::timeout, this, &QGCNetworkRetry::_onDelayUpdateTimer);

    qCDebug(QGCNetworkRetryLog) << "Created with default policy";
}

QGCNetworkRetry::QGCNetworkRetry(const QGCRetryPolicy &policy, QObject *parent)
    : QObject(parent)
    , _policy(policy)
    , _retryTimer(new QTimer(this))
    , _delayUpdateTimer(new QTimer(this))
    , _rng(static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch()))
{
    _retryTimer->setSingleShot(true);
    connect(_retryTimer, &QTimer::timeout, this, &QGCNetworkRetry::_onRetryTimer);

    _delayUpdateTimer->setInterval(100);
    connect(_delayUpdateTimer, &QTimer::timeout, this, &QGCNetworkRetry::_onDelayUpdateTimer);

    qCDebug(QGCNetworkRetryLog) << "Created with custom policy - max attempts:" << policy.maxAttempts;
}

QGCNetworkRetry::~QGCNetworkRetry()
{
    cancel();
    qCDebug(QGCNetworkRetryLog) << "Destroyed";
}

// ============================================================================
// Public Methods
// ============================================================================

void QGCNetworkRetry::setPolicy(const QGCRetryPolicy &policy)
{
    if (_running) {
        qCWarning(QGCNetworkRetryLog) << "Cannot change policy while running";
        return;
    }

    _policy = policy;
    emit maxAttemptsChanged(_policy.maxAttempts);
    qCDebug(QGCNetworkRetryLog) << "Policy updated - max attempts:" << policy.maxAttempts
                                 << "initial delay:" << policy.initialDelayMs << "ms";
}

bool QGCNetworkRetry::execute(RequestFactory factory, CompletionCallback callback)
{
    if (_running) {
        qCWarning(QGCNetworkRetryLog) << "Already running";
        return false;
    }

    if (!factory) {
        qCWarning(QGCNetworkRetryLog) << "No request factory provided";
        return false;
    }

    _requestFactory = std::move(factory);
    _completionCallback = std::move(callback);
    _cancelled = false;
    _setCurrentAttempt(0);
    _setRunning(true);

    qCDebug(QGCNetworkRetryLog) << "Starting execution with max" << _policy.maxAttempts << "attempts";

    _startAttempt();
    return true;
}

void QGCNetworkRetry::cancel()
{
    if (!_running) {
        return;
    }

    qCDebug(QGCNetworkRetryLog) << "Cancelling";
    _cancelled = true;

    _retryTimer->stop();
    _delayUpdateTimer->stop();

    if (_reply != nullptr) {
        _reply->abort();
    }

    _setWaiting(false);
    _setRunning(false);
}

void QGCNetworkRetry::reset()
{
    cancel();
    _requestFactory = nullptr;
    _completionCallback = nullptr;
    _setCurrentAttempt(0);
    _setRemainingDelayMs(0);

    if (_reply != nullptr) {
        _reply->deleteLater();
        _reply = nullptr;
    }

    qCDebug(QGCNetworkRetryLog) << "Reset";
}

// ============================================================================
// Private Slots
// ============================================================================

void QGCNetworkRetry::_onReplyFinished()
{
    if (_reply == nullptr || _cancelled) {
        return;
    }

    const QNetworkReply::NetworkError error = _reply->error();
    const int httpStatus = _reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const bool success = (error == QNetworkReply::NoError) && (httpStatus >= 200 && httpStatus < 300);

    qCDebug(QGCNetworkRetryLog) << "Attempt" << _currentAttempt << "finished - success:" << success
                                 << "error:" << error << "HTTP:" << httpStatus;

    if (success) {
        _complete(true);
        return;
    }

    // Check if we should retry
    if (_currentAttempt < _policy.maxAttempts && _shouldRetry(error, httpStatus)) {
        emit attemptFailed(_currentAttempt, error, _reply->errorString());
        _scheduleRetry();
    } else {
        _complete(false);
    }
}

void QGCNetworkRetry::_onRetryTimer()
{
    _delayUpdateTimer->stop();
    _setWaiting(false);
    _setRemainingDelayMs(0);

    if (!_cancelled) {
        _startAttempt();
    }
}

void QGCNetworkRetry::_onDelayUpdateTimer()
{
    const int remaining = _retryTimer->remainingTime();
    _setRemainingDelayMs(remaining > 0 ? remaining : 0);
}

// ============================================================================
// Private Methods
// ============================================================================

void QGCNetworkRetry::_setRunning(bool running)
{
    if (_running != running) {
        _running = running;
        emit runningChanged(running);
    }
}

void QGCNetworkRetry::_setWaiting(bool waiting)
{
    if (_waiting != waiting) {
        _waiting = waiting;
        emit waitingChanged(waiting);
    }
}

void QGCNetworkRetry::_setCurrentAttempt(int attempt)
{
    if (_currentAttempt != attempt) {
        _currentAttempt = attempt;
        emit currentAttemptChanged(attempt);
    }
}

void QGCNetworkRetry::_setRemainingDelayMs(int ms)
{
    if (_remainingDelayMs != ms) {
        _remainingDelayMs = ms;
        emit remainingDelayMsChanged(ms);
    }
}

void QGCNetworkRetry::_startAttempt()
{
    if (_cancelled) {
        return;
    }

    // Clean up previous reply
    if (_reply != nullptr) {
        _reply->deleteLater();
        _reply = nullptr;
    }

    _setCurrentAttempt(_currentAttempt + 1);
    qCDebug(QGCNetworkRetryLog) << "Starting attempt" << _currentAttempt << "of" << _policy.maxAttempts;

    // Create new request
    _reply = _requestFactory();
    if (_reply == nullptr) {
        qCWarning(QGCNetworkRetryLog) << "Request factory returned null";
        _complete(false);
        return;
    }

    connect(_reply, &QNetworkReply::finished, this, &QGCNetworkRetry::_onReplyFinished);
}

void QGCNetworkRetry::_scheduleRetry()
{
    const int delayMs = _calculateDelay();

    qCDebug(QGCNetworkRetryLog) << "Scheduling retry" << (_currentAttempt + 1)
                                 << "in" << delayMs << "ms";

    emit retrying(_currentAttempt + 1, delayMs);

    _setWaiting(true);
    _setRemainingDelayMs(delayMs);
    _retryTimer->start(delayMs);
    _delayUpdateTimer->start();
}

void QGCNetworkRetry::_complete(bool success)
{
    qCDebug(QGCNetworkRetryLog) << "Complete - success:" << success
                                 << "attempts:" << _currentAttempt;

    _retryTimer->stop();
    _delayUpdateTimer->stop();
    _setWaiting(false);

    if (_completionCallback) {
        _completionCallback(_reply, success);
    }

    emit finished(_reply, success, _currentAttempt);

    _setRunning(false);
}

bool QGCNetworkRetry::_shouldRetry(QNetworkReply::NetworkError error, int httpStatus) const
{
    // Don't retry on success
    if (error == QNetworkReply::NoError && httpStatus >= 200 && httpStatus < 300) {
        return false;
    }

    // Check timeout
    if (_policy.retryOnTimeout) {
        if (error == QNetworkReply::TimeoutError ||
            error == QNetworkReply::OperationCanceledError) {
            qCDebug(QGCNetworkRetryLog) << "Will retry on timeout";
            return true;
        }
    }

    // Check server errors (5xx)
    if (_policy.retryOn5xx && httpStatus >= 500 && httpStatus < 600) {
        qCDebug(QGCNetworkRetryLog) << "Will retry on server error:" << httpStatus;
        return true;
    }

    // Check connection errors
    if (_policy.retryOnConnectionError) {
        switch (error) {
        case QNetworkReply::ConnectionRefusedError:
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::HostNotFoundError:
        case QNetworkReply::TemporaryNetworkFailureError:
        case QNetworkReply::NetworkSessionFailedError:
        case QNetworkReply::UnknownNetworkError:
            qCDebug(QGCNetworkRetryLog) << "Will retry on connection error:" << error;
            return true;
        default:
            break;
        }
    }

    qCDebug(QGCNetworkRetryLog) << "Will not retry - error:" << error << "HTTP:" << httpStatus;
    return false;
}

int QGCNetworkRetry::_calculateDelay() const
{
    // Calculate base delay with exponential backoff
    // delay = initialDelay * (multiplier ^ (attempt - 1))
    const int attempt = _currentAttempt;  // Current failed attempt
    const double multiplier = std::pow(_policy.backoffMultiplier, attempt - 1);
    double delay = _policy.initialDelayMs * multiplier;

    // Apply jitter: delay * (1 - jitter/2 + random * jitter)
    // This gives a range of [delay * (1 - jitter/2), delay * (1 + jitter/2)]
    if (_policy.jitterFactor > 0.0) {
        std::uniform_real_distribution<double> dist(
            1.0 - _policy.jitterFactor / 2.0,
            1.0 + _policy.jitterFactor / 2.0);
        delay *= dist(const_cast<std::mt19937&>(_rng));
    }

    // Clamp to max delay
    delay = std::min(delay, static_cast<double>(_policy.maxDelayMs));

    return static_cast<int>(delay);
}
