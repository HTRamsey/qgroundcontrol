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
#include "QGCMapTasks.h"          // task types + QGCTile/QGCCacheTile
#include "QGCMapUrlEngine.h"      // UrlFactory
#include "QGCLoggingCategory.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

QGC_LOGGING_CATEGORY(QGCDBLog, "qgc.qtlocationplugin.qgctilecachedb")

static const char* kExportSession = "QGCCacheDBExport";

// ---------------------------------------------------------------------
// ctor / dtor

QGCTileCacheDatabase::QGCTileCacheDatabase(QString dbPath,
                                           const QString& connectionName)
    : _dbPath(std::move(dbPath))
    , _connectionName(connectionName)
{
}

QGCTileCacheDatabase::~QGCTileCacheDatabase()
{
    close();
}

// ---------------------------------------------------------------------
// lifecycle

bool QGCTileCacheDatabase::open()
{
    QMutexLocker lock(&_mutex);

    _failed = false;
    _db.reset(new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", _connectionName)));
    _db->setDatabaseName(_dbPath);
    _db->setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    _valid = _db->open();
    if (!_valid) {
        qCCritical(QGCDBLog) << "Unable to open DB:" << _db->lastError().text();
        _failed = true;
        return false;
    }

    _valid = _createSelfSchema();       // ensure schema on this connection
    if (!_valid) _failed = true;
    return _valid;
}

void QGCTileCacheDatabase::close()
{
    QMutexLocker lock(&_mutex);
    _db.reset();                        // destroy handles first
    QSqlDatabase::removeDatabase(_connectionName);
}

// ---------------------------------------------------------------------
// helper utilities

void QGCTileCacheDatabase::deleteBingNoTileTiles()
{
    QMutexLocker lock(&_mutex);

    static const QString alreadyDoneKey = QStringLiteral("_deleteBingNoTileTilesDone");

    QSettings settings;
    if (settings.value(alreadyDoneKey, false).toBool())
        return;

    settings.setValue(alreadyDoneKey, true);

    QFile file(QStringLiteral(":/res/BingNoTileBytes.dat"));
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(QGCDBLog) << "Open" << file.fileName() << "failed:" << file.errorString();
        return;
    }
    const QByteArray noTileBytes = file.readAll();
    file.close();

    QSqlQuery q(*_db);
    if (!q.exec(QStringLiteral("SELECT tileID, tile, hash FROM Tiles WHERE LENGTH(tile)=%1")
                    .arg(noTileBytes.length())))
    {
        qCWarning(QGCDBLog) << "Query failed";
        return;
    }

    QList<quint64> ids;
    while (q.next()) {
        const QByteArray bytes = q.value(1).toByteArray();
        if (bytes == noTileBytes)
            ids.append(q.value(0).toULongLong());
    }

    for (quint64 id : std::as_const(ids)) {
        QSqlQuery(*_db).exec(QStringLiteral("DELETE FROM Tiles WHERE tileID=%1").arg(id));
    }
}

bool QGCTileCacheDatabase::findTileSetID(const QString& name, quint64& setID)
{
    QMutexLocker lock(&_mutex);
    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral("SELECT setID FROM TileSets WHERE name=\"%1\"").arg(name)) && q.next()) {
        setID = q.value(0).toULongLong();
        return true;
    }
    return false;
}

quint64 QGCTileCacheDatabase::getDefaultTileSet()
{
    if (_defaultSet != UINT64_MAX)
        return _defaultSet;

    QMutexLocker lock(&_mutex);
    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral("SELECT setID FROM TileSets WHERE defaultSet=1")) && q.next())
        _defaultSet = q.value(0).toULongLong();
    else
        _defaultSet = 1ULL;

    return _defaultSet;
}

// ---------------------------------------------------------------------
// main CRUD ops (port of original worker helpers)

