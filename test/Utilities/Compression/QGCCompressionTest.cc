#include "QGCCompressionTest.h"
#include "QGCCompression.h"

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QCryptographicHash>
#include <QtCore/QRegularExpression>
#include <QtTest/QTest>

void QGCCompressionTest::init()
{
    UnitTest::init();

    _tempOutputDir = new QTemporaryDir();
    QVERIFY(_tempOutputDir->isValid());
}

void QGCCompressionTest::cleanup()
{
    delete _tempOutputDir;
    _tempOutputDir = nullptr;

    UnitTest::cleanup();
}

bool QGCCompressionTest::_compareFiles(const QString &file1, const QString &file2)
{
    QFile f1(file1);
    QFile f2(file2);

    if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)) {
        return false;
    }

    QCryptographicHash hash1(QCryptographicHash::Sha256);
    QCryptographicHash hash2(QCryptographicHash::Sha256);
    hash1.addData(&f1);
    hash2.addData(&f2);

    return hash1.result() == hash2.result();
}

// ============================================================================
// Format Detection Tests
// ============================================================================

void QGCCompressionTest::_testFormatDetection()
{
    // Extension-based detection
    QCOMPARE(QGCCompression::detectFormat("file.zip"), QGCCompression::Format::ZIP);
    QCOMPARE(QGCCompression::detectFormat("file.7z"), QGCCompression::Format::SEVENZ);
    QCOMPARE(QGCCompression::detectFormat("file.gz"), QGCCompression::Format::GZIP);
    QCOMPARE(QGCCompression::detectFormat("file.gzip"), QGCCompression::Format::GZIP);
    QCOMPARE(QGCCompression::detectFormat("file.xz"), QGCCompression::Format::XZ);
    QCOMPARE(QGCCompression::detectFormat("file.lzma"), QGCCompression::Format::XZ);
    QCOMPARE(QGCCompression::detectFormat("file.zst"), QGCCompression::Format::ZSTD);
    QCOMPARE(QGCCompression::detectFormat("file.zstd"), QGCCompression::Format::ZSTD);
    QCOMPARE(QGCCompression::detectFormat("file.tar"), QGCCompression::Format::TAR);
    QCOMPARE(QGCCompression::detectFormat("file.tar.gz"), QGCCompression::Format::TAR_GZ);
    QCOMPARE(QGCCompression::detectFormat("file.tgz"), QGCCompression::Format::TAR_GZ);
    QCOMPARE(QGCCompression::detectFormat("file.tar.xz"), QGCCompression::Format::TAR_XZ);
    QCOMPARE(QGCCompression::detectFormat("file.txz"), QGCCompression::Format::TAR_XZ);
    QCOMPARE(QGCCompression::detectFormat("file.unknown"), QGCCompression::Format::Auto);
    QCOMPARE(QGCCompression::detectFormat("file.txt"), QGCCompression::Format::Auto);

    // Case insensitive
    QCOMPARE(QGCCompression::detectFormat("FILE.ZIP"), QGCCompression::Format::ZIP);
    QCOMPARE(QGCCompression::detectFormat("File.Gz"), QGCCompression::Format::GZIP);
}

