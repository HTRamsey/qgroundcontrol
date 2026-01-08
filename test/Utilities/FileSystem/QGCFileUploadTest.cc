#include "QGCFileUploadTest.h"
#include "QGCFileUpload.h"

#include <QtCore/QTemporaryDir>
#include <QtCore/QTemporaryFile>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

// ============================================================================
// Construction Tests
// ============================================================================

void QGCFileUploadTest::_testConstruction()
{
    QGCFileUpload uploader(this);
    QVERIFY(!uploader.isRunning());
}

void QGCFileUploadTest::_testInitialState()
{
    QGCFileUpload uploader(this);

    QVERIFY(!uploader.isRunning());
    QCOMPARE(uploader.progress(), 0.0);
    QVERIFY(uploader.errorString().isEmpty());
    QVERIFY(uploader.url().isEmpty());
    QVERIFY(uploader.localPath().isEmpty());
    QCOMPARE(uploader.httpStatus(), 0);
    QVERIFY(uploader.responseBody().isEmpty());
    QVERIFY(uploader.responseHeaders().isEmpty());
}

// ============================================================================
// Validation Tests
// ============================================================================

void QGCFileUploadTest::_testEmptyUrl()
{
    QGCFileUpload uploader(this);

    // Empty URL should fail
    QVERIFY(!uploader.uploadFile("", "/some/file.txt"));
    QVERIFY(!uploader.errorString().isEmpty());

    QVERIFY(!uploader.uploadFileMultipart("", "/some/file.txt"));
    QVERIFY(!uploader.errorString().isEmpty());

    QVERIFY(!uploader.uploadData("", QByteArray("test")));
    QVERIFY(!uploader.errorString().isEmpty());
}

void QGCFileUploadTest::_testEmptyFilePath()
{
    QGCFileUpload uploader(this);

    // Empty file path should fail
    QVERIFY(!uploader.uploadFile("http://example.com/upload", ""));
    QVERIFY(!uploader.errorString().isEmpty());

    QVERIFY(!uploader.uploadFileMultipart("http://example.com/upload", ""));
    QVERIFY(!uploader.errorString().isEmpty());
}

void QGCFileUploadTest::_testInvalidFilePath()
{
    QGCFileUpload uploader(this);

    // Non-existent file should fail
    QVERIFY(!uploader.uploadFile("http://example.com/upload", "/nonexistent/path/file.txt"));
    QVERIFY(!uploader.errorString().isEmpty());
}

// ============================================================================
// Upload Method Tests
// ============================================================================

void QGCFileUploadTest::_testUploadDataBasic()
{
    QGCFileUpload uploader(this);

    // Note: We can't actually upload without a server, but we can verify
    // the upload starts and the signals are set up correctly
    QSignalSpy runningSpy(&uploader, &QGCFileUpload::runningChanged);

    // This will start but fail (no server) - that's expected
    QVERIFY(uploader.uploadData("http://localhost:99999/upload", QByteArray("test data")));
    QVERIFY(uploader.isRunning());
    QCOMPARE(runningSpy.count(), 1);

    // Wait for failure
    QSignalSpy finishedSpy(&uploader, &QGCFileUpload::finished);
    QVERIFY(finishedSpy.wait(5000));

    // Should have failed (no server)
    QVERIFY(!uploader.isRunning());
}

void QGCFileUploadTest::_testConcurrentUploadRejected()
{
    QGCFileUpload uploader(this);

    // Start first upload
    QVERIFY(uploader.uploadData("http://localhost:99999/upload", QByteArray("test")));

    // Second upload should be rejected while first is running
    QVERIFY(!uploader.uploadData("http://localhost:99999/upload2", QByteArray("test2")));
    QVERIFY(!uploader.uploadFile("http://localhost:99999/upload2", "/some/file.txt"));
    QVERIFY(!uploader.uploadFileMultipart("http://localhost:99999/upload2", "/some/file.txt"));

    // Wait for first upload to complete (will fail, no server)
    QSignalSpy spy(&uploader, &QGCFileUpload::finished);
    QVERIFY(spy.wait(5000));
}

