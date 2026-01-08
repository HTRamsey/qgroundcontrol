#pragma once

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QLoggingCategory>

#include <functional>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(QGCTaskRunnerLog)

class QThread;
class QThreadPool;

/// Represents a task to be executed
class QGCTask : public QObject
{
    Q_OBJECT

public:
    enum class Priority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };
    Q_ENUM(Priority)

    enum class State {
        Pending,
        Running,
        Completed,
        Failed,
        Cancelled
    };
    Q_ENUM(State)

    using TaskFunction = std::function<QVariant()>;

    explicit QGCTask(TaskFunction func, Priority priority = Priority::Normal, QObject *parent = nullptr);
    ~QGCTask() override = default;

    QString id() const { return _id; }
    Priority priority() const { return _priority; }
    State state() const { return _state; }
    QVariant result() const { return _result; }
    QString errorString() const { return _errorString; }

    /// Set a description for debugging/logging
    void setDescription(const QString &desc) { _description = desc; }
    QString description() const { return _description; }

    /// Cancel the task (if not yet running)
    void cancel();

    /// Check if task can be cancelled
    bool isCancellable() const { return _state == State::Pending; }

signals:
    void started();
    void completed(const QVariant &result);
    void failed(const QString &error);
    void cancelled();
    void progressChanged(int percent);

private:
    friend class QGCTaskRunner;
    friend class TaskRunnable;

    void _execute();
    void _setState(State state);

    QString _id;
    QString _description;
    Priority _priority;
    State _state = State::Pending;
    TaskFunction _function;
    QVariant _result;
    QString _errorString;
};

/// Runs tasks in background threads with priority queue.
///
/// Example usage:
/// @code
/// QGCTaskRunner runner;
/// auto task = runner.submit([]() {
///     return heavyComputation();
/// });
/// connect(task, &QGCTask::completed, this, [](const QVariant &result) {
///     qDebug() << "Result:" << result;
/// });
/// @endcode
class QGCTaskRunner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)
    Q_PROPERTY(int runningCount READ runningCount NOTIFY runningCountChanged)
    Q_PROPERTY(int maxConcurrent READ maxConcurrent WRITE setMaxConcurrent NOTIFY maxConcurrentChanged)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

public:
    explicit QGCTaskRunner(QObject *parent = nullptr);
    explicit QGCTaskRunner(int maxConcurrent, QObject *parent = nullptr);
    ~QGCTaskRunner() override;

    /// Submit a task for execution
    /// @param func The function to execute
    /// @param priority Task priority
    /// @return The task object (for tracking)
    QGCTask *submit(QGCTask::TaskFunction func, QGCTask::Priority priority = QGCTask::Priority::Normal);

    /// Submit with a description
    QGCTask *submit(const QString &description, QGCTask::TaskFunction func,
                    QGCTask::Priority priority = QGCTask::Priority::Normal);

    /// Cancel all pending tasks
    Q_INVOKABLE void cancelAll();

    /// Wait for all tasks to complete
    Q_INVOKABLE void waitForAll();

    // State
    int pendingCount() const { return _pendingTasks.size(); }
    int runningCount() const { return _runningCount; }
    int completedCount() const { return _completedCount; }
    int failedCount() const { return _failedCount; }

    // Configuration
    int maxConcurrent() const { return _maxConcurrent; }
    void setMaxConcurrent(int max);

    bool isPaused() const { return _paused; }
    void setPaused(bool paused);

signals:
    void pendingCountChanged(int count);
    void runningCountChanged(int count);
    void maxConcurrentChanged(int max);
    void pausedChanged(bool paused);

    /// Emitted when a task completes (success or failure)
    void taskCompleted(QGCTask *task);

    /// Emitted when all tasks are done
    void allTasksCompleted();

private slots:
    void _onTaskFinished();

private:
    void _processQueue();
    void _startTask(QGCTask *task);

    QThreadPool *_threadPool = nullptr;

    QList<QGCTask *> _pendingTasks;
    int _runningCount = 0;
    int _completedCount = 0;
    int _failedCount = 0;

    int _maxConcurrent = 4;
    bool _paused = false;
};