void QGCCompressionTest::_testFormatHelpers()
{
    // isArchiveFormat
    QVERIFY(QGCCompression::isArchiveFormat(QGCCompression::Format::ZIP));
    QVERIFY(QGCCompression::isArchiveFormat(QGCCompression::Format::SEVENZ));
    QVERIFY(QGCCompression::isArchiveFormat(QGCCompression::Format::TAR));
    QVERIFY(QGCCompression::isArchiveFormat(QGCCompression::Format::TAR_GZ));
    QVERIFY(QGCCompression::isArchiveFormat(QGCCompression::Format::TAR_XZ));
    QVERIFY(!QGCCompression::isArchiveFormat(QGCCompression::Format::GZIP));
    QVERIFY(!QGCCompression::isArchiveFormat(QGCCompression::Format::XZ));
    QVERIFY(!QGCCompression::isArchiveFormat(QGCCompression::Format::ZSTD));

    // isCompressionFormat
    QVERIFY(QGCCompression::isCompressionFormat(QGCCompression::Format::GZIP));
    QVERIFY(QGCCompression::isCompressionFormat(QGCCompression::Format::XZ));
    QVERIFY(QGCCompression::isCompressionFormat(QGCCompression::Format::ZSTD));
    QVERIFY(!QGCCompression::isCompressionFormat(QGCCompression::Format::ZIP));
    QVERIFY(!QGCCompression::isCompressionFormat(QGCCompression::Format::TAR));

    // isCompressedFile / isArchiveFile
    QVERIFY(QGCCompression::isCompressedFile("file.gz"));
    QVERIFY(QGCCompression::isCompressedFile("file.xz"));
    QVERIFY(!QGCCompression::isCompressedFile("file.zip"));
    QVERIFY(!QGCCompression::isCompressedFile("file.txt"));
    QVERIFY(QGCCompression::isArchiveFile("file.zip"));
    QVERIFY(QGCCompression::isArchiveFile("file.7z"));
    QVERIFY(QGCCompression::isArchiveFile("file.tar.gz"));
    QVERIFY(!QGCCompression::isArchiveFile("file.gz"));

    // strippedPath
    QCOMPARE(QGCCompression::strippedPath("file.gz"), QString("file"));
    QCOMPARE(QGCCompression::strippedPath("file.xz"), QString("file"));
    QCOMPARE(QGCCompression::strippedPath("file.zst"), QString("file"));
    QCOMPARE(QGCCompression::strippedPath("file.txt"), QString("file.txt"));
    QCOMPARE(QGCCompression::strippedPath("path/to/file.gz"), QString("path/to/file"));

    // formatExtension
    QCOMPARE(QGCCompression::formatExtension(QGCCompression::Format::ZIP), QString(".zip"));
    QCOMPARE(QGCCompression::formatExtension(QGCCompression::Format::GZIP), QString(".gz"));
    QCOMPARE(QGCCompression::formatExtension(QGCCompression::Format::TAR_GZ), QString(".tar.gz"));

    // formatName
    QVERIFY(!QGCCompression::formatName(QGCCompression::Format::ZIP).isEmpty());
    QVERIFY(!QGCCompression::formatName(QGCCompression::Format::GZIP).isEmpty());
}

// ============================================================================
// Archive Extraction Tests (from Qt resources)
// ============================================================================

void QGCCompressionTest::_testZipFromResource()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");
    const QString outputPath = _tempOutputDir->path() + "/resource_output";

    QVERIFY2(QGCCompression::extractArchive(zipResource, outputPath),
             "Failed to extract ZIP from Qt resource");
    QVERIFY2(QDir(outputPath).exists(), "Output directory not created");

    QDir outputDir(outputPath);
    QStringList files = outputDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QVERIFY2(!files.isEmpty(), "No files extracted from ZIP");
}

void QGCCompressionTest::_test7zFromResource()
{
    const QString archiveResource = QStringLiteral(":/unittest/manifest.json.7z");
    const QString outputPath = _tempOutputDir->path() + "/7z_output";

    QVERIFY2(QGCCompression::extractArchive(archiveResource, outputPath, QGCCompression::Format::SEVENZ),
             "Failed to extract 7z from Qt resource");
    QVERIFY2(QDir(outputPath).exists(), "Output directory not created");

    QDir outputDir(outputPath);
    QStringList files = outputDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QVERIFY2(!files.isEmpty(), "No files extracted from 7z");

    QVERIFY2(QFile::exists(outputPath + "/manifest.json"), "manifest.json not extracted");
}

void QGCCompressionTest::_testListArchive()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    QStringList entries = QGCCompression::listArchive(zipResource);
    QVERIFY2(!entries.isEmpty(), "listArchive returned empty list");

    bool foundManifest = false;
    for (const QString &entry : entries) {
        if (entry.contains("manifest.json")) {
            foundManifest = true;
            break;
        }
    }
    QVERIFY2(foundManifest, "manifest.json not found in archive listing");

    // Test auto-detection
    QStringList autoEntries = QGCCompression::listArchive(zipResource, QGCCompression::Format::Auto);
    QCOMPARE(autoEntries.size(), entries.size());
}

void QGCCompressionTest::_testListArchiveDetailed()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    QList<QGCCompression::ArchiveEntry> entries = QGCCompression::listArchiveDetailed(zipResource);
    QVERIFY(!entries.isEmpty());

    bool foundManifest = false;
    for (const auto &entry : entries) {
        if (entry.name == "manifest.json") {
            foundManifest = true;
            QVERIFY(entry.size > 0);
            QVERIFY(!entry.isDirectory);
            break;
        }
    }
    QVERIFY2(foundManifest, "manifest.json not found in detailed listing");
}

