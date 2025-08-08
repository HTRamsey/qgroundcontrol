/****************************************************************************
 *
 * (c) 2009-2025 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file COPYING.md
 * in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCTileCacheWorker.h"

#include "QGCTileCacheDatabase.h"
#include "QGCMapTasks.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

QGC_LOGGING_CATEGORY(QGCTileCacheWorkerLog, "qgc.qtlocationplugin.qgctilecacheworker")

QGCCacheWorker::QGCCacheWorker(QObject *parent)
    : QThread(parent)
{
    _updateTimeout = kShortTimeout;
}

QGCCacheWorker::~QGCCacheWorker() = default;

void QGCCacheWorker::stop()
{
    QMutexLocker lock(&_taskQueueMutex);
    qDeleteAll(_taskQueue);
    _taskQueue.clear();
    lock.unlock();

    if (isRunning()) {
        _waitc.wakeAll();
    }
}

bool QGCCacheWorker::enqueueTask(QGCMapTask *task)
{
    if (!_valid && (task->type() != QGCMapTask::taskInit)) {
        task->setError(tr("Database Not Initialized"));
        task->deleteLater();
        return false;
    }

    QMutexLocker lock(&_taskQueueMutex);
    _taskQueue.enqueue(task);
    lock.unlock();

    if (isRunning()) {
        _waitc.wakeAll();
    } else {
        start(QThread::HighPriority);
    }

    return true;
}

void QGCCacheWorker::run()
{
    // Ensure we have a DB path
    if (_databasePath.isEmpty()) {
        const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        _databasePath = base + QStringLiteral("/TileCache.db");
    }

    if (!_cacheDB) {
        _cacheDB = std::make_unique<QGCTileCacheDatabase>(_databasePath);
    }

    if (!_cacheDB->open()) {
        qCWarning(QGCTileCacheWorkerLog) << "Failed To Init/Open Cache DB at" << _databasePath;
        _failed = true;
        return;
    }

    _cacheDB->deleteBingNoTileTiles();

    _valid  = true;
    _failed = false;

    QMutexLocker lock(&_taskQueueMutex);
    while (true) {
        if (!_taskQueue.isEmpty()) {
            QGCMapTask* const task = _taskQueue.dequeue();
            lock.unlock();
            _runTask(task);
            lock.relock();
            task->deleteLater();

            const qsizetype count = _taskQueue.count();
            if (count > 100) {
                _updateTimeout = kLongTimeout;
            } else if (count < 25) {
                _updateTimeout = kShortTimeout;
            }

            if ((count == 0) || _updateTimer.hasExpired(_updateTimeout)) {
                if (_valid) {
                    lock.unlock();
                    _updateTotals();
                    lock.relock();
                }
            }
        } else {
            (void) _waitc.wait(lock.mutex(), 5000);
            if (_taskQueue.isEmpty()) {
                break;
            }
        }
    }
    lock.unlock();

    _cacheDB->close();
}

void QGCCacheWorker::_runTask(QGCMapTask *task)
{
    switch (task->type()) {
    case QGCMapTask::taskInit:
        break;
    case QGCMapTask::taskCacheTile:
        _cacheDB->saveTile(static_cast<QGCSaveTileTask*>(task));
        break;
    case QGCMapTask::taskFetchTile:
        _cacheDB->getTile(static_cast<QGCFetchTileTask*>(task));
        break;
    case QGCMapTask::taskFetchTileSets:
        _cacheDB->getTileSets(static_cast<QGCFetchTileSetTask*>(task));
        break;
    case QGCMapTask::taskCreateTileSet:
        _cacheDB->createTileSet(static_cast<QGCCreateTileSetTask*>(task));
        break;
    case QGCMapTask::taskGetTileDownloadList:
        _cacheDB->getTileDownloadList(static_cast<QGCGetTileDownloadListTask*>(task));
        break;
    case QGCMapTask::taskUpdateTileDownloadState:
        _cacheDB->updateTileDownloadState(static_cast<QGCUpdateTileDownloadStateTask*>(task));
        break;
    case QGCMapTask::taskDeleteTileSet:
        _cacheDB->deleteTileSet(static_cast<QGCDeleteTileSetTask*>(task));
        break;
    case QGCMapTask::taskRenameTileSet:
        _cacheDB->renameTileSet(static_cast<QGCRenameTileSetTask*>(task));
        break;
    case QGCMapTask::taskPruneCache:
        _cacheDB->pruneCache(static_cast<QGCPruneCacheTask*>(task));
        break;
    case QGCMapTask::taskReset:
        _cacheDB->resetDatabase(static_cast<QGCResetTask*>(task));
        break;
    case QGCMapTask::taskExport:
        _cacheDB->exportSets(static_cast<QGCExportTileTask*>(task));
        break;
    case QGCMapTask::taskImport:
        _cacheDB->importSets(static_cast<QGCImportTileTask*>(task));
        break;
    default:
        qCWarning(QGCTileCacheWorkerLog) << "given unhandled task type" << task->type();
        break;
    }
}

void QGCCacheWorker::_updateTotals()
{
    _cacheDB->queryTotals(_totalCount, _totalSize, _defaultCount, _defaultSize);
    emit updateTotals(_totalCount, _totalSize, _defaultCount, _defaultSize);

    if (!_updateTimer.isValid()) {
        _updateTimer.start();
    } else {
        (void) _updateTimer.restart();
    }
}
