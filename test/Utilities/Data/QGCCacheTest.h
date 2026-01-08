#pragma once

#include "UnitTest.h"

class QGCCacheTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testInsertAndGet();
    void _testCapacity();
    void _testTTLExpiration();
    void _testLRUEviction();
    void _testRemove();
    void _testClear();
    void _testPurgeExpired();
};
