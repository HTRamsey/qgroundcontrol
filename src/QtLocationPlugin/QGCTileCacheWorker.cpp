/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCTileCacheWorker.h"
#include "QGCTileCacheDatabase.h"
#include "QGCCachedTileSet.h"
#include "QGCMapTasks.h"
#include "QGCMapUrlEngine.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QDateTime>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QSettings>

QGC_LOGGING_CATEGORY(QGCTileCacheWorkerLog, "qgc.qtlocationplugin.qgctilecacheworker")

QGCCacheWorker::QGCCacheWorker(QObject *parent)
    : QThread(parent)
    , _database(std::make_unique<QGCTileCacheDatabase>())
{
    qCDebug(QGCTileCacheWorkerLog) << this;
}

QGCCacheWorker::~QGCCacheWorker()
{
    qCDebug(QGCTileCacheWorkerLog) << this;
}

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
    if (!_valid && !_failed) {
        if (!_init()) {
            qCWarning(QGCTileCacheWorkerLog) << "Failed To Init Database";
            return;
        }
    }

    if (_valid) {
        _deleteBingNoTileTiles();
    }

    QMutexLocker lock(&_taskQueueMutex);
    while (true) {
        if (!_taskQueue.isEmpty()) {
            QGCMapTask* task = _taskQueue.dequeue();
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
}

void QGCCacheWorker::_runTask(QGCMapTask *task)
{
    switch (task->type()) {
    case QGCMapTask::taskInit:
        break;
    case QGCMapTask::taskCacheTile:
        _saveTile(task);
        break;
    case QGCMapTask::taskFetchTile:
        _getTile(task);
        break;
    case QGCMapTask::taskFetchTileSets:
        _getTileSets(task);
        break;
    case QGCMapTask::taskCreateTileSet:
        _createTileSet(task);
        break;
    case QGCMapTask::taskGetTileDownloadList:
        _getTileDownloadList(task);
        break;
    case QGCMapTask::taskUpdateTileDownloadState:
        _updateTileDownloadState(task);
        break;
    case QGCMapTask::taskDeleteTileSet:
        _deleteTileSet(task);
        break;
    case QGCMapTask::taskRenameTileSet:
        _renameTileSet(task);
        break;
    case QGCMapTask::taskPruneCache:
        _pruneCache(task);
        break;
    case QGCMapTask::taskReset:
        _resetCacheDatabase(task);
        break;
    case QGCMapTask::taskExport:
        _exportSets(task);
        break;
    case QGCMapTask::taskImport:
        _importSets(task);
        break;
    default:
        qCWarning(QGCTileCacheWorkerLog) << "given unhandled task type" << task->type();
        break;
    }
}

bool QGCCacheWorker::_init()
{
    _failed = false;
    if (!_databasePath.isEmpty()) {
        qCDebug(QGCTileCacheWorkerLog) << "Mapping cache directory:" << _databasePath;

        if (_database->open(_databasePath)) {
            _valid = _database->createSchema(true);
            if (!_valid) {
                qCCritical(QGCTileCacheWorkerLog) << "Failed to create database schema:" << _database->lastError();
                _failed = true;
            }
        } else {
            qCCritical(QGCTileCacheWorkerLog) << "Failed to open database:" << _database->lastError();
            _failed = true;
        }
    } else {
        qCCritical(QGCTileCacheWorkerLog) << "Could not find suitable cache directory.";
        _failed = true;
    }

    return !_failed;
}

bool QGCCacheWorker::_testTask(QGCMapTask *mtask)
{
    if (!_valid) {
        mtask->setError("No Cache Database");
        return false;
    }
    return true;
}

void QGCCacheWorker::_deleteBingNoTileTiles()
{
    static const QString alreadyDoneKey = QStringLiteral("_deleteBingNoTileTilesDone");

    QSettings settings;
    if (settings.value(alreadyDoneKey, false).toBool()) {
        return;
    }
    settings.setValue(alreadyDoneKey, true);

    QFile file(QStringLiteral(":/res/BingNoTileBytes.dat"));
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(QGCTileCacheWorkerLog) << "Failed to Open File" << file.fileName() << ":" << file.errorString();
        return;
    }

    const QByteArray noTileBytes = file.readAll();
    file.close();

    _database->deleteTilesMatchingBytes(noTileBytes);
}

