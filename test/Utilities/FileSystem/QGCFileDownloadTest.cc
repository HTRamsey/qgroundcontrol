#include "QGCFileDownloadTest.h"
#include "QGCCachedFileDownload.h"
#include "QGCFileDownload.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

void QGCFileDownloadTest::_testFileDownload()
{
    // Local File
    QGCFileDownload *const downloader = new QGCFileDownload(this);
    (void) connect(downloader, &QGCFileDownload::downloadComplete, this,
        [this](const QString &remoteFile, const QString &localFile, const QString &errorMsg) {
            sender()->deleteLater();
            QVERIFY(errorMsg.isEmpty());
        });
    QSignalSpy spyQGCFileDownloadDownloadComplete(downloader, &QGCFileDownload::downloadComplete);
    QVERIFY(downloader->download(":/unittest/arducopter.apj"));
    QVERIFY(spyQGCFileDownloadDownloadComplete.wait(1000));
}

void QGCFileDownloadTest::_testCachedFileDownload()
{
    QGCCachedFileDownload file("");
}

void QGCFileDownloadTest::_testAutoDecompressGzip()
{
    // Test downloading a .gz file with autoDecompress=true
    // Uses manifest.json.gz from compression test resources
    QGCFileDownload *const downloader = new QGCFileDownload(this);
    QString resultLocalFile;
    QString resultErrorMsg;

    (void) connect(downloader, &QGCFileDownload::downloadComplete, this,
        [&resultLocalFile, &resultErrorMsg](const QString &remoteFile, const QString &localFile, const QString &errorMsg) {
            Q_UNUSED(remoteFile);
            resultLocalFile = localFile;
            resultErrorMsg = errorMsg;
        });

    QSignalSpy spy(downloader, &QGCFileDownload::downloadComplete);

    // Download with autoDecompress=true
    QVERIFY(downloader->download(":/unittest/manifest.json.gz", {}, true));
    QVERIFY(spy.wait(5000));

    downloader->deleteLater();

    // Verify no error
    QVERIFY2(resultErrorMsg.isEmpty(), qPrintable(resultErrorMsg));

    // Verify the output file is decompressed (should be manifest.json, not .gz)
    QVERIFY2(!resultLocalFile.isEmpty(), "Local file path is empty");
    QVERIFY2(!resultLocalFile.endsWith(".gz"), "File should be decompressed (no .gz extension)");
    QVERIFY2(QFile::exists(resultLocalFile), qPrintable("Output file doesn't exist: " + resultLocalFile));

    // Verify content is valid JSON (not compressed data)
    QFile outputFile(resultLocalFile);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    const QByteArray content = outputFile.readAll();
    outputFile.close();

    QVERIFY2(!content.isEmpty(), "Decompressed file is empty");
    QVERIFY2(content.contains("\"name\""), "Content doesn't look like valid JSON");

    // Cleanup
    QFile::remove(resultLocalFile);
}

void QGCFileDownloadTest::_testAutoDecompressDisabled()
{
    // Test downloading a .gz file with autoDecompress=false (default)
    // The file should remain compressed
    QGCFileDownload *const downloader = new QGCFileDownload(this);
    QString resultLocalFile;
    QString resultErrorMsg;

    (void) connect(downloader, &QGCFileDownload::downloadComplete, this,
        [&resultLocalFile, &resultErrorMsg](const QString &remoteFile, const QString &localFile, const QString &errorMsg) {
            Q_UNUSED(remoteFile);
            resultLocalFile = localFile;
            resultErrorMsg = errorMsg;
        });

    QSignalSpy spy(downloader, &QGCFileDownload::downloadComplete);

    // Download with autoDecompress=false (default)
    QVERIFY(downloader->download(":/unittest/manifest.json.gz", {}, false));
    QVERIFY(spy.wait(5000));

    downloader->deleteLater();

    // Verify no error
    QVERIFY2(resultErrorMsg.isEmpty(), qPrintable(resultErrorMsg));

    // Verify the output file is still compressed (.gz extension)
    QVERIFY2(!resultLocalFile.isEmpty(), "Local file path is empty");
    QVERIFY2(resultLocalFile.endsWith(".gz"), "File should remain compressed with .gz extension");
    QVERIFY2(QFile::exists(resultLocalFile), qPrintable("Output file doesn't exist: " + resultLocalFile));

    // Verify content is compressed (starts with gzip magic bytes 1f 8b)
    QFile outputFile(resultLocalFile);
    QVERIFY(outputFile.open(QIODevice::ReadOnly));
    const QByteArray content = outputFile.readAll();
    outputFile.close();

    QVERIFY2(!content.isEmpty(), "File is empty");
    QVERIFY2(content.size() >= 2, "File too small to be gzip");
    QVERIFY2(static_cast<unsigned char>(content[0]) == 0x1f &&
             static_cast<unsigned char>(content[1]) == 0x8b,
             "File doesn't have gzip magic bytes");

    // Cleanup
    QFile::remove(resultLocalFile);
}

void QGCFileDownloadTest::_testAutoDecompressUncompressedFile()
{
    // Test downloading a non-compressed file with autoDecompress=true
    // Should work normally without any decompression attempt
    QGCFileDownload *const downloader = new QGCFileDownload(this);
    QString resultLocalFile;
    QString resultErrorMsg;

    (void) connect(downloader, &QGCFileDownload::downloadComplete, this,
        [&resultLocalFile, &resultErrorMsg](const QString &remoteFile, const QString &localFile, const QString &errorMsg) {
            Q_UNUSED(remoteFile);
            resultLocalFile = localFile;
            resultErrorMsg = errorMsg;
        });

    QSignalSpy spy(downloader, &QGCFileDownload::downloadComplete);

    // Download non-compressed file with autoDecompress=true
    QVERIFY(downloader->download(":/unittest/arducopter.apj", {}, true));
    QVERIFY(spy.wait(5000));

    downloader->deleteLater();

    // Verify no error
    QVERIFY2(resultErrorMsg.isEmpty(), qPrintable(resultErrorMsg));

    // Verify file downloaded successfully
    QVERIFY2(!resultLocalFile.isEmpty(), "Local file path is empty");
    QVERIFY2(QFile::exists(resultLocalFile), qPrintable("Output file doesn't exist: " + resultLocalFile));
    QVERIFY2(QFileInfo(resultLocalFile).size() > 0, "Downloaded file is empty");

    // Cleanup
    QFile::remove(resultLocalFile);
}
