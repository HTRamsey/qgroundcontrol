/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief Map Tile Cache Worker Thread
 *
 *   @author Gus Grubba <gus@auterion.com>
 *
 */

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtSql/QSqlDatabase>

#define LONG_TIMEOUT 5
#define SHORT_TIMEOUT 2

Q_DECLARE_LOGGING_CATEGORY(QGCTileCacheLog)

class QGCMapTask;
class QGCCachedTileSet;

class QGCCacheWorker : public QObject
{
    Q_OBJECT

public:
    explicit QGCCacheWorker(QObject *parent = nullptr);
    ~QGCCacheWorker();

    void quit();
    bool enqueueTask(QGCMapTask *task);
    void setDatabaseFilePath(const QString &path) { _databasePath = path; }

private:
    void run() final;

    void _runTask(QGCMapTask *task);

    void _saveTile(QGCMapTask *task);
    void _getTile(QGCMapTask *task);
    void _getTileSets(QGCMapTask *task);
    void _createTileSet(QGCMapTask *task);
    void _getTileDownloadList(QGCMapTask *task);
    void _updateTileDownloadState(QGCMapTask *task);
    void _deleteTileSet(QGCMapTask *task);
    void _renameTileSet(QGCMapTask *task);
    void _resetCacheDatabase(QGCMapTask *task);
    void _pruneCache(QGCMapTask *task);
    void _exportSets(QGCMapTask *task);
    void _importSets(QGCMapTask *task);
    bool _testTask(QGCMapTask *task);
    void _deleteBingNoTileTiles();

    bool _connectDB();
    bool _createDB(QSqlDatabase &db, bool createDefault = true);
    bool _findTileSetID(const QString &name, quint64 &setID);
    bool _init();
    quint64 _findTile(const QString &hash);
    quint64 _getDefaultTileSet();
    void _deleteTileSet(quint64 id);
    void _disconnectDB();
    void _updateSetTotals(QGCCachedTileSet *set);
    void _updateTotals();

signals:
    void updateTotals(quint32 totaltiles, quint64 totalsize, quint32 defaulttiles, quint64 defaultsize);

private:
    bool _failed = false;
    int _updateTimeout = SHORT_TIMEOUT;
    QMutex _taskQueueMutex;
    QQueue<QGCMapTask*> _taskQueue;
    QString _databasePath;
    quint32 _defaultCount = 0;
    quint32 _totalCount = 0;
    quint64 _defaultSet = UINT64_MAX;
    quint64 _defaultSize = 0;
    quint64 _totalSize = 0;
    QWaitCondition _waitc;
    std::atomic_bool _valid = false;
    time_t _lastUpdate = 0;
    std::unique_ptr<QSqlDatabase> _db = nullptr;
    QThread *_thread = nullptr;
};