void QGCCacheWorker::_saveTile(QGCMapTask *mtask)
{
    if (!_valid) {
        qCWarning(QGCTileCacheWorkerLog) << "Map Cache error (saveTile): Database not valid";
        return;
    }

    QGCSaveTileTask *task = static_cast<QGCSaveTileTask*>(mtask);

    TileInfo tile;
    tile.hash = task->tile()->hash();
    tile.format = task->tile()->format();
    tile.tile = task->tile()->img();
    tile.size = task->tile()->img().size();
    tile.type = task->tile()->type();
    tile.date = QDateTime::currentSecsSinceEpoch();

    const quint64 setID = task->tile()->tileSet() == UINT64_MAX ?
        _database->getDefaultTileSet() : task->tile()->tileSet();

    if (!_database->saveTile(tile, setID)) {
        // Tile might already exist - not necessarily an error
        qCDebug(QGCTileCacheWorkerLog) << "Tile already exists or save failed for HASH:" << tile.hash;
    } else {
        qCDebug(QGCTileCacheWorkerLog) << "Saved tile HASH:" << tile.hash;
    }
}

void QGCCacheWorker::_getTile(QGCMapTask* mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCFetchTileTask *task = static_cast<QGCFetchTileTask*>(mtask);
    TileInfo tile;

    if (_database->getTile(task->hash(), tile)) {
        qCDebug(QGCTileCacheWorkerLog) << "(Found in DB) HASH:" << task->hash();
        QGCCacheTile *cacheTile = new QGCCacheTile(task->hash(), tile.tile, tile.format, tile.type);
        task->setTileFetched(cacheTile);
    } else {
        qCDebug(QGCTileCacheWorkerLog) << "(NOT in DB) HASH:" << task->hash();
        task->setError("Tile not in cache database");
    }
}

void QGCCacheWorker::_getTileSets(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCFetchTileSetTask *task = static_cast<QGCFetchTileSetTask*>(mtask);
    QList<TileSetInfo> sets;

    if (!_database->getTileSets(sets)) {
        task->setError("Failed to fetch tile sets");
        return;
    }

    for (const TileSetInfo &info : sets) {
        QGCCachedTileSet *set = new QGCCachedTileSet(info.name);
        set->setId(info.setID);
        set->setMapTypeStr(info.typeStr);
        set->setTopleftLat(info.topleftLat);
        set->setTopleftLon(info.topleftLon);
        set->setBottomRightLat(info.bottomRightLat);
        set->setBottomRightLon(info.bottomRightLon);
        set->setMinZoom(info.minZoom);
        set->setMaxZoom(info.maxZoom);
        set->setType(UrlFactory::getProviderTypeFromQtMapId(info.type));
        set->setTotalTileCount(info.numTiles);
        set->setDefaultSet(info.defaultSet);
        set->setCreationDate(QDateTime::fromSecsSinceEpoch(info.date));

        _updateSetTotals(set);

        // Move to app thread
        (void) set->moveToThread(QCoreApplication::instance()->thread());
        task->setTileSetFetched(set);
    }
}

void QGCCacheWorker::_updateSetTotals(QGCCachedTileSet *set)
{
    if (set->defaultSet()) {
        _updateTotals();
        set->setSavedTileCount(_totalCount);
        set->setSavedTileSize(_totalSize);
        set->setTotalTileCount(_defaultCount);
        set->setTotalTileSize(_defaultSize);
        return;
    }

    quint32 savedCount = 0, uniqueCount = 0;
    quint64 savedSize = 0, uniqueSize = 0;

    if (_database->getSetStatistics(set->id(), savedCount, savedSize, uniqueCount, uniqueSize)) {
        set->setSavedTileCount(savedCount);
        set->setSavedTileSize(savedSize);

        // Update estimated size
        quint64 avg = UrlFactory::averageSizeForType(set->type());
        if (set->totalTileCount() <= set->savedTileCount()) {
            set->setTotalTileSize(set->savedTileSize());
        } else {
            if ((set->savedTileCount() > 10) && set->savedTileSize()) {
                avg = set->savedTileSize() / set->savedTileCount();
            }
            set->setTotalTileSize(avg * set->totalTileCount());
        }

        // Set unique counts
        quint32 expectedUcount = set->totalTileCount() - set->savedTileCount();
        if (uniqueCount == 0) {
            uniqueSize = expectedUcount * avg;
        } else {
            expectedUcount = uniqueCount;
        }
        set->setUniqueTileCount(expectedUcount);
        set->setUniqueTileSize(uniqueSize);
    }
}

