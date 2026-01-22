#pragma once

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCRateLimiterLog)

class QTimer;

/// Rate limiter for controlling the frequency of operations.
/// Supports token bucket algorithm with burst capacity.
///
/// Example usage:
/// @code
/// QGCRateLimiter limiter(10, 1000);  // 10 requests per second
/// limiter.execute([this]() { makeApiCall(); });
/// @endcode
class QGCRateLimiter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int maxRequests READ maxRequests WRITE setMaxRequests NOTIFY maxRequestsChanged)
    Q_PROPERTY(int windowMs READ windowMs WRITE setWindowMs NOTIFY windowMsChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)
    Q_PROPERTY(int availableTokens READ availableTokens NOTIFY availableTokensChanged)
    Q_PROPERTY(bool limited READ isLimited NOTIFY limitedChanged)

public:
    using Operation = std::function<void()>;

    /// Create a rate limiter
    /// @param maxRequests Maximum requests allowed in the time window
    /// @param windowMs Time window in milliseconds
    /// @param parent Parent QObject
    explicit QGCRateLimiter(int maxRequests = 10, int windowMs = 1000, QObject *parent = nullptr);
    ~QGCRateLimiter() override;

    // Configuration
    int maxRequests() const { return _maxRequests; }
    void setMaxRequests(int max);

    int windowMs() const { return _windowMs; }
    void setWindowMs(int ms);

    /// Set burst capacity (extra tokens beyond maxRequests)
    void setBurstCapacity(int burst) { _burstCapacity = burst; }
    int burstCapacity() const { return _burstCapacity; }

    // State
    int pendingCount() const { return _pendingOperations.size(); }
    int availableTokens() const;
    bool isLimited() const { return availableTokens() <= 0; }

    /// Check if an operation can be executed immediately
    Q_INVOKABLE bool canExecute() const { return availableTokens() > 0; }

    /// Time until next token becomes available (ms)
    Q_INVOKABLE int timeUntilAvailable() const;

    /// Execute an operation (queued if rate limited)
    /// @param operation The operation to execute
    /// @param dropIfLimited If true, drop operation instead of queuing when limited
    /// @return true if executed immediately, false if queued or dropped
    bool execute(Operation operation, bool dropIfLimited = false);

    /// Try to execute immediately, return false if rate limited
    bool tryExecute(Operation operation);

    /// Clear all pending operations
    Q_INVOKABLE void clearPending();

    /// Reset the rate limiter state
    Q_INVOKABLE void reset();

signals:
    void maxRequestsChanged(int max);
    void windowMsChanged(int ms);
    void pendingCountChanged(int count);
    void availableTokensChanged(int tokens);
    void limitedChanged(bool limited);

    /// Emitted when an operation is executed
    void operationExecuted();

    /// Emitted when an operation is queued
    void operationQueued();

    /// Emitted when an operation is dropped
    void operationDropped();

private slots:
    void _processQueue();
    void _refillTokens();

private:
    void _recordRequest();
    void _updateTokens();

    int _maxRequests;
    int _windowMs;
    int _burstCapacity = 0;

    QQueue<Operation> _pendingOperations;
    QList<qint64> _requestTimestamps;  // For sliding window

    QTimer *_processTimer = nullptr;
    QTimer *_refillTimer = nullptr;

    int _tokens = 0;
    qint64 _lastRefillTime = 0;
};
