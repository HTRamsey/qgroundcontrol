#include "QGCScheduler.h"

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

Q_LOGGING_CATEGORY(QGCSchedulerLog, "Utilities.QGCScheduler")

QGCScheduler::QGCScheduler(QObject *parent)
    : QObject(parent)
    , _timer(new QTimer(this))
{
    _timer->setSingleShot(true);
    connect(_timer, &QTimer::timeout, this, &QGCScheduler::_tick);
}

QGCScheduler::~QGCScheduler()
{
    cancelAll();
}

QString QGCScheduler::schedule(const QString &id, int intervalMs, TaskFunction func)
{
    Task task;
    task.id = id;
    task.function = std::move(func);
    task.intervalMs = intervalMs;
    task.oneShot = false;
    task.nextRun = QDateTime::currentMSecsSinceEpoch() + intervalMs;

    _tasks.insert(id, task);
    emit taskCountChanged(taskCount());

    qCDebug(QGCSchedulerLog) << "Scheduled task" << id << "interval:" << intervalMs << "ms";

    _scheduleNext();
    return id;
}

QString QGCScheduler::scheduleOnce(const QString &id, int delayMs, TaskFunction func)
{
    Task task;
    task.id = id;
    task.function = std::move(func);
    task.intervalMs = delayMs;
    task.oneShot = true;
    task.nextRun = QDateTime::currentMSecsSinceEpoch() + delayMs;

    _tasks.insert(id, task);
    emit taskCountChanged(taskCount());

    qCDebug(QGCSchedulerLog) << "Scheduled one-shot task" << id << "delay:" << delayMs << "ms";

    _scheduleNext();
    return id;
}

QString QGCScheduler::scheduleCron(const QString &id, const QString &expression, TaskFunction func)
{
    const int intervalMs = _parseCronInterval(expression);
    if (intervalMs <= 0) {
        qCWarning(QGCSchedulerLog) << "Invalid cron expression:" << expression;
        return {};
    }
    return schedule(id, intervalMs, std::move(func));
}

int QGCScheduler::_parseCronInterval(const QString &expression)
{
    // Format: "@every 5m" or "@every 1h30m" or "@every 30s"
    static const QRegularExpression pattern(
        QStringLiteral(R"(@every\s+(?:(\d+)h)?(?:(\d+)m)?(?:(\d+)s)?)"),
        QRegularExpression::CaseInsensitiveOption);

    const auto match = pattern.match(expression);
    if (!match.hasMatch()) {
        return -1;
    }

    int ms = 0;
    if (!match.captured(1).isEmpty()) {
        ms += match.captured(1).toInt() * 3600000;  // hours
    }
    if (!match.captured(2).isEmpty()) {
        ms += match.captured(2).toInt() * 60000;    // minutes
    }
    if (!match.captured(3).isEmpty()) {
        ms += match.captured(3).toInt() * 1000;     // seconds
    }

    return ms > 0 ? ms : -1;
}

bool QGCScheduler::cancel(const QString &id)
{
    if (_tasks.remove(id) > 0) {
        emit taskCountChanged(taskCount());
        qCDebug(QGCSchedulerLog) << "Cancelled task" << id;
        _scheduleNext();
        return true;
    }
    return false;
}

void QGCScheduler::cancelAll()
{
    _tasks.clear();
    _timer->stop();
    emit taskCountChanged(0);
    qCDebug(QGCSchedulerLog) << "Cancelled all tasks";
}

bool QGCScheduler::pause(const QString &id)
{
    auto it = _tasks.find(id);
    if (it == _tasks.end()) {
        return false;
    }

    it->enabled = false;
    qCDebug(QGCSchedulerLog) << "Paused task" << id;
    return true;
}

bool QGCScheduler::resume(const QString &id)
{
    auto it = _tasks.find(id);
    if (it == _tasks.end()) {
        return false;
    }

    it->enabled = true;
    it->nextRun = QDateTime::currentMSecsSinceEpoch() + it->intervalMs;
    qCDebug(QGCSchedulerLog) << "Resumed task" << id;
    _scheduleNext();
    return true;
}

bool QGCScheduler::runNow(const QString &id)
{
    auto it = _tasks.find(id);
    if (it == _tasks.end() || !it->function) {
        return false;
    }

    qCDebug(QGCSchedulerLog) << "Running task now:" << id;

    try {
        it->function();
        it->lastRun = QDateTime::currentMSecsSinceEpoch();
        it->runCount++;
        emit taskExecuted(id);
    } catch (const std::exception &e) {
        emit taskFailed(id, QString::fromUtf8(e.what()));
        return false;
    }

    return true;
}

bool QGCScheduler::hasTask(const QString &id) const
{
    return _tasks.contains(id);
}

QGCScheduler::Task QGCScheduler::task(const QString &id) const
{
    return _tasks.value(id);
}

QList<QGCScheduler::Task> QGCScheduler::tasks() const
{
    return _tasks.values();
}

void QGCScheduler::setRunning(bool running)
{
    if (_running == running) {
        return;
    }

    _running = running;
    emit runningChanged(running);

    if (running) {
        _scheduleNext();
    } else {
        _timer->stop();
    }

    qCDebug(QGCSchedulerLog) << "Running:" << running;
}

void QGCScheduler::_tick()
{
    if (!_running) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QStringList toRemove;

    for (auto it = _tasks.begin(); it != _tasks.end(); ++it) {
        Task &task = it.value();

        if (!task.enabled || task.nextRun > now) {
            continue;
        }

        qCDebug(QGCSchedulerLog) << "Executing task" << task.id;

        try {
            if (task.function) {
                task.function();
            }
            task.lastRun = now;
            task.runCount++;
            emit taskExecuted(task.id);
        } catch (const std::exception &e) {
            emit taskFailed(task.id, QString::fromUtf8(e.what()));
        }

        if (task.oneShot) {
            toRemove.append(task.id);
        } else {
            task.nextRun = now + task.intervalMs;
        }
    }

    for (const QString &id : toRemove) {
        _tasks.remove(id);
    }

    if (!toRemove.isEmpty()) {
        emit taskCountChanged(taskCount());
    }

    _scheduleNext();
}

void QGCScheduler::_scheduleNext()
{
    if (!_running || _tasks.isEmpty()) {
        _timer->stop();
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 nextRun = std::numeric_limits<qint64>::max();

    for (const Task &task : _tasks) {
        if (task.enabled && task.nextRun < nextRun) {
            nextRun = task.nextRun;
        }
    }

    if (nextRun == std::numeric_limits<qint64>::max()) {
        _timer->stop();
        return;
    }

    const qint64 delay = std::max(0LL, nextRun - now);
    _timer->start(static_cast<int>(std::min(delay, static_cast<qint64>(INT_MAX))));
}
