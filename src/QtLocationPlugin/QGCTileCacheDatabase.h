/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtSql/QSqlDatabase>
#include <memory>

class QGCCacheTile;
class QGCCachedTileSet;
class QGCTile;

// Task forward decls (from QGCMapTasks.h)
class QGCSaveTileTask;
class QGCFetchTileTask;
class QGCFetchTileSetTask;
class QGCCreateTileSetTask;
class QGCGetTileDownloadListTask;
class QGCUpdateTileDownloadStateTask;
class QGCPruneCacheTask;
class QGCDeleteTileSetTask;
class QGCRenameTileSetTask;
class QGCResetTask;
class QGCImportTileTask;
class QGCExportTileTask;

/*!
 * \brief Thread-safe wrapper around the map tile cache SQLite DB.
 *
 * Replace QGCCacheWorker::_xxx DB helpers with calls here.
 */
class QGCTileCacheDatabase
{
public:
    explicit QGCTileCacheDatabase(QString dbPath,
                                  const QString& connectionName = QStringLiteral("QGCCacheDB"));
    ~QGCTileCacheDatabase();

    // Lifecycle
    bool open();     ///< open (and create schema if needed)
    void close();    ///< drop the connection

    bool isValid()   const { return _valid;  }
    bool hasFailed() const { return _failed; }
    QString path()   const { return _dbPath; }

    // One-offs
    void     deleteBingNoTileTiles();
    bool     findTileSetID(const QString& name, quint64& setID);
    quint64  getDefaultTileSet();

    // CRUD (used by worker tasks)
    void saveTile                (QGCSaveTileTask*      task);
    void getTile                 (QGCFetchTileTask*     task);
    void getTileSets             (QGCFetchTileSetTask*  task);
    void createTileSet           (QGCCreateTileSetTask* task);
    void getTileDownloadList     (QGCGetTileDownloadListTask* task);
    void updateTileDownloadState (QGCUpdateTileDownloadStateTask* task);
    void pruneCache              (QGCPruneCacheTask*    task);
    void deleteTileSet           (qulonglong id);
    void deleteTileSet           (QGCDeleteTileSetTask* task);
    void renameTileSet           (QGCRenameTileSetTask* task);
    void resetDatabase           (QGCResetTask*         task);
    void importSets              (QGCImportTileTask*    task);
    void exportSets              (QGCExportTileTask*    task);

    // Totals for UI
    void queryTotals(quint32& totalCnt,
                     quint64& totalSz,
                     quint32& defaultCnt,
                     quint64& defaultSz);

    // Create schema on an *external* connection (used by export)
    static bool createSchema(QSqlDatabase& db, bool createDefault = true);

private:
    // Internal helpers
    bool    _createSelfSchema(bool createDefault = true);
    void    _updateTotals();
    void    _updateSetTotals(QGCCachedTileSet* set);
    quint64 _findTile(const QString& hash);

    // Data
    QString                       _dbPath;
    QString                       _connectionName;
    std::unique_ptr<QSqlDatabase> _db;
    QMutex                        _mutex;

    bool    _valid  {false};
    bool    _failed {false};

    // Cached aggregates
    quint32 _totalCount   {0};
    quint64 _totalSize    {0};
    quint32 _defaultCount {0};
    quint64 _defaultSize  {0};
    quint64 _defaultSet   {UINT64_MAX};
};
