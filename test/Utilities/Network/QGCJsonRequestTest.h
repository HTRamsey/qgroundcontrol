#pragma once

#include "UnitTest.h"

class QGCJsonRequestTest : public UnitTest
{
    Q_OBJECT

private slots:
    // Construction and state tests
    void _testConstruction();
    void _testInitialState();

    // Property tests
    void _testSetBearerToken();
    void _testSetBaseUrl();
    void _testSetTimeout();

    // Header tests
    void _testSetHeader();
    void _testRemoveHeader();
    void _testClearHeaders();

    // URL resolution tests
    void _testResolveAbsoluteUrl();
    void _testResolveRelativeUrl();

    // Request method tests
    void _testGetBasic();
    void _testGetWithParams();
    void _testPostEmpty();
    void _testPostObject();
    void _testPostArray();
    void _testPut();
    void _testPatch();
    void _testDelete();

    // Concurrent request tests
    void _testConcurrentRequestRejected();

    // Cancel tests
    void _testCancel();

    // Signal tests
    void _testSignals();
};
