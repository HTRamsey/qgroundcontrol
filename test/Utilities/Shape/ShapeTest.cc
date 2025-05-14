#include "ShapeTest.h"
#include "ShapeFileHelper.h"

#include <QtTest/QTest>

void ShapeTest::_testLoadPolylineFromSHP()
{
    const QTemporaryDir tmpDir;
    const auto copyRes = [&tmpDir](const QString &name) -> QString {
        const QString dstPath = tmpDir.filePath(name);
        (void) QFile::remove(dstPath);
        const QString resPath = QStringLiteral(":/unittest/%1").arg(name);
        (void) QFile(resPath).copy(dstPath);
        return dstPath;
    };

    const QString shpFile = copyRes("pline.shp");
    (void) copyRes("pline.dbf");
    (void) copyRes("pline.shx");
    (void) copyRes("pline.prj");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    QVERIFY(ShapeFileHelper::loadPolylineFromFile(shpFile, rgCoords, errorString));
}

void ShapeTest::_testLoadPolygonFromSHP()
{
    const QTemporaryDir tmpDir;
    const auto copyRes = [&tmpDir](const QString &name) -> QString {
        const QString dstPath = tmpDir.filePath(name);
        (void) QFile::remove(dstPath);
        const QString resPath = QStringLiteral(":/unittest/%1").arg(name);
        (void) QFile(resPath).copy(dstPath);
        return dstPath;
    };

    const QString shpFile = copyRes("polygon.shp");
    (void) copyRes("polygon.dbf");
    (void) copyRes("polygon.shx");
    (void) copyRes("polygon.prj");
    QString errorString;
    QList<QGeoCoordinate> rgCoords;
    QVERIFY(ShapeFileHelper::loadPolygonFromFile(shpFile, rgCoords, errorString));
}
