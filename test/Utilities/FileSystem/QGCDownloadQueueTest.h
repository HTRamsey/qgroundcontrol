#pragma once

#include "UnitTest.h"

class QGCDownloadQueueTest : public UnitTest
{
    Q_OBJECT

private slots:
    // Construction tests
    void _testConstruction();
    void _testInitialState();

    // Configuration tests
    void _testSetMaxConcurrent();
    void _testSetMaxRetries();

    // Queue management tests
    void _testAddDownload();
    void _testAddDownloadPriority();
    void _testRemoveDownload();
    void _testClearCompleted();

    // State tests
    void _testStartPause();
    void _testPauseResume();
    void _testCancelAll();

    // Signal tests
    void _testQueueSignals();
};
