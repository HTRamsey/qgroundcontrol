#include "LogManagerTest.h"
#include "LogManager.h"
#include "LogModel.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSignalSpy>
#include <QtCore/QThread>
#include <QtTest/QTest>

void LogManagerTest::init()
{
    UnitTest::init();
    _tempDir = new QTemporaryDir();
    QVERIFY(_tempDir->isValid());
}

void LogManagerTest::cleanup()
{
    // Disable logging to clean up
    LogManager::instance()->setDiskLoggingEnabled(false);
    LogManager::instance()->setRemoteLoggingEnabled(false);
    LogManager::instance()->clear();

    delete _tempDir;
    _tempDir = nullptr;
    UnitTest::cleanup();
}

void LogManagerTest::_testSingleton()
{
    LogManager* instance1 = LogManager::instance();
    LogManager* instance2 = LogManager::instance();

    QVERIFY(instance1 != nullptr);
    QCOMPARE(instance1, instance2);
}

void LogManagerTest::_testModel()
{
    LogManager* manager = LogManager::instance();

    QVERIFY(manager->model() != nullptr);

    // Model should be accessible
    LogModel* model = manager->model();
    QVERIFY(model->rowCount() >= 0);
}

void LogManagerTest::_testDiskLogging()
{
    LogManager* manager = LogManager::instance();

    QSignalSpy enabledSpy(manager, &LogManager::diskLoggingEnabledChanged);

    QVERIFY(!manager->isDiskLoggingEnabled());

    manager->setLogDirectory(_tempDir->path());
    manager->setDiskLoggingEnabled(true);

    QVERIFY(manager->isDiskLoggingEnabled());
    QCOMPARE(enabledSpy.count(), 1);

    manager->setDiskLoggingEnabled(false);
    QVERIFY(!manager->isDiskLoggingEnabled());
    QCOMPARE(enabledSpy.count(), 2);
}

void LogManagerTest::_testRemoteLogging()
{
    LogManager* manager = LogManager::instance();

    QSignalSpy enabledSpy(manager, &LogManager::remoteLoggingEnabledChanged);

    QVERIFY(!manager->isRemoteLoggingEnabled());

    manager->setRemoteEndpoint("127.0.0.1:9999");
    QCOMPARE(manager->remoteEndpoint(), QString("127.0.0.1:9999"));

    manager->setRemoteLoggingEnabled(true);
    QVERIFY(manager->isRemoteLoggingEnabled());
    QCOMPARE(enabledSpy.count(), 1);

    manager->setRemoteLoggingEnabled(false);
    QVERIFY(!manager->isRemoteLoggingEnabled());
}

void LogManagerTest::_testSetLogDirectory()
{
    LogManager* manager = LogManager::instance();

    manager->setLogDirectory(_tempDir->path());

    // Enable and write something to verify directory works
    manager->setDiskLoggingEnabled(true);

    // Trigger a log message (through Qt logging)
    qDebug() << "Test message for log directory";

    manager->flush();
    QThread::msleep(200);

    // Check that log file was created
    const QString logPath = QDir(_tempDir->path()).absoluteFilePath("QGCConsole.log");
    QVERIFY(QFile::exists(logPath));

    manager->setDiskLoggingEnabled(false);
}

void LogManagerTest::_testClear()
{
    LogManager* manager = LogManager::instance();
    LogModel* model = manager->model();

    // Add some log entries via qDebug
    for (int i = 0; i < 5; ++i) {
        qDebug() << "Test entry" << i;
    }

    QCoreApplication::processEvents();

    // Model should have entries
    QVERIFY(model->rowCount() > 0);

    // Clear
    manager->clear();

    QCOMPARE(model->rowCount(), 0);
}

void LogManagerTest::_testFlush()
{
    LogManager* manager = LogManager::instance();

    manager->setLogDirectory(_tempDir->path());
    manager->setDiskLoggingEnabled(true);

    qDebug() << "Flush test message";

    // Flush should complete without error
    manager->flush();

    manager->setDiskLoggingEnabled(false);
}

void LogManagerTest::_testExportToFile()
{
    LogManager* manager = LogManager::instance();

    // Add some entries
    for (int i = 0; i < 3; ++i) {
        qDebug() << "Export test message" << i;
    }

    QCoreApplication::processEvents();

    QSignalSpy startedSpy(manager, &LogManager::exportStarted);
    QSignalSpy finishedSpy(manager, &LogManager::exportFinished);

    const QString exportPath = _tempDir->filePath("export.log");

    manager->exportToFile(exportPath);

    QCOMPARE(startedSpy.count(), 1);

    // Wait for export to complete
    QVERIFY(finishedSpy.wait(5000));
    QCOMPARE(finishedSpy.count(), 1);

    // Check success parameter
    const QList<QVariant> args = finishedSpy.takeFirst();
    QVERIFY(args.at(0).toBool());

    // Verify file exists
    QVERIFY(QFile::exists(exportPath));

    // Verify content
    QFile file(exportPath);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QString content = QString::fromUtf8(file.readAll());

    QVERIFY(content.contains("Export test message"));
}

void LogManagerTest::_testErrorState()
{
    LogManager* manager = LogManager::instance();

    // Initially no error
    QVERIFY(!manager->hasError());

    // Clear error should be safe even when no error
    manager->clearError();
    QVERIFY(!manager->hasError());
}