void QGCCompressionTest::_testGetArchiveStats()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    const QGCCompression::ArchiveStats stats = QGCCompression::getArchiveStats(zipResource);

    // Should have at least one file
    QVERIFY(stats.totalEntries > 0);
    QVERIFY(stats.fileCount > 0);
    QVERIFY(stats.totalUncompressedSize > 0);
    QVERIFY(stats.largestFileSize > 0);
    QVERIFY(!stats.largestFileName.isEmpty());

    // File count + directory count should equal total entries
    QCOMPARE(stats.totalEntries, stats.fileCount + stats.directoryCount);

    // Largest file size should not exceed total size
    QVERIFY(stats.largestFileSize <= stats.totalUncompressedSize);

    // Test with 7z archive
    const QString archiveResource = QStringLiteral(":/unittest/manifest.json.7z");
    const QGCCompression::ArchiveStats stats7z = QGCCompression::getArchiveStats(archiveResource);
    QVERIFY(stats7z.totalEntries > 0);
    QVERIFY(stats7z.fileCount > 0);

    // Test non-existent file returns zero stats
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    const QGCCompression::ArchiveStats emptyStats = QGCCompression::getArchiveStats("/nonexistent/file.zip");
    QCOMPARE(emptyStats.totalEntries, 0);
    QCOMPARE(emptyStats.fileCount, 0);
    QCOMPARE(emptyStats.totalUncompressedSize, 0);
}

void QGCCompressionTest::_testValidateArchive()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");
    QVERIFY(QGCCompression::validateArchive(zipResource));

    // Test corrupt archive fails validation
    const QString corruptZip = _tempOutputDir->path() + "/corrupt_validate.zip";
    QFile corrupt(corruptZip);
    QVERIFY(corrupt.open(QIODevice::WriteOnly));
    corrupt.write("This is not a valid ZIP file");
    corrupt.close();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Failed to open file.*Unrecognized archive format"));
    QVERIFY(!QGCCompression::validateArchive(corruptZip));

    // Test non-existent file
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    QVERIFY(!QGCCompression::validateArchive("/nonexistent/path/file.zip"));
}

void QGCCompressionTest::_testFileExists()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    // File that exists
    QVERIFY(QGCCompression::fileExists(zipResource, "manifest.json"));

    // File that doesn't exist
    QVERIFY(!QGCCompression::fileExists(zipResource, "nonexistent.txt"));

    // Empty file name
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File name cannot be empty"));
    QVERIFY(!QGCCompression::fileExists(zipResource, ""));

    // Non-existent archive
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    QVERIFY(!QGCCompression::fileExists("/nonexistent/path/file.zip", "test.txt"));
}

void QGCCompressionTest::_testExtractArchiveFiltered()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    // Test filter that accepts all files
    const QString outputDir1 = _tempOutputDir->path() + "/filtered_all";
    int extractedCount = 0;
    bool success = QGCCompression::extractArchiveFiltered(zipResource, outputDir1,
        [&extractedCount](const QGCCompression::ArchiveEntry &entry) -> bool {
            Q_UNUSED(entry);
            extractedCount++;
            return true;  // Accept all
        });
    QVERIFY2(success, "extractArchiveFiltered with accept-all filter failed");
    QVERIFY(extractedCount > 0);
    QVERIFY(QFile::exists(outputDir1 + "/manifest.json"));

    // Test filter that rejects all files
    const QString outputDir2 = _tempOutputDir->path() + "/filtered_none";
    success = QGCCompression::extractArchiveFiltered(zipResource, outputDir2,
        [](const QGCCompression::ArchiveEntry &) -> bool {
            return false;  // Reject all
        });
    QVERIFY2(success, "extractArchiveFiltered with reject-all filter should still succeed");
    // Directory should exist but be empty (or not created at all)
    QDir dir2(outputDir2);
    if (dir2.exists()) {
        QStringList files = dir2.entryList(QDir::Files | QDir::NoDotAndDotDot);
        QVERIFY2(files.isEmpty(), "No files should be extracted when filter rejects all");
    }

    // Test filter by extension (only .json files)
    const QString outputDir3 = _tempOutputDir->path() + "/filtered_json";
    QStringList extractedNames;
    success = QGCCompression::extractArchiveFiltered(zipResource, outputDir3,
        [&extractedNames](const QGCCompression::ArchiveEntry &entry) -> bool {
            if (entry.name.endsWith(".json")) {
                extractedNames.append(entry.name);
                return true;
            }
            return false;
        });
    QVERIFY(success);
    QVERIFY(extractedNames.contains("manifest.json"));

    // Test filter receives correct metadata
    bool metadataCorrect = true;
    success = QGCCompression::extractArchiveFiltered(zipResource, _tempOutputDir->path() + "/filtered_meta",
        [&metadataCorrect](const QGCCompression::ArchiveEntry &entry) -> bool {
            if (entry.name == "manifest.json") {
                // Verify metadata is populated
                if (entry.size <= 0) metadataCorrect = false;
                if (entry.isDirectory) metadataCorrect = false;
            }
            return true;
        });
    QVERIFY(success);
    QVERIFY2(metadataCorrect, "Filter did not receive correct metadata");

    // Test with non-existent file
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    success = QGCCompression::extractArchiveFiltered("/nonexistent/file.zip", outputDir1,
        [](const QGCCompression::ArchiveEntry &) { return true; });
    QVERIFY(!success);
}

