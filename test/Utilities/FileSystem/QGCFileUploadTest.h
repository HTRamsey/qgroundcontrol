#pragma once

#include "UnitTest.h"

class QGCFileUploadTest : public UnitTest
{
    Q_OBJECT

private slots:
    // Construction tests
    void _testConstruction();
    void _testInitialState();

    // Validation tests
    void _testEmptyUrl();
    void _testEmptyFilePath();
    void _testInvalidFilePath();

    // Upload method tests
    void _testUploadDataBasic();
    void _testConcurrentUploadRejected();
    void _testCancel();

    // Form field tests
    void _testAddFormField();
    void _testClearFormFields();

    // Header tests
    void _testSetHeader();
    void _testClearHeaders();

    // Content type detection tests
    void _testContentTypeDetection();

    // Signal tests
    void _testSignals();
};
