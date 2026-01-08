#pragma once

#include "UnitTest.h"

class QGCNetworkRetryTest : public UnitTest
{
    Q_OBJECT

private slots:
    // Policy tests
    void _testDefaultPolicy();
    void _testAggressivePolicy();
    void _testConservativePolicy();
    void _testNoRetryPolicy();
    void _testSetPolicy();

    // State tests
    void _testInitialState();
    void _testExecuteWithoutFactory();
    void _testConcurrentExecutionRejected();

    // Cancel and reset tests
    void _testCancel();
    void _testReset();

    // Signal tests
    void _testSignals();

    // Retry logic tests
    void _testShouldRetryOnTimeout();
    void _testShouldRetryOn5xx();
    void _testShouldNotRetryOn4xx();
};