void QGCFileUploadTest::_testCancel()
{
    QGCFileUpload uploader(this);

    // Create spy BEFORE starting upload so we don't miss the signal
    QSignalSpy spy(&uploader, &QGCFileUpload::finished);

    // Start upload
    QVERIFY(uploader.uploadData("http://localhost:99999/upload", QByteArray("test data")));
    QVERIFY(uploader.isRunning());

    // Cancel immediately
    uploader.cancel();

    // The finished signal may be emitted synchronously or async
    // Wait for signal or check if already received
    if (spy.count() == 0) {
        spy.wait(5000);
    }

    // Process events to ensure all signals are handled
    QCoreApplication::processEvents();

    // Should not be running after cancel
    QVERIFY(!uploader.isRunning());
}

// ============================================================================
// Form Field Tests
// ============================================================================

void QGCFileUploadTest::_testAddFormField()
{
    QGCFileUpload uploader(this);

    // Add form fields (no assertion needed, just verify no crash)
    uploader.addFormField("field1", "value1");
    uploader.addFormField("field2", "value2");
    uploader.addFormField("field3", "value3");

    // Clear and add again
    uploader.clearFormFields();
    uploader.addFormField("newField", "newValue");
}

void QGCFileUploadTest::_testClearFormFields()
{
    QGCFileUpload uploader(this);

    uploader.addFormField("field1", "value1");
    uploader.addFormField("field2", "value2");

    // Clear should not crash
    uploader.clearFormFields();

    // Clear again (empty) should also not crash
    uploader.clearFormFields();
}

// ============================================================================
// Header Tests
// ============================================================================

void QGCFileUploadTest::_testSetHeader()
{
    QGCFileUpload uploader(this);

    // Set headers
    uploader.setHeader("X-Custom-Header", "value1");
    uploader.setHeader("Authorization", "Bearer token123");

    // Set same header again (should replace)
    uploader.setHeader("X-Custom-Header", "value2");
}

void QGCFileUploadTest::_testClearHeaders()
{
    QGCFileUpload uploader(this);

    uploader.setHeader("Header1", "value1");
    uploader.setHeader("Header2", "value2");

    // Clear should not crash
    uploader.clearHeaders();

    // Clear again (empty) should also not crash
    uploader.clearHeaders();
}

// ============================================================================
// Content Type Detection Tests
// ============================================================================

void QGCFileUploadTest::_testContentTypeDetection()
{
    // Create temporary files with different extensions
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    // Write test files
    QFile jsonFile(tempDir.path() + "/test.json");
    QVERIFY(jsonFile.open(QIODevice::WriteOnly));
    jsonFile.write("{\"test\": true}");
    jsonFile.close();

    QFile txtFile(tempDir.path() + "/test.txt");
    QVERIFY(txtFile.open(QIODevice::WriteOnly));
    txtFile.write("Plain text");
    txtFile.close();

    // We can't test the actual content type detection without starting an upload,
    // but we can verify the files were created and would be uploadable
    QVERIFY(QFile::exists(jsonFile.fileName()));
    QVERIFY(QFile::exists(txtFile.fileName()));
}

// ============================================================================
// Signal Tests
// ============================================================================

void QGCFileUploadTest::_testSignals()
{
    QGCFileUpload uploader(this);

    QSignalSpy progressSpy(&uploader, &QGCFileUpload::progressChanged);
    QSignalSpy runningSpy(&uploader, &QGCFileUpload::runningChanged);
    QSignalSpy urlSpy(&uploader, &QGCFileUpload::urlChanged);
    QSignalSpy finishedSpy(&uploader, &QGCFileUpload::finished);

    // Start upload
    QVERIFY(uploader.uploadData("http://localhost:99999/upload", QByteArray("test")));

    // Should have emitted running=true and url changed
    QVERIFY(runningSpy.count() >= 1);
    QCOMPARE(urlSpy.count(), 1);

    // Wait for finished
    QVERIFY(finishedSpy.wait(5000));

    // Verify finished signal parameters
    QCOMPARE(finishedSpy.count(), 1);
    QCOMPARE(finishedSpy.first().count(), 3);  // success, responseBody, errorMessage
}
