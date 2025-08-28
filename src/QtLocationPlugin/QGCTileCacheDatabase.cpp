/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCTileCacheDatabase.h"
#include "QGCCachedTileSet.h"
#include "QGCLoggingCategory.h"

#include <QtCore/QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

QGC_LOGGING_CATEGORY(QGCTileCacheDatabaseLog, "qgc.qtlocationplugin.qgctilecachedatabase")

QGCTileCacheDatabase::QGCTileCacheDatabase(QObject *parent)
    : QObject(parent)
{
    qCDebug(QGCTileCacheDatabaseLog) << this;
}

QGCTileCacheDatabase::~QGCTileCacheDatabase()
{
    close();

    qCDebug(QGCTileCacheDatabaseLog) << this;
}

bool QGCTileCacheDatabase::open(const QString &path)
{
    close();

    _db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", kConnectionName));
    _db->setDatabaseName(path);
    _db->setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    if (!_db->open()) {
        _lastError = _db->lastError().text();
        qCWarning(QGCTileCacheDatabaseLog) << "Failed to open database:" << _lastError;
        return false;
    }

    return true;
}

void QGCTileCacheDatabase::close()
{
    if (_db) {
        _db->close();
        _db.reset();
        QSqlDatabase::removeDatabase(kConnectionName);
    }
    _defaultSetCache = UINT64_MAX;
}

bool QGCTileCacheDatabase::isOpen() const
{
    return _db && _db->isOpen();
}

bool QGCTileCacheDatabase::createSchema(bool createDefault)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);

    // Create Tiles table
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS Tiles ("
        "tileID INTEGER PRIMARY KEY NOT NULL, "
        "hash TEXT NOT NULL UNIQUE, "
        "format TEXT NOT NULL, "
        "tile BLOB NULL, "
        "size INTEGER, "
        "type INTEGER, "
        "date INTEGER DEFAULT 0)"))
    {
        _lastError = query.lastError().text();
        qCWarning(QGCTileCacheDatabaseLog) << "Failed to create Tiles table:" << _lastError;
        return false;
    }

    (void) query.exec("CREATE INDEX IF NOT EXISTS hash ON Tiles (hash, size, type)");

    // Create TileSets table
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS TileSets ("
        "setID INTEGER PRIMARY KEY NOT NULL, "
        "name TEXT NOT NULL UNIQUE, "
        "typeStr TEXT, "
        "topleftLat REAL DEFAULT 0.0, "
        "topleftLon REAL DEFAULT 0.0, "
        "bottomRightLat REAL DEFAULT 0.0, "
        "bottomRightLon REAL DEFAULT 0.0, "
        "minZoom INTEGER DEFAULT 3, "
        "maxZoom INTEGER DEFAULT 3, "
        "type INTEGER DEFAULT -1, "
        "numTiles INTEGER DEFAULT 0, "
        "defaultSet INTEGER DEFAULT 0, "
        "date INTEGER DEFAULT 0)"))
    {
        _lastError = query.lastError().text();
        qCWarning(QGCTileCacheDatabaseLog) << "Failed to create TileSets table:" << _lastError;
        return false;
    }

    // Create SetTiles table
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS SetTiles ("
        "setID INTEGER, "
        "tileID INTEGER)"))
    {
        _lastError = query.lastError().text();
        qCWarning(QGCTileCacheDatabaseLog) << "Failed to create SetTiles table:" << _lastError;
        return false;
    }

    // Create TilesDownload table
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS TilesDownload ("
        "setID INTEGER, "
        "hash TEXT NOT NULL UNIQUE, "
        "type INTEGER, "
        "x INTEGER, "
        "y INTEGER, "
        "z INTEGER, "
        "state INTEGER DEFAULT 0)"))
    {
        _lastError = query.lastError().text();
        qCWarning(QGCTileCacheDatabaseLog) << "Failed to create TilesDownload table:" << _lastError;
        return false;
    }

    // Create default tile set if requested
    if (createDefault) {
        const QString s = QStringLiteral("SELECT name FROM TileSets WHERE name = 'Default Tile Set'");
        if (query.exec(s)) {
            if (!query.next()) {
                query.prepare("INSERT INTO TileSets(name, defaultSet, date) VALUES(?, ?, ?)");
                query.addBindValue("Default Tile Set");
                query.addBindValue(1);
                query.addBindValue(QDateTime::currentSecsSinceEpoch());
                if (!query.exec()) {
                    _lastError = query.lastError().text();
                    qCWarning(QGCTileCacheDatabaseLog) << "Failed to create default tile set:" << _lastError;
                    return false;
                }
            }
        }
    }

    return true;
}