void QGCCacheWorker::_updateTotals()
{
    DatabaseTotals totals;
    if (_database->getTotals(totals)) {
        _totalCount = totals.totalCount;
        _totalSize = totals.totalSize;
        _defaultCount = totals.defaultCount;
        _defaultSize = totals.defaultSize;

        emit updateTotals(_totalCount, _totalSize, _defaultCount, _defaultSize);
    }

    if (!_updateTimer.isValid()) {
        _updateTimer.start();
    } else {
        (void) _updateTimer.restart();
    }
}

void QGCCacheWorker::_createTileSet(QGCMapTask *mtask)
{
    if (!_valid) {
        mtask->setError("Error saving tile set");
        return;
    }

    QGCCreateTileSetTask *task = static_cast<QGCCreateTileSetTask*>(mtask);

    // Create tile set info
    TileSetInfo info;
    info.name = task->tileSet()->name();
    info.typeStr = task->tileSet()->mapTypeStr();
    info.topleftLat = task->tileSet()->topleftLat();
    info.topleftLon = task->tileSet()->topleftLon();
    info.bottomRightLat = task->tileSet()->bottomRightLat();
    info.bottomRightLon = task->tileSet()->bottomRightLon();
    info.minZoom = task->tileSet()->minZoom();
    info.maxZoom = task->tileSet()->maxZoom();
    info.type = UrlFactory::getQtMapIdFromProviderType(task->tileSet()->type());
    info.numTiles = task->tileSet()->totalTileCount();
    info.date = QDateTime::currentSecsSinceEpoch();

    quint64 setID;
    if (!_database->createTileSet(info, setID)) {
        mtask->setError("Error creating tile set");
        return;
    }

    task->tileSet()->setId(setID);

    // Prepare download list
    _database->beginTransaction();

    for (int z = task->tileSet()->minZoom(); z <= task->tileSet()->maxZoom(); z++) {
        const QGCTileSet set = UrlFactory::getTileCount(z,
            task->tileSet()->topleftLon(), task->tileSet()->topleftLat(),
            task->tileSet()->bottomRightLon(), task->tileSet()->bottomRightLat(),
            task->tileSet()->type());

        const QString type = task->tileSet()->type();

        for (int x = set.tileX0; x <= set.tileX1; x++) {
            for (int y = set.tileY0; y <= set.tileY1; y++) {
                const QString hash = UrlFactory::getTileHash(type, x, y, z);
                const quint64 tileID = _database->findTileID(hash);

                if (tileID == 0) {
                    // Add to download queue
                    TileDownloadInfo dlInfo;
                    dlInfo.hash = hash;
                    dlInfo.type = UrlFactory::getQtMapIdFromProviderType(type);
                    dlInfo.x = x;
                    dlInfo.y = y;
                    dlInfo.z = z;
                    dlInfo.state = 0;

                    if (!_database->addToDownloadQueue(setID, dlInfo)) {
                        qCWarning(QGCTileCacheWorkerLog) << "Failed to add tile to download queue";
                    }
                } else {
                    // Tile exists, add to set
                    if (!_database->addTileToSet(tileID, setID)) {
                        qCWarning(QGCTileCacheWorkerLog) << "Failed to add existing tile to set";
                    }
                    qCDebug(QGCTileCacheWorkerLog) << "Already Cached HASH:" << hash;
                }
            }
        }
    }

    _database->commitTransaction();
    _updateSetTotals(task->tileSet());
    task->setTileSetSaved();
}

