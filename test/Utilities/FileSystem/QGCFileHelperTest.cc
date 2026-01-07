#include "QGCFileHelperTest.h"
#include "QGCFileHelper.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtTest/QTest>

void QGCFileHelperTest::init()
{
    UnitTest::init();
    _tempDir = new QTemporaryDir();
    QVERIFY(_tempDir->isValid());
}

void QGCFileHelperTest::cleanup()
{
    delete _tempDir;
    _tempDir = nullptr;
    UnitTest::cleanup();
}

// ============================================================================
// exists() tests
// ============================================================================

void QGCFileHelperTest::_testExistsRegularFile()
{
    // Create a test file
    const QString filePath = _tempDir->filePath("testfile.txt");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("test content");
    file.close();

    QVERIFY(QGCFileHelper::exists(filePath));
}

void QGCFileHelperTest::_testExistsDirectory()
{
    // The temp directory should exist
    QVERIFY(QGCFileHelper::exists(_tempDir->path()));

    // Create a subdirectory
    const QString subDir = _tempDir->filePath("subdir");
    QVERIFY(QDir().mkdir(subDir));
    QVERIFY(QGCFileHelper::exists(subDir));
}

void QGCFileHelperTest::_testExistsNonExistent()
{
    QVERIFY(!QGCFileHelper::exists("/nonexistent/path/that/does/not/exist"));
    QVERIFY(!QGCFileHelper::exists(_tempDir->filePath("nonexistent.txt")));
}

void QGCFileHelperTest::_testExistsQtResource()
{
    // Qt resources with :/ prefix should be handled specially
    // exists() returns true for any :/ path (Qt resource system)
    QVERIFY(QGCFileHelper::exists(":/qgc/unittest/manifest.json.zip"));

    // Even non-existent resource paths return true (by design - Qt handles validation)
    // This is intentional: the function assumes :/ paths are valid resources
    QVERIFY(QGCFileHelper::exists(":/any/resource/path"));
}

// ============================================================================
// ensureDirectoryExists() tests
// ============================================================================

void QGCFileHelperTest::_testEnsureDirectoryExistsAlreadyExists()
{
    // Temp directory already exists
    QVERIFY(QGCFileHelper::ensureDirectoryExists(_tempDir->path()));
    QVERIFY(QDir(_tempDir->path()).exists());
}

void QGCFileHelperTest::_testEnsureDirectoryExistsCreate()
{
    const QString newDir = _tempDir->filePath("newdir");
    QVERIFY(!QDir(newDir).exists());

    QVERIFY(QGCFileHelper::ensureDirectoryExists(newDir));
    QVERIFY(QDir(newDir).exists());
}

void QGCFileHelperTest::_testEnsureDirectoryExistsNested()
{
    const QString nestedDir = _tempDir->filePath("level1/level2/level3");
    QVERIFY(!QDir(nestedDir).exists());

    QVERIFY(QGCFileHelper::ensureDirectoryExists(nestedDir));
    QVERIFY(QDir(nestedDir).exists());
}

// ============================================================================
// ensureParentExists() tests
// ============================================================================

void QGCFileHelperTest::_testEnsureParentExistsAlreadyExists()
{
    // Parent of a file in temp dir already exists
    const QString filePath = _tempDir->filePath("somefile.txt");
    QVERIFY(QGCFileHelper::ensureParentExists(filePath));
    QVERIFY(QDir(_tempDir->path()).exists());
}

void QGCFileHelperTest::_testEnsureParentExistsCreate()
{
    const QString filePath = _tempDir->filePath("newparent/somefile.txt");
    const QString parentDir = _tempDir->filePath("newparent");
    QVERIFY(!QDir(parentDir).exists());

    QVERIFY(QGCFileHelper::ensureParentExists(filePath));
    QVERIFY(QDir(parentDir).exists());
}

void QGCFileHelperTest::_testEnsureParentExistsNested()
{
    const QString filePath = _tempDir->filePath("a/b/c/somefile.txt");
    const QString parentDir = _tempDir->filePath("a/b/c");
    QVERIFY(!QDir(parentDir).exists());

    QVERIFY(QGCFileHelper::ensureParentExists(filePath));
    QVERIFY(QDir(parentDir).exists());
}

// ============================================================================
// optimalBufferSize() tests
// ============================================================================

void QGCFileHelperTest::_testOptimalBufferSizeConstants()
{
    // Verify constants are properly defined
    QVERIFY(QGCFileHelper::kBufferSizeMin == 16384);      // 16KB
    QVERIFY(QGCFileHelper::kBufferSizeMax == 131072);     // 128KB
    QVERIFY(QGCFileHelper::kBufferSizeDefault == 65536);  // 64KB

    // Sanity: min < default < max
    QVERIFY(QGCFileHelper::kBufferSizeMin < QGCFileHelper::kBufferSizeDefault);
    QVERIFY(QGCFileHelper::kBufferSizeDefault < QGCFileHelper::kBufferSizeMax);
}