bool QGCTileCacheDatabase::reset()
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    (void) query.exec("DROP TABLE IF EXISTS Tiles");
    (void) query.exec("DROP TABLE IF EXISTS TileSets");
    (void) query.exec("DROP TABLE IF EXISTS SetTiles");
    (void) query.exec("DROP TABLE IF EXISTS TilesDownload");

    return createSchema(true);
}

QString QGCTileCacheDatabase::lastError() const
{
    return _lastError;
}

bool QGCTileCacheDatabase::saveTile(const TileInfo &tile, quint64 setID)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    query.prepare("INSERT INTO Tiles(hash, format, tile, size, type, date) VALUES(?, ?, ?, ?, ?, ?)");
    query.addBindValue(tile.hash);
    query.addBindValue(tile.format);
    query.addBindValue(tile.tile);
    query.addBindValue(tile.size > 0 ? tile.size : tile.tile.size());
    query.addBindValue(tile.type);
    query.addBindValue(tile.date > 0 ? tile.date : QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        // Tile might already exist - not necessarily an error
        return false;
    }

    const quint64 tileID = query.lastInsertId().toULongLong();
    return addTileToSet(tileID, setID);
}

bool QGCTileCacheDatabase::getTile(const QString &hash, TileInfo &tile)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT * FROM Tiles WHERE hash = '%1'").arg(hash);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    if (!query.next()) {
        return false;
    }

    tile.tileID = query.value("tileID").toULongLong();
    tile.hash = query.value("hash").toString();
    tile.format = query.value("format").toString();
    tile.tile = query.value("tile").toByteArray();
    tile.size = query.value("size").toLongLong();
    tile.type = query.value("type").toString();
    tile.date = query.value("date").toLongLong();

    return true;
}