void QGCCompressionTest::_testExtractSingleFile()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    const QString extractedPath = _tempOutputDir->path() + "/extracted_manifest.json";
    QVERIFY(QGCCompression::extractFile(zipResource, "manifest.json", extractedPath));
    QVERIFY(QFile::exists(extractedPath));
    QVERIFY(QFileInfo(extractedPath).size() > 0);

    // Verify content contains expected JSON structure
    QFile extracted(extractedPath);
    QVERIFY(extracted.open(QIODevice::ReadOnly));
    const QByteArray content = extracted.readAll();
    QVERIFY(content.contains("\"name\""));
    extracted.close();

    // Test non-existent file returns false
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File not found in archive"));
    QVERIFY(!QGCCompression::extractFile(zipResource, "nonexistent.txt", _tempOutputDir->path() + "/nope.txt"));
}

void QGCCompressionTest::_testExtractFileData()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    const QByteArray extracted = QGCCompression::extractFileData(zipResource, "manifest.json");
    QVERIFY(!extracted.isEmpty());
    QVERIFY(extracted.contains("\"name\""));

    // Test non-existent file returns empty
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File not found in archive"));
    const QByteArray notFound = QGCCompression::extractFileData(zipResource, "nonexistent.txt");
    QVERIFY(notFound.isEmpty());

    // Test empty file name returns empty
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File name cannot be empty"));
    const QByteArray emptyName = QGCCompression::extractFileData(zipResource, "");
    QVERIFY(emptyName.isEmpty());
}

void QGCCompressionTest::_testExtractMultipleFiles()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    // Extract available file(s)
    const QString outputDir = _tempOutputDir->path() + "/multi_extract";
    QStringList filesToExtract = {"manifest.json"};
    QVERIFY(QGCCompression::extractFiles(zipResource, filesToExtract, outputDir));
    QVERIFY(QFile::exists(outputDir + "/manifest.json"));

    // Test with non-existent file in list - should fail
    const QString outputDir2 = _tempOutputDir->path() + "/multi_extract2";
    QStringList badFiles = {"manifest.json", "nonexistent.txt"};
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File not found in archive"));
    QVERIFY(!QGCCompression::extractFiles(zipResource, badFiles, outputDir2));

    // Test empty list succeeds (no-op)
    QVERIFY(QGCCompression::extractFiles(zipResource, {}, outputDir));
}

