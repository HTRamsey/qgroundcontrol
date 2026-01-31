#pragma once

#include "UnitTest.h"

class QGCStateMachineTest : public UnitTest
{
    Q_OBJECT

private slots:
    // QGCStateMachine helpers
    void _testQGCStateMachineFactories();
    void _testGlobalErrorState();
    void _testLocalErrorState();

    // Property assignment
    void _testPropertyAssignment();
};