quint64 QGCTileCacheDatabase::findTileID(const QString &hash)
{
    if (!isOpen()) {
        return 0;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT tileID FROM Tiles WHERE hash = '%1'").arg(hash);

    if (query.exec(s) && query.next()) {
        return query.value(0).toULongLong();
    }

    return 0;
}

bool QGCTileCacheDatabase::deleteTilesMatchingBytes(const QByteArray &bytes)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    QList<quint64> idsToDelete;

    const QString s = QStringLiteral("SELECT tileID, tile FROM Tiles WHERE LENGTH(tile) = %1").arg(bytes.length());
    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    while (query.next()) {
        if (query.value(1).toByteArray() == bytes) {
            idsToDelete.append(query.value(0).toULongLong());
        }
    }

    for (const quint64 tileId : idsToDelete) {
        const QString deleteQuery = QStringLiteral("DELETE FROM Tiles WHERE tileID = %1").arg(tileId);
        if (!query.exec(deleteQuery)) {
            _lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

bool QGCTileCacheDatabase::createTileSet(const TileSetInfo &info, quint64 &setID)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    query.prepare("INSERT INTO TileSets("
        "name, typeStr, topleftLat, topleftLon, bottomRightLat, bottomRightLon, "
        "minZoom, maxZoom, type, numTiles, defaultSet, date"
        ") VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(info.name);
    query.addBindValue(info.typeStr);
    query.addBindValue(info.topleftLat);
    query.addBindValue(info.topleftLon);
    query.addBindValue(info.bottomRightLat);
    query.addBindValue(info.bottomRightLon);
    query.addBindValue(info.minZoom);
    query.addBindValue(info.maxZoom);
    query.addBindValue(info.type);
    query.addBindValue(info.numTiles);
    query.addBindValue(info.defaultSet ? 1 : 0);
    query.addBindValue(info.date > 0 ? info.date : QDateTime::currentSecsSinceEpoch());

    if (!query.exec()) {
        _lastError = query.lastError().text();
        return false;
    }

    setID = query.lastInsertId().toULongLong();
    return true;
}

bool QGCTileCacheDatabase::getTileSets(QList<TileSetInfo> &sets)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT * FROM TileSets ORDER BY defaultSet DESC, name ASC");

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    while (query.next()) {
        TileSetInfo info;
        info.setID = query.value("setID").toULongLong();
        info.name = query.value("name").toString();
        info.typeStr = query.value("typeStr").toString();
        info.topleftLat = query.value("topleftLat").toDouble();
        info.topleftLon = query.value("topleftLon").toDouble();
        info.bottomRightLat = query.value("bottomRightLat").toDouble();
        info.bottomRightLon = query.value("bottomRightLon").toDouble();
        info.minZoom = query.value("minZoom").toInt();
        info.maxZoom = query.value("maxZoom").toInt();
        info.type = query.value("type").toInt();
        info.numTiles = query.value("numTiles").toUInt();
        info.defaultSet = query.value("defaultSet").toInt() != 0;
        info.date = query.value("date").toLongLong();
        sets.append(info);
    }

    return true;
}

bool QGCTileCacheDatabase::deleteTileSet(quint64 setID)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);

    // Delete tiles unique to this set
    QString s = QStringLiteral(
        "DELETE FROM Tiles WHERE tileID IN ("
        "SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID = B.tileID "
        "WHERE B.setID = %1 GROUP BY A.tileID HAVING COUNT(A.tileID) = 1)").arg(setID);
    (void) query.exec(s);

    s = QStringLiteral("DELETE FROM TilesDownload WHERE setID = %1").arg(setID);
    (void) query.exec(s);

    s = QStringLiteral("DELETE FROM TileSets WHERE setID = %1").arg(setID);
    (void) query.exec(s);

    s = QStringLiteral("DELETE FROM SetTiles WHERE setID = %1").arg(setID);
    (void) query.exec(s);

    return true;
}

