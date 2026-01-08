#pragma once

#include "UnitTest.h"

class QGCDebouncerTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testDefaultConfiguration();
    void _testDebounce();
    void _testLeadingEdge();
    void _testCancel();
    void _testFlush();
    void _testThrottler();
};
