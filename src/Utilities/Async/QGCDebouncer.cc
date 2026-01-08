#include "QGCDebouncer.h"

#include <QtCore/QTimer>
#include <QtCore/QDateTime>

Q_LOGGING_CATEGORY(QGCDebouncerLog, "Utilities.QGCDebouncer")

// ============================================================================
// QGCDebouncer
// ============================================================================

QGCDebouncer::QGCDebouncer(int delayMs, QObject *parent)
    : QObject(parent)
    , _delayMs(delayMs)
    , _timer(new QTimer(this))
{
    _timer->setSingleShot(true);
    connect(_timer, &QTimer::timeout, this, &QGCDebouncer::_onTimeout);
}

QGCDebouncer::~QGCDebouncer()
{
    cancel();
}

void QGCDebouncer::setDelayMs(int ms)
{
    if (ms < 0) ms = 0;
    if (_delayMs == ms) return;

    _delayMs = ms;
    emit delayMsChanged(ms);
}

void QGCDebouncer::setLeading(bool enabled)
{
    if (_leading == enabled) return;

    _leading = enabled;
    emit leadingChanged(enabled);
}

void QGCDebouncer::setTrailing(bool enabled)
{
    if (_trailing == enabled) return;

    _trailing = enabled;
    emit trailingChanged(enabled);
}

void QGCDebouncer::call(Callback callback)
{
    _callback = std::move(callback);
    call(QVariant());
}

void QGCDebouncer::call(const QVariant &value)
{
    _lastValue = value;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Track first call time for maxWait
    if (!_pending) {
        _firstCallTime = now;
        _leadingExecuted = false;
    }

    // Leading edge execution
    if (_leading && !_leadingExecuted) {
        _leadingExecuted = true;
        _execute();

        if (!_trailing) {
            return;  // Only leading, no trailing
        }
    }

    _setPending(true);

    // Check maxWait
    if (_maxWaitMs > 0 && (now - _firstCallTime) >= _maxWaitMs) {
        _execute();
        _firstCallTime = now;
    }

    // Restart timer for trailing edge
    _timer->start(_delayMs);
}

void QGCDebouncer::cancel()
{
    _timer->stop();
    _callback = nullptr;
    _setPending(false);
    _leadingExecuted = false;
}

void QGCDebouncer::flush()
{
    if (_pending) {
        _timer->stop();
        _execute();
    }
}

void QGCDebouncer::_onTimeout()
{
    if (_trailing) {
        _execute();
    }
    _setPending(false);
    _leadingExecuted = false;
}

void QGCDebouncer::_setPending(bool pending)
{
    if (_pending == pending) return;

    _pending = pending;
    emit pendingChanged(pending);
}

void QGCDebouncer::_execute()
{
    if (_callback) {
        _callback();
    }
    emit triggered();
    emit triggeredWithValue(_lastValue);
}

// ============================================================================
// QGCThrottler
// ============================================================================

QGCThrottler::QGCThrottler(int intervalMs, QObject *parent)
    : QObject(parent)
    , _intervalMs(intervalMs)
    , _timer(new QTimer(this))
{
    _timer->setSingleShot(true);
    connect(_timer, &QTimer::timeout, this, &QGCThrottler::_onTimeout);
}

QGCThrottler::~QGCThrottler()
{
    cancel();
}

void QGCThrottler::setIntervalMs(int ms)
{
    if (ms < 0) ms = 0;
    if (_intervalMs == ms) return;

    _intervalMs = ms;
    emit intervalMsChanged(ms);
}

void QGCThrottler::call(Callback callback)
{
    _callback = std::move(callback);
    call(QVariant());
}

void QGCThrottler::call(const QVariant &value)
{
    _lastValue = value;

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 elapsed = now - _lastExecuteTime;

    if (elapsed >= _intervalMs) {
        // Can execute immediately
        if (_leading) {
            _lastExecuteTime = now;
            if (_callback) {
                _callback();
            }
            emit triggered();
            emit triggeredWithValue(_lastValue);
        }

        if (_trailing && !_leading) {
            // Schedule for trailing edge
            _setPending(true);
            _timer->start(_intervalMs);
        }
    } else {
        // Within throttle interval, schedule trailing
        if (_trailing) {
            _setPending(true);
            if (!_timer->isActive()) {
                _timer->start(_intervalMs - static_cast<int>(elapsed));
            }
        }
    }
}

void QGCThrottler::cancel()
{
    _timer->stop();
    _callback = nullptr;
    _setPending(false);
}

void QGCThrottler::_onTimeout()
{
    _lastExecuteTime = QDateTime::currentMSecsSinceEpoch();
    _setPending(false);

    if (_callback) {
        _callback();
    }
    emit triggered();
    emit triggeredWithValue(_lastValue);
}

void QGCThrottler::_setPending(bool pending)
{
    if (_pending == pending) return;

    _pending = pending;
    emit pendingChanged(pending);
}