bool QGCTileCacheDatabase::renameTileSet(quint64 setID, const QString &newName)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("UPDATE TileSets SET name = '%1' WHERE setID = %2").arg(newName).arg(setID);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool QGCTileCacheDatabase::findTileSetID(const QString &name, quint64 &setID)
{
    if (!isOpen()) {
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT setID FROM TileSets WHERE name = '%1'").arg(name);

    if (query.exec(s) && query.next()) {
        setID = query.value(0).toULongLong();
        return true;
    }

    return false;
}

quint64 QGCTileCacheDatabase::getDefaultTileSet()
{
    if (_defaultSetCache != UINT64_MAX) {
        return _defaultSetCache;
    }

    if (!isOpen()) {
        return 1;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT setID FROM TileSets WHERE defaultSet = 1");

    if (query.exec(s) && query.next()) {
        _defaultSetCache = query.value(0).toULongLong();
        return _defaultSetCache;
    }

    return 1;
}

bool QGCTileCacheDatabase::addTileToSet(quint64 tileID, quint64 setID)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("INSERT INTO SetTiles(tileID, setID) VALUES(%1, %2)").arg(tileID).arg(setID);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool QGCTileCacheDatabase::addToDownloadQueue(quint64 setID, const TileDownloadInfo &info)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    query.prepare("INSERT OR IGNORE INTO TilesDownload(setID, hash, type, x, y, z, state) VALUES(?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(setID);
    query.addBindValue(info.hash);
    query.addBindValue(info.type);
    query.addBindValue(info.x);
    query.addBindValue(info.y);
    query.addBindValue(info.z);
    query.addBindValue(info.state);

    if (!query.exec()) {
        _lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool QGCTileCacheDatabase::getDownloadList(quint64 setID, int limit, QList<TileDownloadInfo> &tiles)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT hash, type, x, y, z FROM TilesDownload WHERE setID = %1 AND state = 0 LIMIT %2")
        .arg(setID).arg(limit);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    while (query.next()) {
        TileDownloadInfo info;
        info.hash = query.value("hash").toString();
        info.type = query.value("type").toInt();
        info.x = query.value("x").toInt();
        info.y = query.value("y").toInt();
        info.z = query.value("z").toInt();
        tiles.append(info);
    }

    return true;
}

bool QGCTileCacheDatabase::updateDownloadState(quint64 setID, const QString &hash, int state)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    QString s;

    if (state == 2) { // StateComplete
        s = QStringLiteral("DELETE FROM TilesDownload WHERE setID = %1 AND hash = '%2'").arg(setID).arg(hash);
    } else if (hash == "*") {
        s = QStringLiteral("UPDATE TilesDownload SET state = %1 WHERE setID = %2").arg(state).arg(setID);
    } else {
        s = QStringLiteral("UPDATE TilesDownload SET state = %1 WHERE setID = %2 AND hash = '%3'")
            .arg(state).arg(setID).arg(hash);
    }

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool QGCTileCacheDatabase::getTotals(DatabaseTotals &totals)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);

    // Get total counts
    QString s = QStringLiteral("SELECT COUNT(size), SUM(size) FROM Tiles");
    if (query.exec(s) && query.next()) {
        totals.totalCount = query.value(0).toUInt();
        totals.totalSize = query.value(1).toULongLong();
    }

    // Get default set unique counts
    const quint64 defaultSet = getDefaultTileSet();
    s = QStringLiteral(
        "SELECT COUNT(size), SUM(size) FROM Tiles WHERE tileID IN ("
        "SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID = B.tileID "
        "WHERE B.setID = %1 GROUP BY A.tileID HAVING COUNT(A.tileID) = 1)").arg(defaultSet);

    if (query.exec(s) && query.next()) {
        totals.defaultCount = query.value(0).toUInt();
        totals.defaultSize = query.value(1).toULongLong();
    }

    return true;
}

bool QGCTileCacheDatabase::getSetStatistics(quint64 setID, quint32 &savedCount, quint64 &savedSize,
                                            quint32 &uniqueCount, quint64 &uniqueSize)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);

    // Get saved tiles count and size
    QString s = QStringLiteral(
        "SELECT COUNT(size), SUM(size) FROM Tiles A INNER JOIN SetTiles B ON A.tileID = B.tileID WHERE B.setID = %1")
        .arg(setID);

    if (query.exec(s) && query.next()) {
        savedCount = query.value(0).toUInt();
        savedSize = query.value(1).toULongLong();
    }

    // Get unique tiles count and size
    s = QStringLiteral(
        "SELECT COUNT(size), SUM(size) FROM Tiles WHERE tileID IN ("
        "SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID = B.tileID "
        "WHERE B.setID = %1 GROUP BY A.tileID HAVING COUNT(A.tileID) = 1)").arg(setID);

    if (query.exec(s) && query.next()) {
        uniqueCount = query.value(0).toUInt();
        uniqueSize = query.value(1).toULongLong();
    }

    return true;
}

