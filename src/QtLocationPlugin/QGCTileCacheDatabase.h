/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <memory>

class QSqlDatabase;
class QGCCachedTileSet;
class QGCCacheTile;

struct TileInfo {
    quint64 tileID = 0;
    QString hash;
    QString format;
    QByteArray tile;
    qint64 size = 0;
    QString type;
    qint64 date = 0;
};

struct TileSetInfo {
    quint64 setID = 0;
    QString name;
    QString typeStr;
    double topleftLat = 0.0;
    double topleftLon = 0.0;
    double bottomRightLat = 0.0;
    double bottomRightLon = 0.0;
    int minZoom = 3;
    int maxZoom = 3;
    int type = -1;
    quint32 numTiles = 0;
    bool defaultSet = false;
    qint64 date = 0;
};

struct TileDownloadInfo {
    QString hash;
    int type = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    int state = 0;
};

struct DatabaseTotals {
    quint32 totalCount = 0;
    quint64 totalSize = 0;
    quint32 defaultCount = 0;
    quint64 defaultSize = 0;
};

class QGCTileCacheDatabase : public QObject
{
    Q_OBJECT

public:
    explicit QGCTileCacheDatabase(QObject *parent = nullptr);
    ~QGCTileCacheDatabase();

    bool open(const QString &path);
    void close();
    bool isOpen() const;

    // Database creation and management
    bool createSchema(bool createDefault = true);
    bool reset();
    QString lastError() const;

    // Tile operations
    bool saveTile(const TileInfo &tile, quint64 setID);
    bool getTile(const QString &hash, TileInfo &tile);
    quint64 findTileID(const QString &hash);
    bool deleteTilesMatchingBytes(const QByteArray &bytes);

    // Tile set operations
    bool createTileSet(const TileSetInfo &info, quint64 &setID);
    bool getTileSets(QList<TileSetInfo> &sets);
    bool getTileSet(quint64 setID, TileSetInfo &info);
    bool updateTileSet(quint64 setID, const TileSetInfo &info);
    bool deleteTileSet(quint64 setID);
    bool renameTileSet(quint64 setID, const QString &newName);
    bool findTileSetID(const QString &name, quint64 &setID);
    quint64 getDefaultTileSet();

    // Set tile associations
    bool addTileToSet(quint64 tileID, quint64 setID);
    bool getTilesForSet(quint64 setID, QList<quint64> &tileIDs);

    // Download queue operations
    bool addToDownloadQueue(quint64 setID, const TileDownloadInfo &info);
    bool getDownloadList(quint64 setID, int limit, QList<TileDownloadInfo> &tiles);
    bool updateDownloadState(quint64 setID, const QString &hash, int state);
    bool clearDownloadQueue(quint64 setID);

    // Statistics and maintenance
    bool getTotals(DatabaseTotals &totals);
    bool getSetStatistics(quint64 setID, quint32 &savedCount, quint64 &savedSize,
                         quint32 &uniqueCount, quint64 &uniqueSize);
    bool pruneCache(quint64 setID, qint64 amount, qint64 &pruned);

    // Import/Export
    bool exportDatabase(const QString &targetPath, const QList<quint64> &setIDs,
                       std::function<void(int)> progressCallback = nullptr);
    bool importDatabase(const QString &sourcePath, bool replace,
                       std::function<void(int)> progressCallback = nullptr);

    // Transaction support
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    bool _executeQuery(const QString &query);
    bool _createTables();

    std::unique_ptr<QSqlDatabase> _db;
    QString _lastError;
    quint64 _defaultSetCache = UINT64_MAX;

    static constexpr const char* kConnectionName = "QGCTileCacheDB";
};