void QGCTileCacheDatabase::saveTile(QGCSaveTileTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QSqlQuery q(*_db);
    q.prepare(QStringLiteral(
        "INSERT INTO Tiles(hash,format,tile,size,type,date) VALUES(?,?,?,?,?,?)"));
    q.addBindValue(task->tile()->hash());
    q.addBindValue(task->tile()->format());
    q.addBindValue(task->tile()->img());
    q.addBindValue(task->tile()->img().size());
    q.addBindValue(task->tile()->type());
    q.addBindValue(QDateTime::currentSecsSinceEpoch());

    if (!q.exec()) {
        // duplicate; benign
        return;
    }

    const quint64 tileID = q.lastInsertId().toULongLong();
    const quint64 setID  = task->tile()->tileSet() == UINT64_MAX
                               ? getDefaultTileSet()
                               : task->tile()->tileSet();

    QSqlQuery(*_db).exec(QStringLiteral("INSERT INTO SetTiles(tileID,setID) VALUES(%1,%2)")
                             .arg(tileID).arg(setID));
}

void QGCTileCacheDatabase::getTile(QGCFetchTileTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral("SELECT tile,format,type FROM Tiles WHERE hash=\"%1\"").arg(task->hash()))
        && q.next())
    {
        const QByteArray img = q.value(0).toByteArray();
        const QString fmt    = q.value(1).toString();
        const QString type   = q.value(2).toString();
        auto* tile = new QGCCacheTile(task->hash(), img, fmt, type);
        task->setTileFetched(tile);
    } else {
        task->setError("Tile not in cache database");
    }
}

void QGCTileCacheDatabase::getTileSets(QGCFetchTileSetTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QSqlQuery q(*_db);
    if (!q.exec(QStringLiteral("SELECT * FROM TileSets ORDER BY defaultSet DESC, name ASC"))) {
        task->setError("No tile set in database");
        return;
    }

    while (q.next()) {
        const QString name = q.value("name").toString();
        auto* set = new QGCCachedTileSet(name);
        set->setId(q.value("setID").toULongLong());
        set->setMapTypeStr(q.value("typeStr").toString());
        set->setTopleftLat(q.value("topleftLat").toDouble());
        set->setTopleftLon(q.value("topleftLon").toDouble());
        set->setBottomRightLat(q.value("bottomRightLat").toDouble());
        set->setBottomRightLon(q.value("bottomRightLon").toDouble());
        set->setMinZoom(q.value("minZoom").toInt());
        set->setMaxZoom(q.value("maxZoom").toInt());
        set->setType(UrlFactory::getProviderTypeFromQtMapId(q.value("type").toInt()));
        set->setTotalTileCount(q.value("numTiles").toUInt());
        set->setDefaultSet(q.value("defaultSet").toInt() != 0);
        set->setCreationDate(QDateTime::fromSecsSinceEpoch(q.value("date").toUInt()));
        _updateSetTotals(set);
        (void) set->moveToThread(QCoreApplication::instance()->thread());
        task->setTileSetFetched(set);
    }
}