void QGCCacheWorker::_getTileDownloadList(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCGetTileDownloadListTask *task = static_cast<QGCGetTileDownloadListTask*>(mtask);
    QList<TileDownloadInfo> downloadList;

    if (!_database->getDownloadList(task->setID(), task->count(), downloadList)) {
        task->setError("Failed to get download list");
        return;
    }

    QQueue<QGCTile*> tiles;
    for (const TileDownloadInfo &info : downloadList) {
        QGCTile *tile = new QGCTile;
        tile->setHash(info.hash);
        tile->setType(UrlFactory::getProviderTypeFromQtMapId(info.type));
        tile->setX(info.x);
        tile->setY(info.y);
        tile->setZ(info.z);
        tiles.enqueue(tile);

        // Update state to downloading
        _database->updateDownloadState(task->setID(), info.hash,
            static_cast<int>(QGCTile::StateDownloading));
    }

    task->setTileListFetched(tiles);
}

void QGCCacheWorker::_updateTileDownloadState(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCUpdateTileDownloadStateTask *task = static_cast<QGCUpdateTileDownloadStateTask*>(mtask);

    if (!_database->updateDownloadState(task->setID(), task->hash(), static_cast<int>(task->state()))) {
        qCWarning(QGCTileCacheWorkerLog) << "Failed to update download state";
    }
}

void QGCCacheWorker::_pruneCache(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCPruneCacheTask *task = static_cast<QGCPruneCacheTask*>(mtask);
    qint64 pruned = 0;

    if (_database->pruneCache(_database->getDefaultTileSet(), task->amount(), pruned)) {
        task->setPruned();
    }
}

void QGCCacheWorker::_deleteTileSet(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCDeleteTileSetTask *task = static_cast<QGCDeleteTileSetTask*>(mtask);

    if (_database->deleteTileSet(task->setID())) {
        _updateTotals();
        task->setTileSetDeleted();
    }
}

void QGCCacheWorker::_renameTileSet(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCRenameTileSetTask *task = static_cast<QGCRenameTileSetTask*>(mtask);

    if (!_database->renameTileSet(task->setID(), task->newName())) {
        task->setError("Error renaming tile set");
    }
}

void QGCCacheWorker::_resetCacheDatabase(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCResetTask *task = static_cast<QGCResetTask*>(mtask);

    _valid = _database->reset();
    task->setResetCompleted();
}

void QGCCacheWorker::_importSets(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCImportTileTask *task = static_cast<QGCImportTileTask*>(mtask);

    if (task->replace()) {
        // For replace mode, we need to close and reopen the database
        _database->close();

        // Delete existing database
        (void) QFile::remove(_databasePath);

        // Copy import database to our path
        if (!QFile::copy(task->path(), _databasePath)) {
            task->setError("Failed to copy import database");
            return;
        }

        task->setProgress(25);

        // Reopen the database
        if (!_database->open(_databasePath)) {
            task->setError("Failed to reopen database after import");
            _failed = true;
            _valid = false;
            return;
        }

        task->setProgress(50);

        // Recreate schema if needed
        _valid = _database->createSchema(true);
        if (!_valid) {
            task->setError("Failed to initialize imported database");
            _failed = true;
            return;
        }

        task->setProgress(100);
    } else {
        // Merge mode - use the database's import functionality
        auto progressCallback = [task](int progress) {
            task->setProgress(progress);
        };

        if (!_database->importDatabase(task->path(), false, progressCallback)) {
            task->setError(_database->lastError());
            return;
        }
    }

    task->setImportCompleted();
}

void QGCCacheWorker::_exportSets(QGCMapTask *mtask)
{
    if (!_testTask(mtask)) {
        return;
    }

    QGCExportTileTask *task = static_cast<QGCExportTileTask*>(mtask);

    // Collect set IDs to export
    QList<quint64> setIDs;
    for (int i = 0; i < task->sets().count(); i++) {
        const QGCCachedTileSet *set = task->sets().at(i);
        setIDs.append(set->id());
    }

    // Use database's export functionality
    auto progressCallback = [task](int progress) {
        task->setProgress(progress);
    };

    if (!_database->exportDatabase(task->path(), setIDs, progressCallback)) {
        task->setError(_database->lastError());
        return;
    }

    task->setExportCompleted();
}
