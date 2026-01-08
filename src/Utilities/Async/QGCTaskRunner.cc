#include "QGCTaskRunner.h"

#include <QtCore/QThreadPool>
#include <QtCore/QRunnable>
#include <QtCore/QUuid>

Q_LOGGING_CATEGORY(QGCTaskRunnerLog, "Utilities.QGCTaskRunner")

// ============================================================================
// QGCTask
// ============================================================================

QGCTask::QGCTask(TaskFunction func, Priority priority, QObject *parent)
    : QObject(parent)
    , _id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , _priority(priority)
    , _function(std::move(func))
{
}

void QGCTask::cancel()
{
    if (_state == State::Pending) {
        _setState(State::Cancelled);
        emit cancelled();
    }
}

void QGCTask::_execute()
{
    if (_state == State::Cancelled) {
        return;
    }

    _setState(State::Running);
    emit started();

    try {
        _result = _function();
        _setState(State::Completed);
        emit completed(_result);
    } catch (const std::exception &e) {
        _errorString = QString::fromUtf8(e.what());
        _setState(State::Failed);
        emit failed(_errorString);
    } catch (...) {
        _errorString = tr("Unknown error");
        _setState(State::Failed);
        emit failed(_errorString);
    }
}

void QGCTask::_setState(State state)
{
    _state = state;
}

// ============================================================================
// TaskRunnable - QRunnable wrapper for QGCTask
// ============================================================================

class TaskRunnable : public QRunnable
{
public:
    explicit TaskRunnable(QGCTask *task, QGCTaskRunner *runner)
        : _task(task)
        , _runner(runner)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        if (_task) {
            _task->_execute();

            // Notify runner via queued connection
            QMetaObject::invokeMethod(_runner, "_onTaskFinished", Qt::QueuedConnection);
        }
    }

private:
    QGCTask *_task;
    QGCTaskRunner *_runner;
};

// ============================================================================
// QGCTaskRunner
// ============================================================================

QGCTaskRunner::QGCTaskRunner(QObject *parent)
    : QGCTaskRunner(QThread::idealThreadCount(), parent)
{
}

QGCTaskRunner::QGCTaskRunner(int maxConcurrent, QObject *parent)
    : QObject(parent)
    , _threadPool(new QThreadPool(this))
    , _maxConcurrent(maxConcurrent)
{
    _threadPool->setMaxThreadCount(maxConcurrent);
}

QGCTaskRunner::~QGCTaskRunner()
{
    cancelAll();
    _threadPool->waitForDone();
}

QGCTask *QGCTaskRunner::submit(QGCTask::TaskFunction func, QGCTask::Priority priority)
{
    return submit({}, std::move(func), priority);
}

QGCTask *QGCTaskRunner::submit(const QString &description, QGCTask::TaskFunction func,
                                QGCTask::Priority priority)
{
    auto *task = new QGCTask(std::move(func), priority, this);
    task->setDescription(description);

    // Insert by priority (higher priority at front)
    int insertPos = 0;
    for (int i = 0; i < _pendingTasks.size(); ++i) {
        if (_pendingTasks.at(i)->priority() < priority) {
            insertPos = i;
            break;
        }
        insertPos = i + 1;
    }

    _pendingTasks.insert(insertPos, task);
    emit pendingCountChanged(pendingCount());

    qCDebug(QGCTaskRunnerLog) << "Task submitted:" << task->id()
                               << "priority:" << static_cast<int>(priority)
                               << "pending:" << pendingCount();

    _processQueue();

    return task;
}

void QGCTaskRunner::cancelAll()
{
    for (QGCTask *task : _pendingTasks) {
        task->cancel();
    }
    _pendingTasks.clear();
    emit pendingCountChanged(0);

    qCDebug(QGCTaskRunnerLog) << "All pending tasks cancelled";
}

void QGCTaskRunner::waitForAll()
{
    _threadPool->waitForDone();
}

void QGCTaskRunner::setMaxConcurrent(int max)
{
    if (max < 1) max = 1;
    if (_maxConcurrent == max) return;

    _maxConcurrent = max;
    _threadPool->setMaxThreadCount(max);
    emit maxConcurrentChanged(max);

    _processQueue();
}

void QGCTaskRunner::setPaused(bool paused)
{
    if (_paused == paused) return;

    _paused = paused;
    emit pausedChanged(paused);

    if (!paused) {
        _processQueue();
    }
}

void QGCTaskRunner::_onTaskFinished()
{
    _runningCount--;
    emit runningCountChanged(_runningCount);

    // Count completed/failed
    for (QGCTask *task : findChildren<QGCTask *>()) {
        if (task->state() == QGCTask::State::Completed) {
            _completedCount++;
        } else if (task->state() == QGCTask::State::Failed) {
            _failedCount++;
        }
    }

    emit taskCompleted(nullptr);  // Could track which task

    if (_pendingTasks.isEmpty() && _runningCount == 0) {
        emit allTasksCompleted();
        qCDebug(QGCTaskRunnerLog) << "All tasks completed - success:" << _completedCount
                                   << "failed:" << _failedCount;
    }

    _processQueue();
}

void QGCTaskRunner::_processQueue()
{
    if (_paused) {
        return;
    }

    while (_runningCount < _maxConcurrent && !_pendingTasks.isEmpty()) {
        QGCTask *task = _pendingTasks.takeFirst();
        emit pendingCountChanged(pendingCount());

        if (task->state() == QGCTask::State::Cancelled) {
            continue;
        }

        _startTask(task);
    }
}

void QGCTaskRunner::_startTask(QGCTask *task)
{
    _runningCount++;
    emit runningCountChanged(_runningCount);

    auto *runnable = new TaskRunnable(task, this);
    _threadPool->start(runnable);

    qCDebug(QGCTaskRunnerLog) << "Task started:" << task->id()
                               << "running:" << _runningCount;
}