void QGCTileCacheDatabase::createTileSet(QGCCreateTileSetTask* task)
{
    if (!_valid) { task->setError("Error saving tile set"); return; }
    QMutexLocker lock(&_mutex);

    QSqlQuery q(*_db);
    q.prepare("INSERT INTO TileSets("
              "name,typeStr,topleftLat,topleftLon,bottomRightLat,bottomRightLon,"
              "minZoom,maxZoom,type,numTiles,date"
              ") VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(task->tileSet()->name());
    q.addBindValue(task->tileSet()->mapTypeStr());
    q.addBindValue(task->tileSet()->topleftLat());
    q.addBindValue(task->tileSet()->topleftLon());
    q.addBindValue(task->tileSet()->bottomRightLat());
    q.addBindValue(task->tileSet()->bottomRightLon());
    q.addBindValue(task->tileSet()->minZoom());
    q.addBindValue(task->tileSet()->maxZoom());
    q.addBindValue(UrlFactory::getQtMapIdFromProviderType(task->tileSet()->type()));
    q.addBindValue(task->tileSet()->totalTileCount());
    q.addBindValue(QDateTime::currentSecsSinceEpoch());

    if (!q.exec()) {
        task->setError("Error saving tile set");
        return;
    }

    const quint64 setID = q.lastInsertId().toULongLong();
    task->tileSet()->setId(setID);

    _db->transaction();
    for (int z = task->tileSet()->minZoom(); z <= task->tileSet()->maxZoom(); z++) {
        const QGCTileSet set = UrlFactory::getTileCount(
            z,
            task->tileSet()->topleftLon(), task->tileSet()->topleftLat(),
            task->tileSet()->bottomRightLon(), task->tileSet()->bottomRightLat(),
            task->tileSet()->type());

        const QString type = task->tileSet()->type();
        for (int x = set.tileX0; x <= set.tileX1; x++) {
            for (int y = set.tileY0; y <= set.tileY1; y++) {
                const QString hash = UrlFactory::getTileHash(type, x, y, z);
                const quint64 tileID = _findTile(hash);
                if (tileID == 0) {
                    q.prepare("INSERT OR IGNORE INTO TilesDownload(setID,hash,type,x,y,z,state)"
                              " VALUES(?,?,?,?,?,?,?)");
                    q.addBindValue(setID);
                    q.addBindValue(hash);
                    q.addBindValue(UrlFactory::getQtMapIdFromProviderType(type));
                    q.addBindValue(x);
                    q.addBindValue(y);
                    q.addBindValue(z);
                    q.addBindValue(0);
                    q.exec();
                } else {
                    QSqlQuery(*_db).exec(QStringLiteral(
                        "INSERT OR IGNORE INTO SetTiles(tileID,setID) VALUES(%1,%2)")
                        .arg(tileID).arg(setID));
                }
            }
        }
    }
    _db->commit();

    _updateSetTotals(task->tileSet());
    task->setTileSetSaved();
}

void QGCTileCacheDatabase::getTileDownloadList(QGCGetTileDownloadListTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QQueue<QGCTile*> tiles;
    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral(
            "SELECT hash,type,x,y,z FROM TilesDownload WHERE setID=%1 AND state=0 LIMIT %2")
            .arg(task->setID()).arg(task->count())))
    {
        while (q.next()) {
            auto* t = new QGCTile;
            t->setHash(q.value("hash").toString());
            t->setType(UrlFactory::getProviderTypeFromQtMapId(q.value("type").toInt()));
            t->setX(q.value("x").toInt());
            t->setY(q.value("y").toInt());
            t->setZ(q.value("z").toInt());
            tiles.enqueue(t);
        }

        for (auto* t : std::as_const(tiles)) {
            QSqlQuery(*_db).exec(QStringLiteral(
                "UPDATE TilesDownload SET state=%1 WHERE setID=%2 AND hash=\"%3\"")
                .arg(static_cast<int>(QGCTile::StateDownloading))
                .arg(task->setID())
                .arg(t->hash()));
        }
    }

    task->setTileListFetched(tiles);
}

void QGCTileCacheDatabase::updateTileDownloadState(QGCUpdateTileDownloadStateTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QString s;
    if (task->state() == QGCTile::StateComplete) {
        s = QStringLiteral("DELETE FROM TilesDownload WHERE setID=%1 AND hash=\"%2\"")
                .arg(task->setID()).arg(task->hash());
    } else if (task->hash() == "*") {
        s = QStringLiteral("UPDATE TilesDownload SET state=%1 WHERE setID=%2")
                .arg(static_cast<int>(task->state())).arg(task->setID());
    } else {
        s = QStringLiteral("UPDATE TilesDownload SET state=%1 WHERE setID=%2 AND hash=\"%3\"")
                .arg(static_cast<int>(task->state())).arg(task->setID()).arg(task->hash());
    }
    QSqlQuery(*_db).exec(s);
}

