/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file COPYING.md
 * in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QElapsedTimer>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QWaitCondition>
#include <QtCore/QString>
#include <QtCore/QThread>

#include <memory>

class QGCMapTask;
class QGCTileCacheDatabase;

class QGCCacheWorker : public QThread
{
    Q_OBJECT
public:
    explicit QGCCacheWorker(QObject* parent = nullptr);
    ~QGCCacheWorker() override;

    // Control
    void stop();
    bool enqueueTask(QGCMapTask* task);

    // Optional: set DB path before starting (falls back to AppDataLocation if empty)
    void setDatabasePath(const QString& path) { _databasePath = path; }

signals:
    void updateTotals(quint32 totalCount, quint64 totalSize, quint32 defaultCount, quint64 defaultSize);

protected:
    void run() override;

private:
    void _runTask(QGCMapTask* task);
    void _updateTotals();

private:
    // Task pipeline
    QMutex                      _taskQueueMutex;
    QQueue<QGCMapTask*>         _taskQueue;
    QWaitCondition              _waitc;

    // DB wrapper
    std::unique_ptr<QGCTileCacheDatabase> _cacheDB;
    QString                     _databasePath;

    // State
    bool                        _valid  = false;
    bool                        _failed = false;

    // Totals cache
    quint32                     _totalCount   = 0;
    quint64                     _totalSize    = 0;
    quint32                     _defaultCount = 0;
    quint64                     _defaultSize  = 0;

    // Update pacing
    QElapsedTimer               _updateTimer;
    int                         _updateTimeout = 0;

    // Tunables
    static constexpr int kShortTimeout = 1500;   // ms
    static constexpr int kLongTimeout  = 8000;   // ms
};
