#pragma once

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>
#include <QtCore/QVariant>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCDebouncerLog)

class QTimer;

/// Debounces rapid calls, executing only after a quiet period.
/// Useful for search inputs, resize events, or any rapid updates.
///
/// Example usage:
/// @code
/// QGCDebouncer debouncer(300);  // 300ms delay
/// connect(searchInput, &QLineEdit::textChanged, [&](const QString &text) {
///     debouncer.call([this, text]() { performSearch(text); });
/// });
/// @endcode
class QGCDebouncer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int delayMs READ delayMs WRITE setDelayMs NOTIFY delayMsChanged)
    Q_PROPERTY(bool pending READ isPending NOTIFY pendingChanged)
    Q_PROPERTY(bool leading READ isLeading WRITE setLeading NOTIFY leadingChanged)
    Q_PROPERTY(bool trailing READ isTrailing WRITE setTrailing NOTIFY trailingChanged)

public:
    using Callback = std::function<void()>;

    /// Create a debouncer
    /// @param delayMs Delay in milliseconds before executing
    /// @param parent Parent QObject
    explicit QGCDebouncer(int delayMs = 250, QObject *parent = nullptr);
    ~QGCDebouncer() override;

    // Configuration
    int delayMs() const { return _delayMs; }
    void setDelayMs(int ms);

    /// Execute on leading edge (first call)
    bool isLeading() const { return _leading; }
    void setLeading(bool enabled);

    /// Execute on trailing edge (after delay) - default true
    bool isTrailing() const { return _trailing; }
    void setTrailing(bool enabled);

    /// Set maximum wait time (for trailing edge)
    void setMaxWait(int ms) { _maxWaitMs = ms; }
    int maxWait() const { return _maxWaitMs; }

    // State
    bool isPending() const { return _pending; }

    /// Schedule a callback to be executed after the delay
    void call(Callback callback);

    /// Schedule with a value that can be retrieved in triggered signal
    Q_INVOKABLE void call(const QVariant &value = {});

    /// Cancel any pending execution
    Q_INVOKABLE void cancel();

    /// Execute immediately if pending, then cancel
    Q_INVOKABLE void flush();

signals:
    void delayMsChanged(int ms);
    void pendingChanged(bool pending);
    void leadingChanged(bool leading);
    void trailingChanged(bool trailing);

    /// Emitted when debounced action should execute
    void triggered();

    /// Emitted with the last value passed to call()
    void triggeredWithValue(const QVariant &value);

private slots:
    void _onTimeout();

private:
    void _setPending(bool pending);
    void _execute();

    int _delayMs;
    int _maxWaitMs = 0;
    bool _leading = false;
    bool _trailing = true;
    bool _pending = false;
    bool _leadingExecuted = false;

    QTimer *_timer = nullptr;
    Callback _callback;
    QVariant _lastValue;
    qint64 _firstCallTime = 0;
};

/// Throttles calls to at most once per interval.
/// Unlike debouncing, throttling guarantees regular execution during rapid calls.
class QGCThrottler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalMsChanged)
    Q_PROPERTY(bool pending READ isPending NOTIFY pendingChanged)

public:
    using Callback = std::function<void()>;

    explicit QGCThrottler(int intervalMs = 100, QObject *parent = nullptr);
    ~QGCThrottler() override;

    int intervalMs() const { return _intervalMs; }
    void setIntervalMs(int ms);

    bool isPending() const { return _pending; }

    /// Execute on leading edge (first call) - default true
    void setLeading(bool enabled) { _leading = enabled; }
    bool isLeading() const { return _leading; }

    /// Execute on trailing edge (last call in interval)
    void setTrailing(bool enabled) { _trailing = enabled; }
    bool isTrailing() const { return _trailing; }

    void call(Callback callback);
    Q_INVOKABLE void call(const QVariant &value = {});
    Q_INVOKABLE void cancel();

signals:
    void intervalMsChanged(int ms);
    void pendingChanged(bool pending);
    void triggered();
    void triggeredWithValue(const QVariant &value);

private slots:
    void _onTimeout();

private:
    void _setPending(bool pending);

    int _intervalMs;
    bool _leading = true;
    bool _trailing = true;
    bool _pending = false;

    QTimer *_timer = nullptr;
    Callback _callback;
    QVariant _lastValue;
    qint64 _lastExecuteTime = 0;
};