void QGCTileCacheDatabase::pruneCache(QGCPruneCacheTask* task)
{
    if (!_valid) return;
    QMutexLocker lock(&_mutex);

    QSqlQuery q(*_db);
    const QString s = QStringLiteral(
        "SELECT tileID,size,hash FROM Tiles WHERE tileID IN "
        "(SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID=B.tileID "
        " WHERE B.setID=%1 GROUP BY A.tileID HAVING COUNT(A.tileID)=1) "
        "ORDER BY date ASC LIMIT 128").arg(getDefaultTileSet());
    if (!q.exec(s)) {
        task->setPruned();
        return;
    }

    QList<quint64> ids;
    qint64 remaining = static_cast<qint64>(task->amount());
    while (q.next() && (remaining >= 0)) {
        ids << q.value(0).toULongLong();
        remaining -= q.value(1).toLongLong();
    }

    for (quint64 id : std::as_const(ids))
        QSqlQuery(*_db).exec(QStringLiteral("DELETE FROM Tiles WHERE tileID=%1").arg(id));

    task->setPruned();
}

void QGCTileCacheDatabase::deleteTileSet(qulonglong id)
{
    QMutexLocker lock(&_mutex);
    QSqlQuery q(*_db);
    q.exec(QStringLiteral(
        "DELETE FROM Tiles WHERE tileID IN (SELECT A.tileID FROM SetTiles A "
        "JOIN SetTiles B ON A.tileID=B.tileID WHERE B.setID=%1 "
        "GROUP BY A.tileID HAVING COUNT(A.tileID)=1)").arg(id));
    q.exec(QStringLiteral("DELETE FROM TilesDownload WHERE setID=%1").arg(id));
    q.exec(QStringLiteral("DELETE FROM TileSets      WHERE setID=%1").arg(id));
    q.exec(QStringLiteral("DELETE FROM SetTiles      WHERE setID=%1").arg(id));
    _updateTotals();
}

void QGCTileCacheDatabase::deleteTileSet(QGCDeleteTileSetTask* task)
{
    if (!_valid) { task->setError("No Cache Database"); return; }
    deleteTileSet(task->setID());
    task->setTileSetDeleted();
}

void QGCTileCacheDatabase::renameTileSet(QGCRenameTileSetTask* task)
{
    if (!_valid) { task->setError("No Cache Database"); return; }
    QMutexLocker lock(&_mutex);
    QSqlQuery(*_db).exec(QStringLiteral(
        "UPDATE TileSets SET name=\"%1\" WHERE setID=%2").arg(task->newName()).arg(task->setID()));
}

void QGCTileCacheDatabase::resetDatabase(QGCResetTask* task)
{
    if (!_valid) { task->setError("No Cache Database"); return; }
    QMutexLocker lock(&_mutex);
    QSqlQuery q(*_db);
    q.exec("DROP TABLE Tiles");
    q.exec("DROP TABLE TileSets");
    q.exec("DROP TABLE SetTiles");
    q.exec("DROP TABLE TilesDownload");
    _valid = _createSelfSchema();
    task->setResetCompleted();
}

// ---------------------------------------------------------------------
// import / export