void QGCFileHelperTest::_testOptimalBufferSizeWithinBounds()
{
    const size_t size = QGCFileHelper::optimalBufferSize();

    QVERIFY(size >= QGCFileHelper::kBufferSizeMin);
    QVERIFY(size <= QGCFileHelper::kBufferSizeMax);
}

void QGCFileHelperTest::_testOptimalBufferSizeCached()
{
    // Multiple calls should return the same value (cached)
    const size_t first = QGCFileHelper::optimalBufferSize();
    const size_t second = QGCFileHelper::optimalBufferSize();
    const size_t third = QGCFileHelper::optimalBufferSize(_tempDir->path());

    QCOMPARE(first, second);
    QCOMPARE(second, third);
}

void QGCFileHelperTest::_testOptimalBufferSizeWithPath()
{
    // With path argument, should still return valid size within bounds
    const size_t size = QGCFileHelper::optimalBufferSize(_tempDir->path());

    QVERIFY(size >= QGCFileHelper::kBufferSizeMin);
    QVERIFY(size <= QGCFileHelper::kBufferSizeMax);
}

// ============================================================================
// atomicWrite() tests
// ============================================================================

void QGCFileHelperTest::_testAtomicWriteBasic()
{
    const QString filePath = _tempDir->filePath("atomic_basic.txt");
    const QByteArray data = "Hello, atomic write!";

    QVERIFY(!QFile::exists(filePath));
    QVERIFY(QGCFileHelper::atomicWrite(filePath, data));
    QVERIFY(QFile::exists(filePath));

    // Verify content
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), data);
}

void QGCFileHelperTest::_testAtomicWriteOverwrite()
{
    const QString filePath = _tempDir->filePath("atomic_overwrite.txt");
    const QByteArray originalData = "Original content";
    const QByteArray newData = "New content after overwrite";

    // Write original
    QVERIFY(QGCFileHelper::atomicWrite(filePath, originalData));

    // Overwrite
    QVERIFY(QGCFileHelper::atomicWrite(filePath, newData));

    // Verify new content
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), newData);

    // Verify no temp file left behind
    QVERIFY(!QFile::exists(filePath + ".tmp"));
}

void QGCFileHelperTest::_testAtomicWriteCreatesParent()
{
    const QString filePath = _tempDir->filePath("nested/dirs/atomic.txt");
    const QByteArray data = "Data in nested directory";

    QVERIFY(!QFile::exists(filePath));
    QVERIFY(QGCFileHelper::atomicWrite(filePath, data));
    QVERIFY(QFile::exists(filePath));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), data);
}

void QGCFileHelperTest::_testAtomicWriteEmptyPath()
{
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("file path is empty"));
    QVERIFY(!QGCFileHelper::atomicWrite(QString(), QByteArray("data")));
}

void QGCFileHelperTest::_testAtomicWriteEmptyData()
{
    const QString filePath = _tempDir->filePath("atomic_empty.txt");

    // Empty data should succeed and create empty file
    QVERIFY(QGCFileHelper::atomicWrite(filePath, QByteArray()));
    QVERIFY(QFile::exists(filePath));

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.size(), 0);
}

// ============================================================================
// Disk space utilities tests
// ============================================================================

void QGCFileHelperTest::_testAvailableDiskSpaceBasic()
{
    // Should return positive value for valid path
    const qint64 available = QGCFileHelper::availableDiskSpace(_tempDir->path());
    QVERIFY(available > 0);

    // Should work for file path too (uses parent directory)
    const QString filePath = _tempDir->filePath("testfile.txt");
    const qint64 available2 = QGCFileHelper::availableDiskSpace(filePath);
    QVERIFY(available2 > 0);
}

void QGCFileHelperTest::_testAvailableDiskSpaceEmptyPath()
{
    // Empty path should return -1
    QCOMPARE(QGCFileHelper::availableDiskSpace(QString()), qint64(-1));
}

void QGCFileHelperTest::_testHasSufficientDiskSpaceBasic()
{
    // Should have space for 1 byte
    QVERIFY(QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), 1));

    // Should have space for 1KB
    QVERIFY(QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), 1024));

    // Should probably not have space for 1 exabyte (10^18 bytes)
    QVERIFY(!QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), 1000000000000000000LL));
}

void QGCFileHelperTest::_testHasSufficientDiskSpaceZeroBytes()
{
    // Zero bytes should always succeed
    QVERIFY(QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), 0));

    // Negative bytes should also succeed (treated as unknown/nothing to check)
    QVERIFY(QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), -1));
}

void QGCFileHelperTest::_testHasSufficientDiskSpaceWithMargin()
{
    // Get available space
    const qint64 available = QGCFileHelper::availableDiskSpace(_tempDir->path());
    QVERIFY(available > 0);

    // Should have space for half the available (with 10% margin)
    QVERIFY(QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), available / 2, 1.1));

    // Should not have space for 2x available
    QVERIFY(!QGCFileHelper::hasSufficientDiskSpace(_tempDir->path(), available * 2, 1.0));
}