bool QGCTileCacheDatabase::pruneCache(quint64 setID, qint64 amount, qint64 &pruned)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);

    // Select oldest tiles in set
    const QString s = QStringLiteral(
        "SELECT tileID, size FROM Tiles WHERE tileID IN ("
        "SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID = B.tileID "
        "WHERE B.setID = %1 GROUP BY A.tileID HAVING COUNT(A.tileID) = 1) "
        "ORDER BY date ASC LIMIT 128").arg(setID);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    QList<quint64> tilesToDelete;
    pruned = 0;

    while (query.next() && amount > 0) {
        tilesToDelete << query.value(0).toULongLong();
        const qint64 size = query.value(1).toLongLong();
        amount -= size;
        pruned += size;
    }

    // Delete selected tiles
    for (const quint64 tileID : tilesToDelete) {
        const QString deleteQuery = QStringLiteral("DELETE FROM Tiles WHERE tileID = %1").arg(tileID);
        if (!query.exec(deleteQuery)) {
            _lastError = query.lastError().text();
            return false;
        }
    }

    return true;
}

bool QGCTileCacheDatabase::beginTransaction()
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }
    return _db->transaction();
}

bool QGCTileCacheDatabase::commitTransaction()
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }
    return _db->commit();
}

bool QGCTileCacheDatabase::rollbackTransaction()
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }
    return _db->rollback();
}

bool QGCTileCacheDatabase::exportDatabase(const QString &targetPath, const QList<quint64> &setIDs,
                                         std::function<void(int)> progressCallback)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    // Delete target if it exists
    (void) QFile::remove(targetPath);

    // Create exported database
    QSqlDatabase dbExport = QSqlDatabase::addDatabase("QSQLITE", "QGCTileExportSession");
    dbExport.setDatabaseName(targetPath);
    dbExport.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    if (!dbExport.open()) {
        _lastError = "Failed to create export database";
        QSqlDatabase::removeDatabase("QGCTileExportSession");
        return false;
    }

    // Create schema in export database
    QGCTileCacheDatabase exportDb;
    exportDb._db = std::make_unique<QSqlDatabase>(dbExport);
    if (!exportDb.createSchema(false)) {
        _lastError = "Failed to create export database schema";
        QSqlDatabase::removeDatabase("QGCTileExportSession");
        return false;
    }

    // Calculate total tiles for progress
    quint64 totalTiles = 0;
    quint64 currentProgress = 0;

    for (quint64 setID : setIDs) {
        TileSetInfo setInfo;
        if (getTileSet(setID, setInfo)) {
            if (setInfo.defaultSet) {
                totalTiles += setInfo.numTiles;
            } else {
                // Get unique tile count for non-default sets
                quint32 savedCount, uniqueCount;
                quint64 savedSize, uniqueSize;
                if (getSetStatistics(setID, savedCount, savedSize, uniqueCount, uniqueSize)) {
                    totalTiles += uniqueCount;
                }
            }
        }
    }

    if (totalTiles == 0) {
        totalTiles = 1;
    }

    // Export each tile set
    for (quint64 setID : setIDs) {
        TileSetInfo setInfo;
        if (!getTileSet(setID, setInfo)) {
            continue;
        }

        // Create tile set in export database
        quint64 exportSetID;
        if (!exportDb.createTileSet(setInfo, exportSetID)) {
            qCWarning(QGCTileCacheDatabaseLog) << "Failed to create tile set in export database";
            continue;
        }

        // Get all tiles for this set
        QList<quint64> tileIDs;
        if (!getTilesForSet(setID, tileIDs)) {
            continue;
        }

        // Begin transaction for bulk insert
        (void) exportDb.beginTransaction();

        for (quint64 tileID : tileIDs) {
            // Get tile data
            QSqlQuery query(*_db);
            const QString s = QStringLiteral("SELECT * FROM Tiles WHERE tileID = %1").arg(tileID);

            if (!query.exec(s) || !query.next()) {
                continue;
            }

            TileInfo tile;
            tile.hash = query.value("hash").toString();
            tile.format = query.value("format").toString();
            tile.tile = query.value("tile").toByteArray();
            tile.size = query.value("size").toLongLong();
            tile.type = query.value("type").toString();
            tile.date = query.value("date").toLongLong();

            // Save tile to export database
            exportDb.saveTile(tile, exportSetID);

            currentProgress++;
            if (progressCallback) {
                int progress = static_cast<int>((static_cast<double>(currentProgress) / static_cast<double>(totalTiles)) * 100.0);
                progressCallback(progress);
            }
        }

        (void) exportDb.commitTransaction();
    }

    exportDb._db->close();
    exportDb._db.reset();
    QSqlDatabase::removeDatabase("QGCTileExportSession");

    return true;
}

