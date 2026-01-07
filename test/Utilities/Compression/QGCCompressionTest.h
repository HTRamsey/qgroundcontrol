#pragma once

#include "UnitTest.h"

#include <QtCore/QTemporaryDir>

/// Tests for QGCCompression (decompression-only: format detection, extraction, decompression)
class QGCCompressionTest : public UnitTest
{
    Q_OBJECT

public:
    QGCCompressionTest() = default;

private slots:
    void init() override;
    void cleanup() override;

    // Format detection
    void _testFormatDetection();
    void _testFormatHelpers();

    // Archive extraction from Qt resources
    void _testZipFromResource();
    void _test7zFromResource();
    void _testListArchive();
    void _testListArchiveDetailed();
    void _testGetArchiveStats();
    void _testValidateArchive();
    void _testFileExists();
    void _testExtractArchiveFiltered();
    void _testExtractSingleFile();
    void _testExtractFileData();
    void _testExtractMultipleFiles();
    void _testExtractByPattern();

    // Single-file decompression from Qt resources
    void _testDecompressFromResource();
    void _testDecompressData();
    void _testDecompressIfNeeded();

    // Progress callbacks
    void _testProgressCallbackExtract();

    // Edge cases and error handling
    void _testCorruptArchive();
    void _testNonExistentInput();

    // QIODevice-based operations
    void _testDecompressFromDevice();
    void _testExtractFromDevice();
    void _testExtractFileDataFromDevice();

private:
    bool _compareFiles(const QString &file1, const QString &file2);

    QTemporaryDir *_tempOutputDir = nullptr;
};
