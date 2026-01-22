#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QQueue>
#include <QtCore/QVariant>
#include <QtCore/QJsonObject>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCRetryQueueLog)

class QTimer;

/// Persistent queue that retries failed operations.
///
/// Features:
/// - Automatic retry with exponential backoff
/// - Persistence to disk for recovery after restart
/// - Connection-aware (pauses when offline)
/// - Priority support
///
/// Example usage:
/// @code
/// QGCRetryQueue queue("/path/to/queue.json");
/// queue.setProcessor([](const QVariant &data) {
///     return uploadToServer(data);  // Returns true on success
/// });
/// queue.enqueue(myData);
/// @endcode
class QGCRetryQueue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool processing READ isProcessing NOTIFY processingChanged)
    Q_PROPERTY(bool online READ isOnline WRITE setOnline NOTIFY onlineChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

public:
    using Processor = std::function<bool(const QVariant &data)>;

    struct Item {
        QString id;
        QVariant data;
        int attempts = 0;
        int priority = 0;
        qint64 createdAt = 0;
        qint64 lastAttempt = 0;
        qint64 nextRetry = 0;
        QString lastError;
    };

    explicit QGCRetryQueue(QObject *parent = nullptr);
    explicit QGCRetryQueue(const QString &persistPath, QObject *parent = nullptr);
    ~QGCRetryQueue() override;

    /// Set the processing function
    void setProcessor(Processor processor);

    /// Set persistence file path
    void setPersistPath(const QString &path);
    QString persistPath() const { return _persistPath; }

    // Configuration
    void setMaxAttempts(int max) { _maxAttempts = max; }
    int maxAttempts() const { return _maxAttempts; }

    void setInitialDelayMs(int ms) { _initialDelayMs = ms; }
    int initialDelayMs() const { return _initialDelayMs; }

    void setMaxDelayMs(int ms) { _maxDelayMs = ms; }
    int maxDelayMs() const { return _maxDelayMs; }

    void setBackoffMultiplier(double mult) { _backoffMultiplier = mult; }
    double backoffMultiplier() const { return _backoffMultiplier; }

    // Queue operations
    /// Enqueue an item for processing
    /// @return Item ID
    Q_INVOKABLE QString enqueue(const QVariant &data, int priority = 0);

    /// Remove an item by ID
    Q_INVOKABLE bool remove(const QString &id);

    /// Clear all items
    Q_INVOKABLE void clear();

    /// Force retry of failed items now
    Q_INVOKABLE void retryNow();

    // State
    int count() const { return _queue.size(); }
    bool isProcessing() const { return _processing; }
    bool isEmpty() const { return _queue.isEmpty(); }

    bool isOnline() const { return _online; }
    void setOnline(bool online);

    bool isPaused() const { return _paused; }
    void setPaused(bool paused);

    /// Get all items (for inspection)
    QList<Item> items() const { return _queue; }

    /// Get item by ID
    Item item(const QString &id) const;

    /// Save queue to disk
    Q_INVOKABLE bool save();

    /// Load queue from disk
    Q_INVOKABLE bool load();

signals:
    void countChanged(int count);
    void processingChanged(bool processing);
    void onlineChanged(bool online);
    void pausedChanged(bool paused);

    void itemEnqueued(const QString &id);
    void itemCompleted(const QString &id);
    void itemFailed(const QString &id, const QString &error);
    void itemDropped(const QString &id, const QString &reason);

private slots:
    void _processNext();

private:
    void _scheduleProcessing();
    int _calculateDelay(int attempts) const;
    Item *_findItem(const QString &id);

    QString _persistPath;
    Processor _processor;

    QList<Item> _queue;
    QTimer *_processTimer = nullptr;

    int _maxAttempts = 5;
    int _initialDelayMs = 1000;
    int _maxDelayMs = 60000;
    double _backoffMultiplier = 2.0;

    bool _processing = false;
    bool _online = true;
    bool _paused = false;
};