void QGCTileCacheDatabase::importSets(QGCImportTileTask* task)
{
    if (!_valid) { task->setError("No Cache Database"); return; }
    QMutexLocker lock(&_mutex);

    if (task->replace()) {
        // Replace the entire database file
        close();
        (void) QFile::remove(_dbPath);
        (void) QFile::copy(task->path(), _dbPath);
        task->setProgress(25);
        open();
        if (_valid) {
            task->setProgress(50);
            deleteBingNoTileTiles();
        }
        task->setProgress(100);
        task->setImportCompleted();
        return;
    }

    // Merge import
    QSqlDatabase dbImport = QSqlDatabase::addDatabase("QSQLITE", kExportSession);
    dbImport.setDatabaseName(task->path());
    dbImport.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");
    if (!dbImport.open()) {
        task->setError("Error opening import database");
        QSqlDatabase::removeDatabase(kExportSession);
        return;
    }

    QSqlQuery qImp(dbImport);

    quint64 tileCount = 0;
    if (qImp.exec(QStringLiteral("SELECT COUNT(tileID) FROM Tiles")) && qImp.next())
        tileCount = qImp.value(0).toULongLong();

    if (tileCount > 0 && qImp.exec(QStringLiteral("SELECT * FROM TileSets ORDER BY defaultSet DESC, name ASC"))) {
        quint64 currentCount = 0;
        int lastProgress = -1;

        while (qImp.next()) {
            QString name = qImp.value("name").toString();
            const quint64 setID           = qImp.value("setID").toULongLong();
            const QString mapType         = qImp.value("typeStr").toString();
            const double tlLat            = qImp.value("topleftLat").toDouble();
            const double tlLon            = qImp.value("topleftLon").toDouble();
            const double brLat            = qImp.value("bottomRightLat").toDouble();
            const double brLon            = qImp.value("bottomRightLon").toDouble();
            const int    minZoom          = qImp.value("minZoom").toInt();
            const int    maxZoom          = qImp.value("maxZoom").toInt();
            const int    type             = qImp.value("type").toInt();
            const quint32 numTiles        = qImp.value("numTiles").toUInt();
            const int    defaultSet       = qImp.value("defaultSet").toInt();

            quint64 insertSetID = getDefaultTileSet();
            if (defaultSet == 0) {
                // Ensure unique name
                if (findTileSetID(name, insertSetID)) {
                    int suffix = 0;
                    while (true) {
                        const QString trial = QString::asprintf("%s %02d", name.toLatin1().constData(), ++suffix);
                        if (!findTileSetID(trial, insertSetID) || suffix > 99) {
                            name = trial;
                            break;
                        }
                    }
                }
                QSqlQuery c(*_db);
                c.prepare("INSERT INTO TileSets("
                          "name,typeStr,topleftLat,topleftLon,bottomRightLat,bottomRightLon,"
                          "minZoom,maxZoom,type,numTiles,defaultSet,date"
                          ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?)");
                c.addBindValue(name);
                c.addBindValue(mapType);
                c.addBindValue(tlLat);
                c.addBindValue(tlLon);
                c.addBindValue(brLat);
                c.addBindValue(brLon);
                c.addBindValue(minZoom);
                c.addBindValue(maxZoom);
                c.addBindValue(type);
                c.addBindValue(numTiles);
                c.addBindValue(defaultSet);
                c.addBindValue(QDateTime::currentSecsSinceEpoch());
                if (!c.exec()) {
                    task->setError("Error adding imported tile set to database");
                    continue;
                }
                insertSetID = c.lastInsertId().toULongLong();
            }

            // Copy unique tiles
            QSqlQuery qSel(dbImport);
            qSel.exec(QStringLiteral(
                "SELECT * FROM Tiles WHERE tileID IN (SELECT A.tileID FROM SetTiles A "
                "JOIN SetTiles B ON A.tileID=B.tileID WHERE B.setID=%1 "
                "GROUP BY A.tileID HAVING COUNT(A.tileID)=1)").arg(setID));

            quint64 tilesFound = 0;
            quint64 tilesSaved = 0;
            _db->transaction();
            while (qSel.next()) {
                tilesFound++;
                const QString   hash = qSel.value("hash").toString();
                const QString   fmt  = qSel.value("format").toString();
                const QByteArray img = qSel.value("tile").toByteArray();
                const int       tval = qSel.value("type").toInt();

                QSqlQuery c(*_db);
                c.prepare("INSERT INTO Tiles(hash,format,tile,size,type,date) "
                          "VALUES(?,?,?,?,?,?)");
                c.addBindValue(hash);
                c.addBindValue(fmt);
                c.addBindValue(img);
                c.addBindValue(img.size());
                c.addBindValue(tval);
                c.addBindValue(QDateTime::currentSecsSinceEpoch());
                if (c.exec()) {
                    tilesSaved++;
                    const quint64 newTileId = c.lastInsertId().toULongLong();
                    QSqlQuery(*_db).exec(QStringLiteral(
                        "INSERT INTO SetTiles(tileID,setID) VALUES(%1,%2)")
                        .arg(newTileId).arg(insertSetID));
                    currentCount++;
                    if (tileCount > 0) {
                        const int progress = static_cast<int>((static_cast<double>(currentCount) /
                                                               static_cast<double>(tileCount)) * 100.0);
                        if (progress != lastProgress) {
                            lastProgress = progress;
                            task->setProgress(progress);
                        }
                    }
                }
            }
            _db->commit();

            if (tilesSaved > 0) {
                QSqlQuery upd(*_db);
                if (upd.exec(QStringLiteral(
                        "SELECT COUNT(size) FROM Tiles A INNER JOIN SetTiles B ON A.tileID=B.tileID WHERE B.setID=%1")
                        .arg(insertSetID)) && upd.next())
                {
                    const quint64 count = upd.value(0).toULongLong();
                    QSqlQuery(*_db).exec(QStringLiteral(
                        "UPDATE TileSets SET numTiles=%1 WHERE setID=%2").arg(count).arg(insertSetID));
                }
            }

            const qint64 uniqueTiles = static_cast<qint64>(tilesFound) - static_cast<qint64>(tilesSaved);
            if (uniqueTiles < static_cast<qint64>(tileCount))
                tileCount -= uniqueTiles;
            else
                tileCount = 0;

            if (tilesSaved == 0 && defaultSet == 0) {
                deleteTileSet(insertSetID);
            }
        }
    } else {
        task->setError("No tile set in database");
    }

    dbImport.close();
    QSqlDatabase::removeDatabase(kExportSession);
    if (tileCount == 0) {
        task->setError("No unique tiles in imported database");
    }
    task->setImportCompleted();
}