bool QGCTileCacheDatabase::importDatabase(const QString &sourcePath, bool replace,
                                         std::function<void(int)> progressCallback)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    if (replace) {
        // Simple replacement - close current, copy new, reopen
        QString currentPath = _db->databaseName();
        close();

        (void) QFile::remove(currentPath);
        if (!QFile::copy(sourcePath, currentPath)) {
            _lastError = "Failed to copy import database";
            return false;
        }

        if (progressCallback) {
            progressCallback(50);
        }

        bool result = open(currentPath);
        if (progressCallback) {
            progressCallback(100);
        }

        return result;
    }

    // Merge import - open source database
    QSqlDatabase dbImport = QSqlDatabase::addDatabase("QSQLITE", "QGCTileImportSession");
    dbImport.setDatabaseName(sourcePath);
    dbImport.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    if (!dbImport.open()) {
        _lastError = "Failed to open import database";
        QSqlDatabase::removeDatabase("QGCTileImportSession");
        return false;
    }

    // Calculate total tiles for progress
    QSqlQuery countQuery(dbImport);
    quint64 totalTiles = 0;
    if (countQuery.exec("SELECT COUNT(tileID) FROM Tiles") && countQuery.next()) {
        totalTiles = countQuery.value(0).toULongLong();
    }

    if (totalTiles == 0) {
        _lastError = "No tiles in import database";
        dbImport.close();
        QSqlDatabase::removeDatabase("QGCTileImportSession");
        return false;
    }

    quint64 currentProgress = 0;
    int lastReportedProgress = -1;

    // Import tile sets
    QSqlQuery importQuery(dbImport);
    if (!importQuery.exec("SELECT * FROM TileSets ORDER BY defaultSet DESC, name ASC")) {
        _lastError = "Failed to read tile sets from import database";
        dbImport.close();
        QSqlDatabase::removeDatabase("QGCTileImportSession");
        return false;
    }

    while (importQuery.next()) {
        TileSetInfo setInfo;
        setInfo.setID = importQuery.value("setID").toULongLong();
        setInfo.name = importQuery.value("name").toString();
        setInfo.typeStr = importQuery.value("typeStr").toString();
        setInfo.topleftLat = importQuery.value("topleftLat").toDouble();
        setInfo.topleftLon = importQuery.value("topleftLon").toDouble();
        setInfo.bottomRightLat = importQuery.value("bottomRightLat").toDouble();
        setInfo.bottomRightLon = importQuery.value("bottomRightLon").toDouble();
        setInfo.minZoom = importQuery.value("minZoom").toInt();
        setInfo.maxZoom = importQuery.value("maxZoom").toInt();
        setInfo.type = importQuery.value("type").toInt();
        setInfo.numTiles = importQuery.value("numTiles").toUInt();
        setInfo.defaultSet = importQuery.value("defaultSet").toInt() != 0;
        setInfo.date = importQuery.value("date").toLongLong();

        const quint64 importSetID = setInfo.setID;
        quint64 targetSetID = getDefaultTileSet();

        // For non-default sets, check if name exists and make unique if needed
        if (!setInfo.defaultSet) {
            quint64 existingID;
            if (findTileSetID(setInfo.name, existingID)) {
                int counter = 1;
                QString baseName = setInfo.name;
                do {
                    setInfo.name = QString("%1 %02d").arg(baseName).arg(++counter);
                } while (findTileSetID(setInfo.name, existingID) && counter < 100);
            }

            // Create the tile set
            if (!createTileSet(setInfo, targetSetID)) {
                qCWarning(QGCTileCacheDatabaseLog) << "Failed to create tile set during import:" << setInfo.name;
                continue;
            }
        }

        // Import tiles for this set
        QSqlQuery tileQuery(dbImport);
        QString s = QStringLiteral(
            "SELECT * FROM Tiles WHERE tileID IN ("
            "SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID = B.tileID "
            "WHERE B.setID = %1 GROUP BY A.tileID HAVING COUNT(A.tileID) = 1)").arg(importSetID);

        if (!tileQuery.exec(s)) {
            continue;
        }

        quint64 tilesImported = 0;
        beginTransaction();

        while (tileQuery.next()) {
            TileInfo tile;
            tile.hash = tileQuery.value("hash").toString();
            tile.format = tileQuery.value("format").toString();
            tile.tile = tileQuery.value("tile").toByteArray();
            tile.size = tileQuery.value("size").toLongLong();
            tile.type = tileQuery.value("type").toString();
            tile.date = tileQuery.value("date").toLongLong();

            if (saveTile(tile, targetSetID)) {
                tilesImported++;
            }

            currentProgress++;
            if (progressCallback && totalTiles > 0) {
                int progress = static_cast<int>((static_cast<double>(currentProgress) / static_cast<double>(totalTiles)) * 100.0);
                if (progress != lastReportedProgress) {
                    lastReportedProgress = progress;
                    progressCallback(progress);
                }
            }
        }

        commitTransaction();

        // Update tile count if tiles were added
        if (tilesImported > 0 && !setInfo.defaultSet) {
            QSqlQuery updateQuery(*_db);
            quint32 savedCount, uniqueCount;
            quint64 savedSize, uniqueSize;
            if (getSetStatistics(targetSetID, savedCount, savedSize, uniqueCount, uniqueSize)) {
                const QString updateSql = QStringLiteral("UPDATE TileSets SET numTiles = %1 WHERE setID = %2")
                    .arg(savedCount).arg(targetSetID);
                (void) updateQuery.exec(updateSql);
            }
        }

        // Remove empty imported set
        if (tilesImported == 0 && !setInfo.defaultSet) {
            deleteTileSet(targetSetID);
        }
    }

    dbImport.close();
    QSqlDatabase::removeDatabase("QGCTileImportSession");

    return true;
}

