#pragma once

#include "UnitTest.h"

#include <QtCore/QTemporaryDir>

/// Tests for QGCFileHelper (generic file system utilities)
class QGCFileHelperTest : public UnitTest
{
    Q_OBJECT

public:
    QGCFileHelperTest() = default;

private slots:
    void init() override;
    void cleanup() override;

    // exists() tests
    void _testExistsRegularFile();
    void _testExistsDirectory();
    void _testExistsNonExistent();
    void _testExistsQtResource();

    // ensureDirectoryExists() tests
    void _testEnsureDirectoryExistsAlreadyExists();
    void _testEnsureDirectoryExistsCreate();
    void _testEnsureDirectoryExistsNested();

    // ensureParentExists() tests
    void _testEnsureParentExistsAlreadyExists();
    void _testEnsureParentExistsCreate();
    void _testEnsureParentExistsNested();

    // optimalBufferSize() tests
    void _testOptimalBufferSizeConstants();
    void _testOptimalBufferSizeWithinBounds();
    void _testOptimalBufferSizeCached();
    void _testOptimalBufferSizeWithPath();

    // atomicWrite() tests
    void _testAtomicWriteBasic();
    void _testAtomicWriteOverwrite();
    void _testAtomicWriteCreatesParent();
    void _testAtomicWriteEmptyPath();
    void _testAtomicWriteEmptyData();

    // Disk space utilities tests
    void _testAvailableDiskSpaceBasic();
    void _testAvailableDiskSpaceEmptyPath();
    void _testHasSufficientDiskSpaceBasic();
    void _testHasSufficientDiskSpaceZeroBytes();
    void _testHasSufficientDiskSpaceWithMargin();

private:
    QTemporaryDir *_tempDir = nullptr;
};
