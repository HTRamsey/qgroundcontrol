#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>

#include <functional>

Q_DECLARE_LOGGING_CATEGORY(QGCSchedulerLog)

class QTimer;

/// Cron-like task scheduler for periodic operations.
///
/// Example usage:
/// @code
/// QGCScheduler scheduler;
/// scheduler.schedule("cleanup", 60000, []() { cleanupOldFiles(); });
/// scheduler.scheduleOnce("startup", 5000, []() { initializeSystem(); });
/// @endcode
class QGCScheduler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int taskCount READ taskCount NOTIFY taskCountChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)

public:
    using TaskFunction = std::function<void()>;

    struct Task {
        QString id;
        TaskFunction function;
        int intervalMs = 0;
        bool oneShot = false;
        bool enabled = true;
        qint64 lastRun = 0;
        qint64 nextRun = 0;
        int runCount = 0;
    };

    explicit QGCScheduler(QObject *parent = nullptr);
    ~QGCScheduler() override;

    /// Schedule a recurring task
    /// @return Task ID
    QString schedule(const QString &id, int intervalMs, TaskFunction func);

    /// Schedule a one-time task
    QString scheduleOnce(const QString &id, int delayMs, TaskFunction func);

    /// Schedule using cron-like expression (simplified: supports interval only)
    /// Format: "@every 5m" or "@every 1h30m" or "@every 30s"
    QString scheduleCron(const QString &id, const QString &expression, TaskFunction func);

    /// Cancel a scheduled task
    Q_INVOKABLE bool cancel(const QString &id);

    /// Cancel all tasks
    Q_INVOKABLE void cancelAll();

    /// Pause a task
    Q_INVOKABLE bool pause(const QString &id);

    /// Resume a task
    Q_INVOKABLE bool resume(const QString &id);

    /// Run a task immediately (in addition to its schedule)
    Q_INVOKABLE bool runNow(const QString &id);

    /// Check if task exists
    Q_INVOKABLE bool hasTask(const QString &id) const;

    /// Get task info
    Task task(const QString &id) const;

    /// Get all tasks
    QList<Task> tasks() const;

    // State
    int taskCount() const { return _tasks.size(); }
    bool isRunning() const { return _running; }
    void setRunning(bool running);

signals:
    void taskCountChanged(int count);
    void runningChanged(bool running);
    void taskExecuted(const QString &id);
    void taskFailed(const QString &id, const QString &error);

private slots:
    void _tick();

private:
    void _scheduleNext();
    static int _parseCronInterval(const QString &expression);

    QHash<QString, Task> _tasks;
    QTimer *_timer = nullptr;
    bool _running = true;
};
