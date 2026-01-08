#pragma once

#include "UnitTest.h"

class QGCNetworkManagerTest : public UnitTest
{
    Q_OBJECT

private slots:
    // Singleton tests
    void _testSingleton();
    void _testSingletonSameInstance();

    // Configuration tests
    void _testDefaultConfiguration();
    void _testSetDefaultTimeout();
    void _testSetAutoRedirect();
    void _testSetMaxRedirects();

    // Statistics tests
    void _testInitialStatistics();
    void _testClearStatistics();

    // Request tests
    void _testGetRequest();
    void _testPostRequest();
    void _testPutRequest();
    void _testDeleteRequest();
    void _testHeadRequest();

    // Tracking tests
    void _testActiveRequestCount();
    void _testRequestSignals();
};