void QGCTileCacheDatabase::exportSets(QGCExportTileTask* task)
{
    if (!_valid) { task->setError("No Cache Database"); return; }
    QMutexLocker lock(&_mutex);

    (void) QFile::remove(task->path());

    QSqlDatabase dbExp = QSqlDatabase::addDatabase("QSQLITE", kExportSession);
    dbExp.setDatabaseName(task->path());
    dbExp.setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");

    if (!dbExp.open()) {
        task->setError("Error opening export database");
        QSqlDatabase::removeDatabase(kExportSession);
        return;
    }

    if (!createSchema(dbExp, /*createDefault*/ false)) {
        task->setError("Error creating export database");
        dbExp.close();
        QSqlDatabase::removeDatabase(kExportSession);
        return;
    }

    // Progress target
    quint64 totalTiles = 0;
    for (int i = 0; i < task->sets().count(); ++i) {
        const QGCCachedTileSet* set = task->sets().at(i);
        totalTiles += set->defaultSet() ? set->totalTileCount()
                                        : set->uniqueTileCount();
    }
    if (totalTiles == 0) totalTiles = 1;

    quint64 processed = 0;
    QSqlQuery qExp(dbExp);

    // For each set
    for (int i = 0; i < task->sets().count(); ++i) {
        const QGCCachedTileSet* set = task->sets().at(i);

        qExp.prepare("INSERT INTO TileSets("
                     "name,typeStr,topleftLat,topleftLon,bottomRightLat,bottomRightLon,"
                     "minZoom,maxZoom,type,numTiles,defaultSet,date"
                     ") VALUES(?,?,?,?,?,?,?,?,?,?,?,?)");
        qExp.addBindValue(set->name());
        qExp.addBindValue(set->mapTypeStr());
        qExp.addBindValue(set->topleftLat());
        qExp.addBindValue(set->topleftLon());
        qExp.addBindValue(set->bottomRightLat());
        qExp.addBindValue(set->bottomRightLon());
        qExp.addBindValue(set->minZoom());
        qExp.addBindValue(set->maxZoom());
        qExp.addBindValue(UrlFactory::getQtMapIdFromProviderType(set->type()));
        qExp.addBindValue(set->totalTileCount());
        qExp.addBindValue(set->defaultSet());
        qExp.addBindValue(QDateTime::currentSecsSinceEpoch());

        if (!qExp.exec()) {
            task->setError("Error adding tile set to exported database");
            break;
        }

        const quint64 exportSetID = qExp.lastInsertId().toULongLong();

        // Iterate tiles in this set
        QSqlQuery qSrc(*_db);
        if (!qSrc.exec(QStringLiteral("SELECT * FROM SetTiles WHERE setID=%1").arg(set->id()))) {
            continue;
        }

        dbExp.transaction();
        while (qSrc.next()) {
            const quint64 tileID = qSrc.value("tileID").toULongLong();

            QSqlQuery qTile(*_db);
            if (!qTile.exec(QStringLiteral("SELECT * FROM Tiles WHERE tileID=%1").arg(tileID)) || !qTile.next())
                continue;

            const QString   hash = qTile.value("hash").toString();
            const QString   fmt  = qTile.value("format").toString();
            const QByteArray img = qTile.value("tile").toByteArray();
            const int       type = qTile.value("type").toInt();

            qExp.prepare("INSERT INTO Tiles(hash,format,tile,size,type,date) VALUES(?,?,?,?,?,?)");
            qExp.addBindValue(hash);
            qExp.addBindValue(fmt);
            qExp.addBindValue(img);
            qExp.addBindValue(img.size());
            qExp.addBindValue(type);
            qExp.addBindValue(QDateTime::currentSecsSinceEpoch());
            if (!qExp.exec()) continue;

            const quint64 exportTileID = qExp.lastInsertId().toULongLong();
            QSqlQuery(dbExp).exec(QStringLiteral(
                "INSERT INTO SetTiles(tileID,setID) VALUES(%1,%2)").arg(exportTileID).arg(exportSetID));

            ++processed;
            task->setProgress(static_cast<int>((static_cast<double>(processed) / totalTiles) * 100.0));
        }
        dbExp.commit();
    }

    dbExp.close();
    QSqlDatabase::removeDatabase(kExportSession);
    task->setExportCompleted();
}

