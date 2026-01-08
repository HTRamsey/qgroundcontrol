#pragma once

/// @file QGCNetworkRetry.h
/// @brief Network retry policy with exponential backoff

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkReply>

#include <functional>
#include <random>

Q_DECLARE_LOGGING_CATEGORY(QGCNetworkRetryLog)

/// Retry policy configuration
struct QGCRetryPolicy
{
    int maxAttempts = 3;              ///< Maximum number of attempts (including initial)
    int initialDelayMs = 1000;        ///< Initial delay before first retry (ms)
    int maxDelayMs = 30000;           ///< Maximum delay between retries (ms)
    double backoffMultiplier = 2.0;   ///< Multiplier for exponential backoff
    double jitterFactor = 0.1;        ///< Random jitter factor (0.0-1.0)
    bool retryOnTimeout = true;       ///< Retry on timeout errors
    bool retryOn5xx = true;           ///< Retry on server errors (5xx)
    bool retryOnConnectionError = true; ///< Retry on connection failures

    /// Create default policy
    static QGCRetryPolicy defaultPolicy() { return {}; }

    /// Create aggressive retry policy (more attempts, shorter delays)
    static QGCRetryPolicy aggressive() {
        return {5, 500, 10000, 1.5, 0.2, true, true, true};
    }

    /// Create conservative retry policy (fewer attempts, longer delays)
    static QGCRetryPolicy conservative() {
        return {2, 2000, 60000, 3.0, 0.1, true, true, false};
    }

    /// Create no-retry policy (single attempt)
    static QGCRetryPolicy noRetry() {
        return {1, 0, 0, 1.0, 0.0, false, false, false};
    }
};

/// Network request retry handler with exponential backoff
///
/// Features:
/// - Configurable retry policy
/// - Exponential backoff with jitter
/// - Automatic retry on transient errors
/// - Progress tracking
/// - Cancellation support
///
/// Example usage with callback:
/// @code
/// auto *retry = new QGCNetworkRetry(this);
/// retry->setPolicy(QGCRetryPolicy::aggressive());
///
/// retry->execute([this]() {
///     return networkManager->get(request);
/// }, [this](QNetworkReply *reply, bool success) {
///     if (success) {
///         processResponse(reply->readAll());
///     } else {
///         handleError(reply->errorString());
///     }
/// });
/// @endcode
///
/// Example usage with signals:
/// @code
/// auto *retry = new QGCNetworkRetry(this);
/// connect(retry, &QGCNetworkRetry::finished, this,
///         [](QNetworkReply *reply, bool success, int attempts) {
///     qDebug() << "Completed after" << attempts << "attempts";
/// });
/// connect(retry, &QGCNetworkRetry::retrying, this,
///         [](int attempt, int delayMs) {
///     qDebug() << "Retry" << attempt << "in" << delayMs << "ms";
/// });
/// retry->execute([this]() { return networkManager->get(request); });
/// @endcode
class QGCNetworkRetry : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QGCNetworkRetry)

    /// Current attempt number (1-based)
    Q_PROPERTY(int currentAttempt READ currentAttempt NOTIFY currentAttemptChanged FINAL)

    /// Maximum attempts from policy
    Q_PROPERTY(int maxAttempts READ maxAttempts NOTIFY maxAttemptsChanged FINAL)

    /// Whether a retry operation is in progress
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged FINAL)

    /// Whether waiting for retry delay
    Q_PROPERTY(bool waiting READ isWaiting NOTIFY waitingChanged FINAL)

    /// Remaining delay before next retry (ms)
    Q_PROPERTY(int remainingDelayMs READ remainingDelayMs NOTIFY remainingDelayMsChanged FINAL)

public:
    /// Request factory function type
    using RequestFactory = std::function<QNetworkReply*()>;

    /// Completion callback type
    using CompletionCallback = std::function<void(QNetworkReply*, bool success)>;

    explicit QGCNetworkRetry(QObject *parent = nullptr);
    explicit QGCNetworkRetry(const QGCRetryPolicy &policy, QObject *parent = nullptr);
    ~QGCNetworkRetry() override;

    // Property getters
    int currentAttempt() const { return _currentAttempt; }
    int maxAttempts() const { return _policy.maxAttempts; }
    bool isRunning() const { return _running; }
    bool isWaiting() const { return _waiting; }
    int remainingDelayMs() const { return _remainingDelayMs; }

    /// Get current retry policy
    const QGCRetryPolicy &policy() const { return _policy; }

    /// Set retry policy (must be called before execute())
    void setPolicy(const QGCRetryPolicy &policy);

    /// Get the last network reply (may be nullptr)
    QNetworkReply *lastReply() const { return _reply; }

public slots:
    /// Execute request with retry policy
    /// @param factory Function that creates a new QNetworkReply for each attempt
    /// @param callback Optional callback called when complete (success or all retries exhausted)
    /// @return true if execution started
    bool execute(RequestFactory factory, CompletionCallback callback = nullptr);

    /// Cancel current operation (including pending retry)
    void cancel();

    /// Reset state for reuse
    void reset();

signals:
    /// Emitted when attempt number changes
    void currentAttemptChanged(int attempt);

    /// Emitted when max attempts changes (policy changed)
    void maxAttemptsChanged(int maxAttempts);

    /// Emitted when running state changes
    void runningChanged(bool running);

    /// Emitted when waiting state changes
    void waitingChanged(bool waiting);

    /// Emitted when remaining delay changes
    void remainingDelayMsChanged(int remainingMs);

    /// Emitted before each retry attempt
    /// @param attempt Upcoming attempt number (2 = first retry)
    /// @param delayMs Delay before retry in milliseconds
    void retrying(int attempt, int delayMs);

    /// Emitted when an attempt fails but will be retried
    /// @param attempt Failed attempt number
    /// @param error Network error code
    /// @param errorString Error description
    void attemptFailed(int attempt, QNetworkReply::NetworkError error, const QString &errorString);

    /// Emitted when operation completes (success or all retries exhausted)
    /// @param reply The final network reply
    /// @param success true if request succeeded
    /// @param totalAttempts Total number of attempts made
    void finished(QNetworkReply *reply, bool success, int totalAttempts);

private slots:
    void _onReplyFinished();
    void _onRetryTimer();
    void _onDelayUpdateTimer();

private:
    void _setRunning(bool running);
    void _setWaiting(bool waiting);
    void _setCurrentAttempt(int attempt);
    void _setRemainingDelayMs(int ms);
    void _startAttempt();
    void _scheduleRetry();
    void _complete(bool success);
    bool _shouldRetry(QNetworkReply::NetworkError error, int httpStatus) const;
    int _calculateDelay() const;

    QGCRetryPolicy _policy;
    RequestFactory _requestFactory;
    CompletionCallback _completionCallback;

    QNetworkReply *_reply = nullptr;
    QTimer *_retryTimer = nullptr;
    QTimer *_delayUpdateTimer = nullptr;

    std::mt19937 _rng;

    int _currentAttempt = 0;
    int _remainingDelayMs = 0;
    bool _running = false;
    bool _waiting = false;
    bool _cancelled = false;
};
