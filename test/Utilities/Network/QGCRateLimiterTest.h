#pragma once

#include "UnitTest.h"

class QGCRateLimiterTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testDefaultConfiguration();
    void _testCanExecute();
    void _testTryExecute();
    void _testBurstCapacity();
    void _testExecuteWithCallback();
    void _testReset();
    void _testTimeUntilAvailable();
};