// ---------------------------------------------------------------------
// totals / misc

void QGCTileCacheDatabase::queryTotals(quint32& totalCnt,
                                       quint64& totalSz,
                                       quint32& defaultCnt,
                                       quint64& defaultSz)
{
    QMutexLocker lock(&_mutex);
    _updateTotals();
    totalCnt   = _totalCount;
    totalSz    = _totalSize;
    defaultCnt = _defaultCount;
    defaultSz  = _defaultSize;
}

void QGCTileCacheDatabase::_updateTotals()
{
    QSqlQuery q(*_db);

    if (q.exec(QStringLiteral("SELECT COUNT(size), SUM(size) FROM Tiles")) && q.next()) {
        _totalCount = q.value(0).toUInt();
        _totalSize  = q.value(1).toULongLong();
    }

    if (q.exec(QStringLiteral(
            "SELECT COUNT(size), SUM(size) FROM Tiles WHERE tileID IN "
            "(SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID=B.tileID "
            " WHERE B.setID=%1 GROUP BY A.tileID HAVING COUNT(A.tileID)=1)")
            .arg(getDefaultTileSet())) && q.next())
    {
        _defaultCount = q.value(0).toUInt();
        _defaultSize  = q.value(1).toULongLong();
    }
}

void QGCTileCacheDatabase::_updateSetTotals(QGCCachedTileSet* set)
{
    if (set->defaultSet()) {
        _updateTotals();
        set->setSavedTileCount(_totalCount);
        set->setSavedTileSize(_totalSize);
        set->setTotalTileCount(_defaultCount);
        set->setTotalTileSize(_defaultSize);
        return;
    }

    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral(
            "SELECT COUNT(size), SUM(size) FROM Tiles A INNER JOIN SetTiles B "
            "ON A.tileID=B.tileID WHERE B.setID=%1").arg(set->id())) && q.next())
    {
        set->setSavedTileCount(q.value(0).toUInt());
        set->setSavedTileSize(q.value(1).toULongLong());
    }

    quint64 avg = UrlFactory::averageSizeForType(set->type());
    if (set->savedTileCount() > 10 && set->savedTileSize())
        avg = set->savedTileSize() / qMax<quint64>(1, set->savedTileCount());

    if (set->totalTileCount() <= set->savedTileCount())
        set->setTotalTileSize(set->savedTileSize());
    else
        set->setTotalTileSize(avg * set->totalTileCount());

    // Unique tiles
    quint32 ucount = 0;
    quint64 usize  = 0;
    if (q.exec(QStringLiteral(
            "SELECT COUNT(size), SUM(size) FROM Tiles WHERE tileID IN "
            "(SELECT A.tileID FROM SetTiles A JOIN SetTiles B ON A.tileID=B.tileID "
            " WHERE B.setID=%1 GROUP BY A.tileID HAVING COUNT(A.tileID)=1)").arg(set->id()))
        && q.next())
    {
        ucount = q.value(0).toUInt();
        usize  = q.value(1).toULongLong();
    }

    quint32 expectedUcount = set->totalTileCount() - set->savedTileCount();
    if (ucount == 0) {
        usize = static_cast<quint64>(expectedUcount) * avg;
    } else {
        expectedUcount = ucount;
    }
    set->setUniqueTileCount(expectedUcount);
    set->setUniqueTileSize(usize);
}

