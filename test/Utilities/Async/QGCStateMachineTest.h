#pragma once

#include "UnitTest.h"

class QGCStateMachineTest : public UnitTest
{
    Q_OBJECT

private slots:
    void _testBasicTransitions();
    void _testGuards();
    void _testActions();
    void _testHistoricalStates();
    void _testPersistence();
};