void QGCCompressionTest::_testExtractByPattern()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");

    // Test wildcard pattern matching
    const QString outputDir = _tempOutputDir->path() + "/pattern_extract";
    QStringList extractedFiles;

    QVERIFY(QGCCompression::extractByPattern(zipResource, {"*.json"}, outputDir, &extractedFiles));
    QVERIFY(!extractedFiles.isEmpty());
    QVERIFY(extractedFiles.contains("manifest.json"));
    QVERIFY(QFile::exists(outputDir + "/manifest.json"));

    // Test exact filename as pattern (should work like extractFiles)
    const QString outputDir2 = _tempOutputDir->path() + "/pattern_exact";
    QStringList exactFiles;
    QVERIFY(QGCCompression::extractByPattern(zipResource, {"manifest.json"}, outputDir2, &exactFiles));
    QCOMPARE(exactFiles.size(), 1);
    QCOMPARE(exactFiles.first(), QStringLiteral("manifest.json"));

    // Test no matches returns false
    const QString outputDir3 = _tempOutputDir->path() + "/pattern_nomatch";
    QVERIFY(!QGCCompression::extractByPattern(zipResource, {"*.xyz"}, outputDir3));

    // Test empty patterns returns false
    const QString outputDir4 = _tempOutputDir->path() + "/pattern_empty";
    QVERIFY(!QGCCompression::extractByPattern(zipResource, {}, outputDir4));
}

// ============================================================================
// Single-File Decompression Tests (from Qt resources)
// ============================================================================

void QGCCompressionTest::_testDecompressFromResource()
{
    struct ResourceTest {
        const char *resource;
        const char *outputName;
        const char *name;
    };

    const ResourceTest resources[] = {
        { ":/unittest/manifest.json.gz",  "manifest_gz.json",  "GZIP" },
        { ":/unittest/manifest.json.xz",  "manifest_xz.json",  "XZ" },
        { ":/unittest/manifest.json.zst", "manifest_zst.json", "ZSTD" },
        { ":/unittest/manifest.json.bz2", "manifest_bz2.json", "BZip2" },
        { ":/unittest/manifest.json.lz4", "manifest_lz4.json", "LZ4" },
    };

    for (const auto &res : resources) {
        const QString outputFile = _tempOutputDir->path() + "/" + res.outputName;

        QVERIFY2(QGCCompression::decompressFile(res.resource, outputFile),
                 qPrintable(QString("Failed to decompress %1 file").arg(res.name)));
        QVERIFY(QFile::exists(outputFile));
        QVERIFY(QFileInfo(outputFile).size() > 0);
    }
}

void QGCCompressionTest::_testDecompressData()
{
    // Read compressed data from resources and decompress in memory
    struct ResourceTest {
        const char *resource;
        const char *name;
    };

    const ResourceTest resources[] = {
        { ":/unittest/manifest.json.gz",  "GZIP" },
        { ":/unittest/manifest.json.xz",  "XZ" },
        { ":/unittest/manifest.json.zst", "ZSTD" },
        { ":/unittest/manifest.json.bz2", "BZip2" },
        { ":/unittest/manifest.json.lz4", "LZ4" },
    };

    for (const auto &res : resources) {
        QFile resourceFile(res.resource);
        QVERIFY2(resourceFile.open(QIODevice::ReadOnly),
                 qPrintable(QString("Failed to open resource: %1").arg(res.resource)));
        const QByteArray compressedData = resourceFile.readAll();
        resourceFile.close();

        QVERIFY2(!compressedData.isEmpty(),
                 qPrintable(QString("Resource is empty: %1").arg(res.resource)));

        const QByteArray decompressed = QGCCompression::decompressData(compressedData);
        QVERIFY2(!decompressed.isEmpty(),
                 qPrintable(QString("%1 decompression returned empty data").arg(res.name)));
        QVERIFY2(decompressed.contains("\"name\""),
                 qPrintable(QString("%1 decompressed content invalid").arg(res.name)));
    }
}

void QGCCompressionTest::_testDecompressIfNeeded()
{
    // Test with compressed file
    const QString gzResource = QStringLiteral(":/unittest/manifest.json.gz");
    const QString outputPath = _tempOutputDir->path() + "/decompress_if_needed.json";

    QString result = QGCCompression::decompressIfNeeded(gzResource, outputPath);
    QVERIFY2(!result.isEmpty(), "decompressIfNeeded should return output path");
    QCOMPARE(result, outputPath);
    QVERIFY(QFile::exists(outputPath));
    QVERIFY(QFileInfo(outputPath).size() > 0);

    // Test with non-compressed file (should return original path)
    const QString plainFile = _tempOutputDir->path() + "/plain.txt";
    QFile plain(plainFile);
    QVERIFY(plain.open(QIODevice::WriteOnly));
    plain.write("plain text content");
    plain.close();

    result = QGCCompression::decompressIfNeeded(plainFile);
    QCOMPARE(result, plainFile);
    QVERIFY(QFile::exists(plainFile));

    // Test with non-existent file
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    result = QGCCompression::decompressIfNeeded("/nonexistent/file.gz");
    QVERIFY(result.isEmpty());
}