bool QGCTileCacheDatabase::getTileSet(quint64 setID, TileSetInfo &info)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT * FROM TileSets WHERE setID = %1").arg(setID);

    if (!query.exec(s) || !query.next()) {
        return false;
    }

    info.setID = query.value("setID").toULongLong();
    info.name = query.value("name").toString();
    info.typeStr = query.value("typeStr").toString();
    info.topleftLat = query.value("topleftLat").toDouble();
    info.topleftLon = query.value("topleftLon").toDouble();
    info.bottomRightLat = query.value("bottomRightLat").toDouble();
    info.bottomRightLon = query.value("bottomRightLon").toDouble();
    info.minZoom = query.value("minZoom").toInt();
    info.maxZoom = query.value("maxZoom").toInt();
    info.type = query.value("type").toInt();
    info.numTiles = query.value("numTiles").toUInt();
    info.defaultSet = query.value("defaultSet").toInt() != 0;
    info.date = query.value("date").toLongLong();

    return true;
}

bool QGCTileCacheDatabase::getTilesForSet(quint64 setID, QList<quint64> &tileIDs)
{
    if (!isOpen()) {
        _lastError = "Database not open";
        return false;
    }

    QSqlQuery query(*_db);
    const QString s = QStringLiteral("SELECT tileID FROM SetTiles WHERE setID = %1").arg(setID);

    if (!query.exec(s)) {
        _lastError = query.lastError().text();
        return false;
    }

    while (query.next()) {
        tileIDs.append(query.value(0).toULongLong());
    }

    return true;
}