quint64 QGCTileCacheDatabase::_findTile(const QString& hash)
{
    QSqlQuery q(*_db);
    if (q.exec(QStringLiteral("SELECT tileID FROM Tiles WHERE hash=\"%1\"").arg(hash)) && q.next())
        return q.value(0).toULongLong();
    return 0;
}

// ---------------------------------------------------------------------
// schema

bool QGCTileCacheDatabase::createSchema(QSqlDatabase& db, bool createDefault)
{
    QSqlQuery q(db);

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS Tiles ("
            "tileID INTEGER PRIMARY KEY NOT NULL, "
            "hash TEXT NOT NULL UNIQUE, "
            "format TEXT NOT NULL, "
            "tile BLOB NULL, "
            "size INTEGER, "
            "type INTEGER, "
            "date INTEGER DEFAULT 0)"))
    {
        qCWarning(QGCDBLog) << "Create Tiles:" << q.lastError().text();
        return false;
    }
    (void) q.exec("CREATE INDEX IF NOT EXISTS hash ON Tiles ( hash, size, type )");

    if (!q.exec(
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
        qCWarning(QGCDBLog) << "Create TileSets:" << q.lastError().text();
        return false;
    }

    if (!q.exec("CREATE TABLE IF NOT EXISTS SetTiles (setID INTEGER, tileID INTEGER)")) {
        qCWarning(QGCDBLog) << "Create SetTiles:" << q.lastError().text();
        return false;
    }

    if (!q.exec(
            "CREATE TABLE IF NOT EXISTS TilesDownload ("
            "setID INTEGER, "
            "hash TEXT NOT NULL UNIQUE, "
            "type INTEGER, "
            "x INTEGER, "
            "y INTEGER, "
            "z INTEGER, "
            "state INTEGER DEFAULT 0)"))
    {
        qCWarning(QGCDBLog) << "Create TilesDownload:" << q.lastError().text();
        return false;
    }

    if (createDefault) {
        (void) q.exec(QStringLiteral(
            "INSERT OR IGNORE INTO TileSets(name, defaultSet, date) "
            "VALUES(\"Default Tile Set\", 1, %1)")
            .arg(QDateTime::currentSecsSinceEpoch()));
    }

    return true;
}

bool QGCTileCacheDatabase::_createSelfSchema(bool createDefault)
{
    return createSchema(*_db, createDefault);
}