// ============================================================================
// Progress Callback Tests
// ============================================================================

void QGCCompressionTest::_testProgressCallbackExtract()
{
    const QString zipResource = QStringLiteral(":/unittest/manifest.json.zip");
    const QString outputPath = _tempOutputDir->path() + "/progress_extract";

    qint64 lastProgress = 0;
    int callCount = 0;
    bool progressValid = true;

    bool success = QGCCompression::extractArchive(zipResource, outputPath, QGCCompression::Format::ZIP,
        [&](qint64 bytesProcessed, qint64 totalBytes) -> bool {
            if (bytesProcessed < lastProgress) progressValid = false;
            lastProgress = bytesProcessed;
            callCount++;
            Q_UNUSED(totalBytes);
            return true;  // Continue
        });

    QVERIFY2(success, "Archive extraction with progress callback failed");
    QVERIFY2(progressValid, "Progress values were invalid");
    // Note: callCount may be 0 for very small archives
    QVERIFY(QDir(outputPath).exists());
}

// ============================================================================
// Edge Cases and Error Handling Tests
// ============================================================================

void QGCCompressionTest::_testCorruptArchive()
{
    // Create a file with garbage data pretending to be a ZIP
    const QString corruptZip = _tempOutputDir->path() + "/corrupt.zip";
    QFile file(corruptZip);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("This is not a valid ZIP file - just garbage data!");
    file.close();

    // Attempt to extract - should fail gracefully
    const QString outputPath = _tempOutputDir->path() + "/corrupt_output";
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Failed to open file.*Unrecognized archive format"));
    bool success = QGCCompression::extractArchive(corruptZip, outputPath);
    QVERIFY2(!success, "Extracting corrupt archive should fail");

    // Attempt to list - should return empty or fail
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Failed to open file.*Unrecognized archive format"));
    QStringList entries = QGCCompression::listArchive(corruptZip);
    QVERIFY2(entries.isEmpty(), "Listing corrupt archive should return empty list");

    // Test corrupt compressed file with valid GZIP header but truncated data
    const QString corruptGz = _tempOutputDir->path() + "/corrupt.gz";
    QFile gzFile(corruptGz);
    QVERIFY(gzFile.open(QIODevice::WriteOnly));
    const char corruptGzipData[] = {
        '\x1f', '\x8b',  // GZIP magic
        '\x08',          // Deflate method
        '\x00',          // Flags
        '\x00', '\x00', '\x00', '\x00',  // Modification time
        '\x00',          // Extra flags
        '\xff',          // OS (unknown)
    };
    gzFile.write(corruptGzipData, sizeof(corruptGzipData));
    gzFile.close();

    const QString decompressedPath = _tempOutputDir->path() + "/corrupt_decompressed";
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Failed to open file.*truncated gzip input"));
    success = QGCCompression::decompressFile(corruptGz, decompressedPath);
    QVERIFY2(!success, "Decompressing truncated GZIP file should fail");
}

void QGCCompressionTest::_testNonExistentInput()
{
    const QString nonExistent = "/path/that/does/not/exist/file.txt";
    const QString outputPath = _tempOutputDir->path() + "/output";

    // Test decompressFile with non-existent input
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    QVERIFY2(!QGCCompression::decompressFile(nonExistent + ".gz", outputPath),
             "decompressFile should fail for non-existent input");

    // Test extractArchive with non-existent archive
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    QVERIFY2(!QGCCompression::extractArchive(nonExistent + ".zip", outputPath),
             "extractArchive should fail for non-existent archive");

    // Test listArchive with non-existent file
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File does not exist"));
    QStringList entries = QGCCompression::listArchive(nonExistent + ".zip");
    QVERIFY2(entries.isEmpty(), "listArchive should return empty for non-existent file");
}

// ============================================================================
// QIODevice-based Operation Tests
// ============================================================================

void QGCCompressionTest::_testDecompressFromDevice()
{
    // Load compressed data from Qt resource into a QBuffer
    QFile resourceFile(":/unittest/manifest.json.gz");
    QVERIFY(resourceFile.open(QIODevice::ReadOnly));
    const QByteArray compressedData = resourceFile.readAll();
    resourceFile.close();
    QVERIFY(!compressedData.isEmpty());

    // Test decompression to file using QBuffer as device
    QBuffer buffer;
    buffer.setData(compressedData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));

    const QString outputPath = _tempOutputDir->path() + "/device_decompressed.json";
    QVERIFY2(QGCCompression::decompressFromDevice(&buffer, outputPath),
             "Failed to decompress from QBuffer device");
    QVERIFY(QFile::exists(outputPath));
    QVERIFY(QFileInfo(outputPath).size() > 0);

    // Verify content
    QFile decompressed(outputPath);
    QVERIFY(decompressed.open(QIODevice::ReadOnly));
    const QByteArray content = decompressed.readAll();
    QVERIFY(content.contains("\"name\""));
    decompressed.close();
    buffer.close();

    // Test decompression to memory
    buffer.setData(compressedData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));

    const QByteArray memoryResult = QGCCompression::decompressFromDevice(&buffer);
    QVERIFY2(!memoryResult.isEmpty(), "Decompress to memory from device failed");
    QVERIFY(memoryResult.contains("\"name\""));
    QCOMPARE(memoryResult, content);
    buffer.close();

    // Test with null device
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Device is null"));
    QVERIFY(!QGCCompression::decompressFromDevice(nullptr, outputPath));

    // Test with closed device
    QBuffer closedBuffer;
    closedBuffer.setData(compressedData);
    // Don't open it
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Device is null, not open, or not readable"));
    QVERIFY(!QGCCompression::decompressFromDevice(&closedBuffer, outputPath));
}

void QGCCompressionTest::_testExtractFromDevice()
{
    // Load ZIP from Qt resource into a QBuffer
    QFile resourceFile(":/unittest/manifest.json.zip");
    QVERIFY(resourceFile.open(QIODevice::ReadOnly));
    const QByteArray zipData = resourceFile.readAll();
    resourceFile.close();
    QVERIFY(!zipData.isEmpty());

    // Extract using QBuffer as device
    QBuffer buffer;
    buffer.setData(zipData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));

    const QString outputDir = _tempOutputDir->path() + "/device_extract";
    QVERIFY2(QGCCompression::extractFromDevice(&buffer, outputDir),
             "Failed to extract archive from QBuffer device");
    QVERIFY(QDir(outputDir).exists());

    // Verify extracted content
    QDir extractedDir(outputDir);
    QStringList files = extractedDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
    QVERIFY2(!files.isEmpty(), "No files extracted from device");
    buffer.close();

    // Test with null device
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Device is null"));
    QVERIFY(!QGCCompression::extractFromDevice(nullptr, outputDir));
}

void QGCCompressionTest::_testExtractFileDataFromDevice()
{
    // Load ZIP from Qt resource into a QBuffer
    QFile resourceFile(":/unittest/manifest.json.zip");
    QVERIFY(resourceFile.open(QIODevice::ReadOnly));
    const QByteArray zipData = resourceFile.readAll();
    resourceFile.close();
    QVERIFY(!zipData.isEmpty());

    // Extract single file from device to memory
    QBuffer buffer;
    buffer.setData(zipData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));

    const QByteArray extracted = QGCCompression::extractFileDataFromDevice(&buffer, "manifest.json");
    QVERIFY2(!extracted.isEmpty(), "Failed to extract file data from device");
    QVERIFY(extracted.contains("\"name\""));
    buffer.close();

    // Test non-existent file
    buffer.setData(zipData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File not found in archive"));
    const QByteArray notFound = QGCCompression::extractFileDataFromDevice(&buffer, "nonexistent.txt");
    QVERIFY(notFound.isEmpty());
    buffer.close();

    // Test empty file name
    buffer.setData(zipData);
    QVERIFY(buffer.open(QIODevice::ReadOnly));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("File name cannot be empty"));
    const QByteArray emptyName = QGCCompression::extractFileDataFromDevice(&buffer, "");
    QVERIFY(emptyName.isEmpty());
    buffer.close();

    // Test null device
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("Device is null"));
    const QByteArray nullDevice = QGCCompression::extractFileDataFromDevice(nullptr, "manifest.json");
    QVERIFY(nullDevice.isEmpty());
}
